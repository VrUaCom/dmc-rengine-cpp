#include "dmc_rengine/formats/effect_pack.hpp"

#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::EffectPackContract;

[[nodiscard]] EffectPackParseResult fail(
    EffectPackParseError error, std::string message) {
    return EffectPackParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] const ContainerEntry* entry_at_slot(
    const ContainerDocument& document,
    std::uint32_t slot) noexcept {
    const auto found = std::find_if(
        document.entries.begin(), document.entries.end(),
        [slot](const ContainerEntry& entry) {
            return entry.slot_index == slot;
        });
    return found == document.entries.end() ? nullptr : &*found;
}

[[nodiscard]] bool manifest_is_text(std::span<const std::byte> bytes) noexcept {
    bool saw_text = false;
    for (const auto value : bytes) {
        const auto raw = std::to_integer<unsigned char>(value);
        if (raw == 0U) {
            break;
        }
        if (raw != '\r' && raw != '\n' && (raw < 0x20U || raw > 0x7EU)) {
            return false;
        }
        saw_text = true;
    }
    return saw_text;
}

[[nodiscard]] std::string manifest_text_of(std::span<const std::byte> bytes) {
    std::string text;
    for (const auto value : bytes) {
        const auto raw = std::to_integer<unsigned char>(value);
        if (raw == 0U) {
            break;
        }
        text.push_back(static_cast<char>(raw));
    }
    return text;
}

struct ManifestLine final {
    char kind{};
    std::uint32_t identifier{};
    std::string text;
    std::size_t source_line{};
};

[[nodiscard]] bool collect_lines(
    const std::string& text,
    std::vector<ManifestLine>& lines) {
    std::size_t at = 0U;
    std::size_t physical_line = 0U;
    while (at <= text.size()) {
        ++physical_line;
        auto end = text.find('\n', at);
        if (end == std::string::npos) {
            end = text.size();
        }
        const auto raw = std::string_view{text}.substr(at, end - at);
        at = end + 1U;

        // The grammar is the contract's, not this file's. It was here once,
        // and the classifier needed the same rule to tell a manifest slot from
        // any other text — two copies of a grammar is how a slot comes to be a
        // manifest to one reader and not to the other.
        const auto read = Contract::read_manifest_line(raw);
        if (read.line_kind == Contract::ManifestLineKind::invalid) {
            return false;
        }
        if (!read.is_record()) {
            if (at > text.size()) {
                break;
            }
            continue;
        }

        lines.push_back(ManifestLine{
            .kind = read.kind,
            .identifier = read.identifier,
            .text = std::string{Contract::trim_manifest_line(raw)},
            .source_line = physical_line,
        });
        if (at > text.size()) {
            break;
        }
    }
    return true;
}

} // namespace

bool EffectPackDocument::valid() const noexcept {
    // The manifest names every record that is not a companion, and the two
    // counts add up to what the container holds. The earlier invariant was
    // `populated_record_count == manifest_line_count`, which is the same
    // statement only for a pack with no companions — and refused every pack
    // that has one.
    return document_size != 0U && manifest_names_every_populated_record &&
        records.size() == manifest_line_count &&
        manifest_line_count + companion_record_count == populated_record_count;
}

