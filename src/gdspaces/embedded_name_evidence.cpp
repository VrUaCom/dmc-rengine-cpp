#include "dmc_rengine/gdspaces/embedded_name_evidence.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/resource_name_evidence.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

struct DecodedCharacter final {
    std::uint32_t codepoint{};
    std::uint64_t source_offset{};
};

struct ParsedAlias final {
    std::string name;
    std::uint64_t source_offset{};
};

[[nodiscard]] bool valid_digest(std::string_view digest) noexcept {
    if (digest.size() != 64U) {
        return false;
    }
    return std::all_of(
        digest.begin(), digest.end(),
        [](unsigned char character) {
            return std::isxdigit(character) != 0;
        });
}

[[nodiscard]] bool is_continuation(unsigned char value) noexcept {
    return value >= 0x80U && value <= 0xBFU;
}

void append_decoded(
    std::vector<DecodedCharacter>& decoded,
    std::uint32_t codepoint,
    std::size_t source_offset,
    std::size_t& utf16_length,
    std::size_t& printable_count) {
    // The retained TypeScript parser performed NUL -> newline before measuring
    // printableRatio(). Preserve that exact ordering here.
    if (codepoint == 0U) {
        codepoint = static_cast<std::uint32_t>('\n');
    }

    decoded.push_back(DecodedCharacter{
        .codepoint = codepoint,
        .source_offset = static_cast<std::uint64_t>(source_offset),
    });

    utf16_length += codepoint > 0xFFFFU ? 2U : 1U;
    if (codepoint == static_cast<std::uint32_t>('\t') ||
        codepoint == static_cast<std::uint32_t>('\n') ||
        codepoint == static_cast<std::uint32_t>('\r') ||
        (codepoint >= 32U && codepoint <= 126U)) {
        ++printable_count;
    }
}

[[nodiscard]] std::vector<DecodedCharacter> decode_utf8_with_replacement(
    std::span<const std::byte> bytes,
    std::size_t& utf16_length,
    std::size_t& printable_count) {
    std::vector<DecodedCharacter> decoded;
    decoded.reserve(bytes.size());
    utf16_length = 0U;
    printable_count = 0U;

    std::size_t cursor = 0U;
    while (cursor < bytes.size()) {
        const auto lead = std::to_integer<unsigned char>(bytes[cursor]);
        if (lead <= 0x7FU) {
            append_decoded(
                decoded,
                static_cast<std::uint32_t>(lead),
                cursor,
                utf16_length,
                printable_count);
            ++cursor;
            continue;
        }

        std::size_t width = 0U;
        std::uint32_t codepoint = 0U;
        unsigned char second_min = 0x80U;
        unsigned char second_max = 0xBFU;
        if (lead >= 0xC2U && lead <= 0xDFU) {
            width = 2U;
            codepoint = static_cast<std::uint32_t>(lead & 0x1FU);
        } else if (lead >= 0xE0U && lead <= 0xEFU) {
            width = 3U;
            codepoint = static_cast<std::uint32_t>(lead & 0x0FU);
            if (lead == 0xE0U) {
                second_min = 0xA0U;
            } else if (lead == 0xEDU) {
                second_max = 0x9FU;
            }
        } else if (lead >= 0xF0U && lead <= 0xF4U) {
            width = 4U;
            codepoint = static_cast<std::uint32_t>(lead & 0x07U);
            if (lead == 0xF0U) {
                second_min = 0x90U;
            } else if (lead == 0xF4U) {
                second_max = 0x8FU;
            }
        } else {
            append_decoded(
                decoded,
                0xFFFDU,
                cursor,
                utf16_length,
                printable_count);
            ++cursor;
            continue;
        }

        const auto sequence_start = cursor;
        const auto second_index = cursor + 1U;
        if (second_index >= bytes.size()) {
            append_decoded(
                decoded,
                0xFFFDU,
                sequence_start,
                utf16_length,
                printable_count);
            cursor = bytes.size();
            continue;
        }

        const auto second = std::to_integer<unsigned char>(bytes[second_index]);
        if (second < second_min || second > second_max) {
            append_decoded(
                decoded,
                0xFFFDU,
                sequence_start,
                utf16_length,
                printable_count);
            ++cursor;
            continue;
        }

        codepoint = (codepoint << 6U) |
            static_cast<std::uint32_t>(second & 0x3FU);
        std::size_t next = second_index + 1U;
        bool incomplete = false;
        bool invalid_continuation = false;
        while (next < sequence_start + width) {
            if (next >= bytes.size()) {
                incomplete = true;
                break;
            }
            const auto continuation =
                std::to_integer<unsigned char>(bytes[next]);
            if (!is_continuation(continuation)) {
                invalid_continuation = true;
                break;
            }
            codepoint = (codepoint << 6U) |
                static_cast<std::uint32_t>(continuation & 0x3FU);
            ++next;
        }

        if (incomplete) {
            append_decoded(
                decoded,
                0xFFFDU,
                sequence_start,
                utf16_length,
                printable_count);
            cursor = bytes.size();
            continue;
        }
        if (invalid_continuation) {
            append_decoded(
                decoded,
                0xFFFDU,
                sequence_start,
                utf16_length,
                printable_count);
            // Valid continuation bytes already consumed by the decoder belong
            // to the rejected sequence. Reprocess the first offending byte.
            cursor = next;
            continue;
        }

        append_decoded(
            decoded,
            codepoint,
            sequence_start,
            utf16_length,
            printable_count);
        cursor = next;
    }

    return decoded;
}

