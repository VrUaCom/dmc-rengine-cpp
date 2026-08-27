#include "dmc_rengine/gdspaces/index_manifest.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <cctype>
#include <span>
#include <string_view>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string_view trim_ascii(std::string_view value) noexcept {
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1U);
    }
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1U);
    }
    return value;
}

[[nodiscard]] bool text_byte_allowed(std::byte value) noexcept {
    const auto character = std::to_integer<unsigned char>(value);
    if (character == static_cast<unsigned char>('\t') ||
        character == static_cast<unsigned char>('\r') ||
        character == static_cast<unsigned char>('\n')) {
        return true;
    }
    return character >= 0x20U && character <= 0x7EU;
}

[[nodiscard]] bool strip_folder_marker(
    std::string_view input,
    std::string_view& name) noexcept {
    auto trimmed = trim_ascii(input);
    constexpr std::string_view marker = "folder";
    if (trimmed.size() < marker.size()) {
        name = trimmed;
        return false;
    }

    const auto suffix = trimmed.substr(trimmed.size() - marker.size());
    if (suffix != marker) {
        name = trimmed;
        return false;
    }

    const auto prefix_size = trimmed.size() - marker.size();
    if (prefix_size == 0U ||
        std::isspace(static_cast<unsigned char>(trimmed[prefix_size - 1U])) == 0) {
        name = trimmed;
        return false;
    }

    name = trim_ascii(trimmed.substr(0U, prefix_size));
    return true;
}

void split_name(
    std::string_view name,
    std::string& stem,
    std::optional<std::string>& extension) {
    const auto separator = name.find_last_of("/\\");
    const auto dot = name.find_last_of('.');
    if (dot == std::string_view::npos ||
        (separator != std::string_view::npos && dot <= separator) ||
        dot + 1U >= name.size()) {
        stem.assign(name);
        extension.reset();
        return;
    }

    stem.assign(name.substr(0U, dot));
    extension = std::string{name.substr(dot + 1U)};
}

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

void add_error(
    IndexManifestParseResult& result,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

} // namespace

IndexManifest::IndexManifest(
    ResourceId source,
    std::string observed_sha256,
    IndexContainerDirective directive,
    std::vector<IndexManifestEntry> entries)
    : source_(std::move(source)),
      observed_sha256_(std::move(observed_sha256)),
      directive_(directive),
      entries_(std::move(entries)) {}

const ResourceId& IndexManifest::source() const noexcept {
    return source_;
}

std::string_view IndexManifest::observed_sha256() const noexcept {
    return observed_sha256_;
}

IndexContainerDirective IndexManifest::directive() const noexcept {
    return directive_;
}

const std::vector<IndexManifestEntry>& IndexManifest::entries() const noexcept {
    return entries_;
}

bool IndexManifest::valid() const noexcept {
    if (!source_.valid() || !valid_digest(observed_sha256_) ||
        entries_.empty()) {
        return false;
    }
    return std::all_of(
        entries_.begin(), entries_.end(),
        [](const IndexManifestEntry& entry) {
            return !entry.name.empty() && !entry.stem.empty() &&
                   entry.line_number > 0U;
        });
}

bool IndexManifestParseResult::ok() const noexcept {
    if (!manifest.has_value() || !manifest->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

IndexManifestParseResult IndexManifestParser::parse(
    const ResourcePayload& index_payload) {
    IndexManifestParseResult result;

    if (!index_payload.readable()) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.unreadable",
            "The .index resource payload is not readable.");
        return result;
    }

    if (index_payload.bytes.empty() ||
        index_payload.resource.id.size !=
            static_cast<std::uint64_t>(index_payload.bytes.size())) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.incomplete-observation",
            "The .index authority observation must cover the complete declared resource byte span.");
        return result;
    }

    if (index_payload.byte_provenance.has_value() &&
        !index_payload.byte_provenance->valid()) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.invalid-byte-provenance",
            "The .index resource carries invalid byte provenance and cannot become name authority.");
        return result;
    }

    const auto source_classification = ResourceClassifier::classify(
        index_payload.resource.id.logical_path,
        std::span<const std::byte>{
            index_payload.bytes.data(), index_payload.bytes.size()});
    if (source_classification.format != "index" ||
        source_classification.magic_confirmed) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.wrong-resource-kind",
            "Only a path-classified .index text resource may become companion name authority.");
        return result;
    }

    if (!std::all_of(
            index_payload.bytes.begin(), index_payload.bytes.end(),
            text_byte_allowed)) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.binary-like",
            "The .index payload contains unsupported control bytes and is not accepted as name authority.");
        return result;
    }

    const auto digest = core::Sha256::compute(
        std::span<const std::byte>{
            index_payload.bytes.data(), index_payload.bytes.size()}).hex();
    const std::string text{
        reinterpret_cast<const char*>(index_payload.bytes.data()),
        index_payload.bytes.size()};

    IndexContainerDirective directive = IndexContainerDirective::none;
    std::vector<IndexManifestEntry> entries;
    std::size_t line_number = 0U;
    std::size_t cursor = 0U;

    while (cursor <= text.size()) {
        ++line_number;
        const auto end = text.find('\n', cursor);
        auto raw_line = end == std::string::npos
            ? std::string_view{text}.substr(cursor)
            : std::string_view{text}.substr(cursor, end - cursor);
        if (!raw_line.empty() && raw_line.back() == '\r') {
            raw_line.remove_suffix(1U);
        }
        const auto trimmed = trim_ascii(raw_line);

        if (!trimmed.empty()) {
            if (entries.empty() &&
                directive == IndexContainerDirective::none &&
                trimmed == "PNST") {
                directive = IndexContainerDirective::pnst_non_empty_slots;
            } else {
                IndexManifestEntry entry;
                entry.raw.assign(raw_line);
                std::string_view name_view;
                entry.is_folder = strip_folder_marker(trimmed, name_view);
                entry.name.assign(name_view);
                split_name(entry.name, entry.stem, entry.extension);
                entry.line_number = line_number;
                if (entry.name.empty()) {
                    add_error(
                        result,
                        index_payload.resource.id,
                        "gdspaces.index.empty-name",
                        "A non-empty .index line does not contain a usable name after structural markers are removed.");
                    return result;
                }
                entries.push_back(std::move(entry));
            }
        }

        if (end == std::string::npos) {
            break;
        }
        cursor = end + 1U;
    }

    if (entries.empty()) {
        add_error(
            result,
            index_payload.resource.id,
            "gdspaces.index.empty-manifest",
            "The .index resource does not contain any usable member names.");
        return result;
    }

    result.manifest = IndexManifest(
        index_payload.resource.id,
        digest,
        directive,
        std::move(entries));
    return result;
}

} // namespace dmc::rengine::gdspaces
