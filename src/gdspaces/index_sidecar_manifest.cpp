#include "dmc_rengine/gdspaces/index_sidecar_manifest.hpp"

#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <string_view>

namespace dmc::rengine::gdspaces {
namespace {

using Walk = profiles::dmc3::RelativeSlotWalkContract;

[[nodiscard]] std::optional<std::vector<std::string>> lines_of(
    std::span<const std::byte> bytes) {
    if (bytes.empty() || bytes.size() > IndexSidecarManifest::k_max_bytes) {
        return std::nullopt;
    }
    std::vector<std::string> lines;
    std::string current;
    for (const auto value : bytes) {
        const auto raw = std::to_integer<unsigned char>(value);
        if (raw == 0U) {
            break;
        }
        if (raw == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }
        if (raw == '\r') {
            continue;
        }
        // Anything outside printable ASCII means this is not a text index.
        if (raw < 0x20U || raw > 0x7EU) {
            return std::nullopt;
        }
        if (current.size() > IndexSidecarManifest::k_max_name_bytes) {
            return std::nullopt;
        }
        current.push_back(static_cast<char>(raw));
    }
    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

[[nodiscard]] bool is_container_directive(std::string_view line) {
    return line == Walk::pac_magic || line == Walk::pnst_magic;
}

} // namespace

bool IndexSidecarManifest::is_own_rendered_sidecar(
    std::span<const std::byte> bytes) noexcept {
    // Our renderer emits tab-separated columns; an extraction tool's file is
    // one plain name per line. One tab anywhere is enough to tell them apart.
    for (const auto value : bytes) {
        if (std::to_integer<unsigned char>(value) == '\t') {
            return true;
        }
    }
    return false;
}

std::optional<IndexSidecarManifest::Document> IndexSidecarManifest::parse(
    std::span<const std::byte> bytes) {
    if (is_own_rendered_sidecar(bytes)) {
        return std::nullopt;
    }
    const auto lines = lines_of(bytes);
    if (!lines.has_value() || lines->size() < 2U) {
        return std::nullopt;
    }
    if (!is_container_directive((*lines)[k_directive_line])) {
        return std::nullopt;
    }

    Document document;
    document.container_directive = (*lines)[k_directive_line];
    for (std::size_t line = k_directive_line + 1U; line < lines->size(); ++line) {
        const auto& text = (*lines)[line];
        // A blank trailing line is padding, not a slot. A blank line between
        // names would shift every slot after it, so it ends the read instead.
        if (text.empty()) {
            break;
        }
        if (document.entries.size() >= k_max_names) {
            return std::nullopt;
        }
        document.entries.push_back(Entry{
            .slot_index = slot_for_line(line),
            .name = text,
            .source_line = line,
        });
    }
    if (document.entries.empty()) {
        return std::nullopt;
    }
    return document;
}

} // namespace dmc::rengine::gdspaces