[[nodiscard]] bool split_whitespace(std::uint32_t codepoint) noexcept {
    return codepoint == static_cast<std::uint32_t>(' ') ||
           codepoint == static_cast<std::uint32_t>('\t') ||
           codepoint == static_cast<std::uint32_t>('\n') ||
           codepoint == static_cast<std::uint32_t>('\r');
}

[[nodiscard]] bool ecmascript_trim_whitespace(
    std::uint32_t codepoint) noexcept {
    if (split_whitespace(codepoint) || codepoint == 0x000BU ||
        codepoint == 0x000CU || codepoint == 0x00A0U ||
        codepoint == 0x1680U || codepoint == 0x2028U ||
        codepoint == 0x2029U || codepoint == 0x202FU ||
        codepoint == 0x205FU || codepoint == 0x3000U ||
        codepoint == 0xFEFFU) {
        return true;
    }
    return codepoint >= 0x2000U && codepoint <= 0x200AU;
}

void append_utf8(std::string& output, std::uint32_t codepoint) {
    if (codepoint <= 0x7FU) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FFU) {
        output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint <= 0xFFFFU) {
        output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
        output.push_back(static_cast<char>(
            0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
        output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
        output.push_back(static_cast<char>(
            0x80U | ((codepoint >> 12U) & 0x3FU)));
        output.push_back(static_cast<char>(
            0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    }
}

[[nodiscard]] bool ascii_word(unsigned char value) noexcept {
    return (value >= static_cast<unsigned char>('a') &&
            value <= static_cast<unsigned char>('z')) ||
           (value >= static_cast<unsigned char>('A') &&
            value <= static_cast<unsigned char>('Z')) ||
           (value >= static_cast<unsigned char>('0') &&
            value <= static_cast<unsigned char>('9')) ||
           value == static_cast<unsigned char>('_');
}

[[nodiscard]] unsigned char ascii_lower(unsigned char value) noexcept {
    if (value >= static_cast<unsigned char>('A') &&
        value <= static_cast<unsigned char>('Z')) {
        return static_cast<unsigned char>(
            value - static_cast<unsigned char>('A') +
            static_cast<unsigned char>('a'));
    }
    return value;
}

[[nodiscard]] bool match_ascii_ci(
    std::string_view value,
    std::size_t offset,
    std::string_view expected) noexcept {
    if (offset > value.size() || expected.size() > value.size() - offset) {
        return false;
    }
    for (std::size_t index = 0U; index < expected.size(); ++index) {
        const auto actual = static_cast<unsigned char>(value[offset + index]);
        const auto wanted = static_cast<unsigned char>(expected[index]);
        if (ascii_lower(actual) != ascii_lower(wanted)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool word_boundary_after(
    std::string_view value,
    std::size_t offset) noexcept {
    return offset >= value.size() ||
        !ascii_word(static_cast<unsigned char>(value[offset]));
}

[[nodiscard]] bool retained_extension_match(std::string_view token) noexcept {
    constexpr std::array<std::string_view, 21> extensions{
        "ptx", "clt", "c1d", "scm", "sch", "txt", "pac",
        "cam", "hid", "tsc", "hits", "dds", "tm2", "mod",
        "lig", "eve", "pos", "sef", "dca", "est", "mot",
    };

    for (std::size_t dot = 0U; dot < token.size(); ++dot) {
        if (token[dot] != '.') {
            continue;
        }
        const auto start = dot + 1U;
        for (const auto extension : extensions) {
            if (!match_ascii_ci(token, start, extension)) {
                continue;
            }
            const auto end = start + extension.size();
            if (extension != "mot" && word_boundary_after(token, end)) {
                return true;
            }
            if (extension == "mot") {
                if (word_boundary_after(token, end)) {
                    return true;
                }
                std::size_t digit_end = end;
                while (digit_end < token.size()) {
                    const auto value =
                        static_cast<unsigned char>(token[digit_end]);
                    if (value < static_cast<unsigned char>('0') ||
                        value > static_cast<unsigned char>('9')) {
                        break;
                    }
                    ++digit_end;
                }
                if (digit_end > end && word_boundary_after(token, digit_end)) {
                    return true;
                }
            }
        }
    }
    return false;
}

[[nodiscard]] std::vector<ParsedAlias> parse_retained_name_list(
    std::span<const std::byte> bytes) {
    if (bytes.empty() || bytes.size() > 4096U) {
        return {};
    }

    std::size_t utf16_length = 0U;
    std::size_t printable_count = 0U;
    const auto decoded = decode_utf8_with_replacement(
        bytes, utf16_length, printable_count);
    if (utf16_length == 0U ||
        static_cast<double>(printable_count) /
                static_cast<double>(utf16_length) <
            0.75) {
        return {};
    }

    std::vector<ParsedAlias> aliases;
    std::size_t cursor = 0U;
    while (cursor < decoded.size()) {
        while (cursor < decoded.size() &&
               split_whitespace(decoded[cursor].codepoint)) {
            ++cursor;
        }
        if (cursor >= decoded.size()) {
            break;
        }

        std::size_t end = cursor;
        while (end < decoded.size() &&
               !split_whitespace(decoded[end].codepoint)) {
            ++end;
        }

        std::size_t first = cursor;
        while (first < end &&
               ecmascript_trim_whitespace(decoded[first].codepoint)) {
            ++first;
        }
        std::size_t last = end;
        while (last > first &&
               ecmascript_trim_whitespace(decoded[last - 1U].codepoint)) {
            --last;
        }

        if (first < last) {
            std::string token;
            for (std::size_t index = first; index < last; ++index) {
                append_utf8(token, decoded[index].codepoint);
            }
            if (!token.empty() && retained_extension_match(token)) {
                aliases.push_back(ParsedAlias{
                    .name = std::move(token),
                    .source_offset = decoded[first].source_offset,
                });
            }
        }
        cursor = end;
    }
    return aliases;
}

[[nodiscard]] ContainerChild* find_slot(
    ContainerExpansion& expansion,
    std::uint32_t slot) noexcept {
    const auto iterator = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [&](const ContainerChild& child) {
            return child.entry.slot_index == slot;
        });
    return iterator == expansion.children.end() ? nullptr : &*iterator;
}

[[nodiscard]] const ContainerChild* find_slot(
    const ContainerExpansion& expansion,
    std::uint32_t slot) noexcept {
    const auto iterator = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [&](const ContainerChild& child) {
            return child.entry.slot_index == slot;
        });
    return iterator == expansion.children.end() ? nullptr : &*iterator;
}

[[nodiscard]] const ResourceNameEvidence* external_index_evidence(
    const ResourcePayload& payload) noexcept {
    const auto iterator = std::find_if(
        payload.name_evidence.begin(), payload.name_evidence.end(),
        [](const ResourceNameEvidence& evidence) {
            return evidence.kind() == ResourceNameEvidenceKind::external_index &&
                   evidence.valid();
        });
    return iterator == payload.name_evidence.end() ? nullptr : &*iterator;
}

void split_name(
    std::string_view name,
    std::string& stem,
    std::string& extension) {
    const auto separator = name.find_last_of("/\\");
    const auto dot = name.find_last_of('.');
    if (dot == std::string_view::npos ||
        (separator != std::string_view::npos && dot <= separator) ||
        dot + 1U >= name.size()) {
        stem.assign(name);
        extension.clear();
        return;
    }
    stem.assign(name.substr(0U, dot));
    extension.assign(name.substr(dot + 1U));
}

[[nodiscard]] std::string canonical_extension(std::string_view format) {
    if (format == "pe") {
        return "exe";
    }
    if (format == "name-list") {
        return "txt";
    }
    return std::string{format};
}

[[nodiscard]] std::string make_display_name(
    std::string_view stem,
    std::string_view extension) {
    if (extension.empty()) {
        return std::string{stem};
    }
    std::string result;
    result.reserve(stem.size() + extension.size() + 1U);
    result.append(stem);
    result.push_back('.');
    result.append(extension);
    return result;
}

[[nodiscard]] std::string alias_display_name(
    const ResourcePayload& child,
    std::string_view alias) {
    std::string stem;
    std::string source_extension;
    split_name(alias, stem, source_extension);
    const auto classification = ResourceClassifier::classify(
        alias,
        std::span<const std::byte>{child.bytes.data(), child.bytes.size()});
    if (classification.magic_confirmed) {
        return make_display_name(
            stem, canonical_extension(classification.format));
    }
    return source_extension.empty()
        ? std::string{alias}
        : make_display_name(stem, source_extension);
}

[[nodiscard]] std::string parent_stem(const ContainerExpansion& expansion) {
    std::string stem;
    std::string extension;
    split_name(expansion.parent.display_name, stem, extension);
    if (!stem.empty()) {
        return stem;
    }
    split_name(expansion.parent.id.logical_path, stem, extension);
    return stem.empty() ? std::string{"container"} : stem;
}

[[nodiscard]] std::string synthetic_name_list_display(
    const ContainerExpansion& expansion) {
    std::ostringstream output;
    output << parent_stem(expansion) << '_' << std::setfill('0')
           << std::setw(3) << 0 << ".name-list.txt";
    return output.str();
}

void add_error(
    std::vector<Diagnostic>& diagnostics,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

void add_warning(
    std::vector<Diagnostic>& diagnostics,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::warning,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

} // namespace

EmbeddedNameAlias::EmbeddedNameAlias(
    std::string name,
    std::uint64_t source_offset)
    : name_(std::move(name)), source_offset_(source_offset) {}

std::string_view EmbeddedNameAlias::name() const noexcept {
    return name_;
}

std::uint64_t EmbeddedNameAlias::source_offset() const noexcept {
    return source_offset_;
}

EmbeddedNameListObservation::EmbeddedNameListObservation(
    ResourceId parent_resource,
    ResourceId authority_resource,
    std::string authority_sha256,
    std::vector<EmbeddedNameAlias> aliases)
    : parent_resource_(std::move(parent_resource)),
      authority_resource_(std::move(authority_resource)),
      authority_sha256_(std::move(authority_sha256)),
      aliases_(std::move(aliases)) {}

const ResourceId& EmbeddedNameListObservation::parent_resource() const noexcept {
    return parent_resource_;
}

const ResourceId& EmbeddedNameListObservation::authority_resource() const noexcept {
    return authority_resource_;
}

std::string_view EmbeddedNameListObservation::authority_sha256() const noexcept {
    return authority_sha256_;
}

const std::vector<EmbeddedNameAlias>&
EmbeddedNameListObservation::aliases() const noexcept {
    return aliases_;
}

bool EmbeddedNameListObservation::valid() const noexcept {
    if (!parent_resource_.valid() || !authority_resource_.valid() ||
        !valid_digest(authority_sha256_) || aliases_.empty() ||
        authority_resource_.size == 0U || authority_resource_.size > 4096U) {
        return false;
    }
    return std::all_of(
        aliases_.begin(), aliases_.end(),
        [&](const EmbeddedNameAlias& alias) {
            return !alias.name().empty() &&
                   alias.source_offset() < authority_resource_.size;
        });
}

bool EmbeddedNameObserveResult::ok() const noexcept {
    if (!observation.has_value() || !observation->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

bool EmbeddedNameApplyResult::ok() const noexcept {
    if (!applied) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

EmbeddedNameObserveResult EmbeddedNameEvidenceBuilder::observe(
    const ContainerExpansion& expansion) {
    EmbeddedNameObserveResult result;
    if (!expansion.usable()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.embedded-name.invalid-expansion",
            "Embedded-name observation requires a usable physical container expansion.");
        return result;
    }

    const auto* slot_zero = find_slot(expansion, 0U);
    if (slot_zero == nullptr || !slot_zero->entry.populated) {
        return result;
    }
    if (!slot_zero->payload.readable()) {
        add_error(
            result.diagnostics,
            slot_zero->payload.resource.id,
            "gdspaces.embedded-name.slot-zero-unreadable",
            "Physical slot 0 is present but is not safely readable.");
        return result;
    }
    if (slot_zero->payload.resource.id.size !=
        static_cast<std::uint64_t>(slot_zero->payload.bytes.size())) {
        add_error(
            result.diagnostics,
            slot_zero->payload.resource.id,
            "gdspaces.embedded-name.incomplete-observation",
            "Embedded-name authority must cover the complete physical slot-0 byte span.");
        return result;
    }
    if (slot_zero->payload.byte_provenance.has_value() &&
        !slot_zero->payload.byte_provenance->valid()) {
        add_error(
            result.diagnostics,
            slot_zero->payload.resource.id,
            "gdspaces.embedded-name.invalid-byte-provenance",
            "Physical slot 0 carries invalid byte provenance and cannot become naming evidence.");
        return result;
    }

    const auto parsed = parse_retained_name_list(
        std::span<const std::byte>{
            slot_zero->payload.bytes.data(), slot_zero->payload.bytes.size()});
    if (parsed.empty()) {
        return result;
    }

    std::vector<EmbeddedNameAlias> aliases;
    aliases.reserve(parsed.size());
    for (const auto& alias : parsed) {
        aliases.push_back(EmbeddedNameAlias(alias.name, alias.source_offset));
    }

    const auto digest = core::Sha256::compute(
        std::span<const std::byte>{
            slot_zero->payload.bytes.data(), slot_zero->payload.bytes.size()}).hex();
    result.observation = EmbeddedNameListObservation(
        expansion.parent.id,
        slot_zero->payload.resource.id,
        digest,
        std::move(aliases));
    if (!result.observation->valid()) {
        add_error(
            result.diagnostics,
            slot_zero->payload.resource.id,
            "gdspaces.embedded-name.invalid-observation",
            "The parsed embedded-name list could not form a valid sealed observation.");
        result.observation.reset();
    }
    return result;
}

EmbeddedNameApplyResult EmbeddedNameEvidenceBuilder::apply(
    ContainerExpansion& expansion,
    const EmbeddedNameListObservation& observation) {
    EmbeddedNameApplyResult result;
    if (!expansion.usable() || !observation.valid()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.embedded-name.apply-invalid",
            "Cannot apply an invalid embedded-name observation or apply to an unusable expansion.");
        return result;
    }
    if (observation.parent_resource() != expansion.parent.id) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.embedded-name.parent-mismatch",
            "Embedded-name observation belongs to a different physical parent resource.");
        return result;
    }

    auto* slot_zero = find_slot(expansion, 0U);
    if (slot_zero == nullptr ||
        slot_zero->payload.resource.id != observation.authority_resource()) {
        add_error(
            result.diagnostics,
            observation.authority_resource(),
            "gdspaces.embedded-name.authority-mismatch",
            "The observed slot-0 authority does not match the current physical child identity.");
        return result;
    }
    const auto current_digest = core::Sha256::compute(
        std::span<const std::byte>{
            slot_zero->payload.bytes.data(), slot_zero->payload.bytes.size()}).hex();
    if (current_digest != observation.authority_sha256()) {
        add_error(
            result.diagnostics,
            slot_zero->payload.resource.id,
            "gdspaces.embedded-name.authority-bytes-changed",
            "Physical slot-0 bytes changed after the embedded-name observation was sealed.");
        return result;
    }

    struct StagedEvidence final {
        ContainerChild* child{};
        ResourceNameEvidence evidence;
    };
    std::vector<StagedEvidence> staged;
    staged.reserve(observation.aliases().size());

    for (std::size_t index = 0U; index < observation.aliases().size(); ++index) {
        if (index >= static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max() - 1U)) {
            add_error(
                result.diagnostics,
                expansion.parent.id,
                "gdspaces.embedded-name.slot-overflow",
                "Embedded alias sequence exceeds the physical slot-index range.");
            return result;
        }
        const auto target_slot = static_cast<std::uint32_t>(index + 1U);
        auto* child = find_slot(expansion, target_slot);
        if (child == nullptr) {
            add_warning(
                result.diagnostics,
                expansion.parent.id,
                "gdspaces.embedded-name.alias-without-slot",
                "An embedded alias has no corresponding physical slot and remains unapplied.");
            continue;
        }

        const auto& alias = observation.aliases()[index];
        ResourceNameEvidence evidence(
            ResourceNameEvidenceKind::embedded_alias,
            ResourceNameMappingMode::embedded_alias_sequence,
            observation.authority_resource(),
            std::string{observation.authority_sha256()},
            std::string{alias.name()},
            std::string{alias.name()},
            target_slot,
            std::nullopt,
            alias.source_offset());
        if (!evidence.valid()) {
            add_error(
                result.diagnostics,
                child->payload.resource.id,
                "gdspaces.embedded-name.evidence-invalid",
                "The sealed slot-0 observation could not produce valid embedded-alias evidence.");
            return result;
        }
        staged.push_back(StagedEvidence{
            .child = child,
            .evidence = std::move(evidence),
        });
    }

    // Slot 0 itself is now structurally proven to be the embedded name-list
    // authority. This semantic label remains outside ResourceId/write authority.
    slot_zero->payload.resource.format = "name-list";
    slot_zero->payload.resource.container = false;
    if (const auto* index_evidence = external_index_evidence(slot_zero->payload)) {
        std::string stem;
        std::string extension;
        split_name(index_evidence->normalized_name(), stem, extension);
        slot_zero->payload.resource.display_name = make_display_name(stem, "txt");
        slot_zero->payload.resource.synthetic_name = false;
    } else {
        slot_zero->payload.resource.display_name =
            synthetic_name_list_display(expansion);
        slot_zero->payload.resource.synthetic_name = true;
    }

    for (auto& item : staged) {
        auto& evidence = item.child->payload.name_evidence;
        evidence.erase(
            std::remove_if(
                evidence.begin(), evidence.end(),
                [](const ResourceNameEvidence& existing) {
                    return existing.kind() ==
                        ResourceNameEvidenceKind::embedded_alias;
                }),
            evidence.end());
        const auto alias_name = std::string{item.evidence.normalized_name()};
        evidence.push_back(std::move(item.evidence));

        // External .index is the canonical stronger display stem. Embedded
        // aliases remain queryable evidence but must not override it.
        if (external_index_evidence(item.child->payload) == nullptr) {
            item.child->payload.resource.display_name =
                alias_display_name(item.child->payload, alias_name);
            item.child->payload.resource.synthetic_name = false;
        }
    }

    result.applied = true;
    return result;
}

} // namespace dmc::rengine::gdspaces