EffectPackParseResult EffectPackParser::parse(std::span<const std::byte> bytes) {
    const auto outer = PnstParser::parse(bytes);
    if (!outer.ok()) {
        return fail(
            EffectPackParseError::not_a_container,
            "effect pack is not a valid PNST container");
    }
    if (outer.document->entries.size() != Contract::outer_slot_count ||
        outer.document->declared_slot_count != Contract::outer_slot_count) {
        return fail(
            EffectPackParseError::wrong_outer_slot_count,
            "effect pack must have exactly manifest slot 0 and records slot 1");
    }

    const auto* manifest_entry = entry_at_slot(
        *outer.document, static_cast<std::uint32_t>(Contract::manifest_slot_index));
    const auto* records_entry = entry_at_slot(
        *outer.document, static_cast<std::uint32_t>(Contract::records_slot_index));
    if (manifest_entry == nullptr || records_entry == nullptr ||
        !manifest_entry->populated || !records_entry->populated) {
        return fail(
            EffectPackParseError::manifest_missing,
            "effect pack manifest or record container is absent");
    }

    const auto manifest_bytes = bytes.subspan(
        static_cast<std::size_t>(manifest_entry->offset),
        static_cast<std::size_t>(manifest_entry->size));
    if (!manifest_is_text(manifest_bytes)) {
        return fail(
            EffectPackParseError::manifest_not_text,
            "effect pack slot 0 is not an ASCII manifest");
    }

    EffectPackDocument document;
    document.document_size = static_cast<std::uint64_t>(bytes.size());
    document.manifest_text = manifest_text_of(manifest_bytes);

    std::vector<ManifestLine> lines;
    if (!collect_lines(document.manifest_text, lines)) {
        return fail(
            EffectPackParseError::malformed_manifest_line,
            "effect manifest line is not a kind letter followed by a decimal identifier");
    }
    if (lines.empty() || lines.size() > k_max_records) {
        return fail(
            EffectPackParseError::malformed_manifest_line,
            "effect manifest names no records or exceeds the safety limit");
    }

    const auto records_bytes = bytes.subspan(
        static_cast<std::size_t>(records_entry->offset),
        static_cast<std::size_t>(records_entry->size));
    const auto inner = PnstParser::parse(records_bytes);
    if (!inner.ok()) {
        return fail(
            EffectPackParseError::not_a_container,
            "effect pack records slot is not a valid PNST container");
    }

    std::vector<ContainerEntry> populated;
    for (const auto& entry : inner.document->entries) {
        if (entry.populated) {
            populated.push_back(entry);
        }
    }
    std::sort(
        populated.begin(), populated.end(),
        [](const ContainerEntry& left, const ContainerEntry& right) {
            return left.slot_index < right.slot_index;
        });

    // A pack may hold records the manifest does not name. This reader required
    // the two counts to be equal, which was true of the two packs it was
    // recovered from and is not a property of the format: the em000 pack holds
    // 183 populated records against 173 manifest lines, and requiring equality
    // refused the whole pack — so all 173 named records lost their names over
    // ten the manifest never claimed.
    //
    // The ten are byte-identical companions. Setting those aside, line k names
    // record k for all 173, kind and identifier, in order.
    std::vector<ContainerEntry> named;
    named.reserve(populated.size());
    for (const auto& entry : populated) {
        const auto record = records_bytes.subspan(
            static_cast<std::size_t>(entry.offset),
            static_cast<std::size_t>(entry.size));
        if (Contract::is_companion_record(record)) {
            document.companion_record_count += 1U;
            continue;
        }
        named.push_back(entry);
    }

    document.manifest_line_count = static_cast<std::uint32_t>(lines.size());
    document.populated_record_count = static_cast<std::uint32_t>(populated.size());
    if (document.manifest_line_count != named.size()) {
        return fail(
            EffectPackParseError::line_count_mismatch,
            "effect manifest does not name exactly one entry per named record payload");
    }
    document.manifest_names_every_populated_record = true;
    populated = std::move(named);

    bool extents_match = true;
    document.records.reserve(populated.size());
    for (std::size_t ordinal = 0U; ordinal < populated.size(); ++ordinal) {
        const auto& entry = populated[ordinal];
        const auto& line = lines[ordinal];
        const auto expected = Contract::extent_for(line.kind);
        const bool matches = expected != 0U &&
            entry.size == static_cast<std::uint64_t>(expected);
        if (expected != 0U && !matches) {
            extents_match = false;
        }
        document.records.push_back(EffectRecord{
            .slot_index = entry.slot_index,
            .kind = line.kind,
            .identifier = line.identifier,
            .name = line.text,
            .source_line = line.source_line,
            .offset = records_entry->offset + entry.offset,
            .extent = entry.size,
            .extent_matches_kind = matches,
            .kind_known = Contract::is_known_kind(line.kind),
        });
    }
    document.extents_match_known_kinds = extents_match;

    if (!document.valid()) {
        return fail(
            EffectPackParseError::invalid_document,
            "effect pack decoded to an inconsistent stored-name document");
    }

    return EffectPackParseResult{
        .document = std::move(document),
        .error = EffectPackParseError::none,
        .message = {},
    };
}

bool EffectPackParser::structurally_valid(
    std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats
