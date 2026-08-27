#include "dmc_rengine/formats/effect_pack.hpp"

#include "dmc_rengine/formats/relative_slot_container.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <algorithm>
#include <charconv>
#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::EffectPackContract;
using Walk = profiles::dmc3::RelativeSlotWalkContract;

[[nodiscard]] EffectPackParseResult fail(
    EffectPackParseError error, std::string message) {
    return EffectPackParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] RelativeSlotContainerSpec pnst_spec() {
    RelativeSlotContainerSpec spec;
    for (std::size_t index = 0U; index < Walk::pnst_magic_bytes; ++index) {
        spec.magic[index] = static_cast<std::byte>(Walk::pnst_magic[index]);
    }
    spec.magic_bytes = Walk::pnst_magic_bytes;
    spec.document_format = "pnst";
    return spec;
}

// The manifest is ASCII, CRLF-terminated and NUL-padded to its slot. Anything
// outside that is not a manifest, and reading it as one would be inventing the
// names this format exists to stop inventing.
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
};

// Returns false on a line that is present but not of the form the format uses.
// A comment is skipped; a blank line is skipped; anything else must parse.
[[nodiscard]] bool collect_lines(
    const std::string& text, std::vector<ManifestLine>& lines) {
    std::size_t at = 0U;
    while (at <= text.size()) {
        auto end = text.find('\n', at);
        if (end == std::string::npos) {
            end = text.size();
        }
        auto line = text.substr(at, end - at);
        at = end + 1U;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty() || line.front() == Contract::comment_prefix) {
            if (at > text.size()) {
                break;
            }
            continue;
        }
        const auto space = line.find(Contract::field_separator);
        if (space != 1U || line.size() < 3U) {
            return false;
        }
        std::uint32_t identifier = 0U;
        const auto* first = line.data() + 2;
        const auto* last = line.data() + line.size();
        const auto parsed = std::from_chars(first, last, identifier);
        if (parsed.ec != std::errc{} || parsed.ptr != last) {
            return false;
        }
        lines.push_back(ManifestLine{
            .kind = line.front(),
            .identifier = identifier,
            .text = line,
        });
        if (at > text.size()) {
            break;
        }
    }
    return true;
}

} // namespace

bool EffectPackDocument::valid() const noexcept {
    return document_size != 0U && manifest_names_every_slot &&
        records.size() == record_slot_count &&
        record_slot_count == manifest_line_count;
}

EffectPackParseResult EffectPackParser::parse(std::span<const std::byte> bytes) {
    const auto spec = pnst_spec();
    const auto outer = parse_relative_slot_container(bytes, spec);
    if (!outer.ok()) {
        return fail(
            EffectPackParseError::not_a_container,
            "effect pack is not a PNST container");
    }
    const auto& outer_entries = outer.document->entries;
    if (outer_entries.size() != Contract::outer_slot_count) {
        return fail(
            EffectPackParseError::wrong_outer_slot_count,
            "an effect pack has exactly two slots: a manifest and its records");
    }

    const auto& manifest_entry = outer_entries[Contract::manifest_slot_index];
    const auto& records_entry = outer_entries[Contract::records_slot_index];
    if (!manifest_entry.populated || !records_entry.populated) {
        return fail(
            EffectPackParseError::manifest_missing,
            "an effect pack's manifest or record slot is absent");
    }

    const auto manifest_bytes = bytes.subspan(
        static_cast<std::size_t>(manifest_entry.offset),
        static_cast<std::size_t>(manifest_entry.size));
    if (!manifest_is_text(manifest_bytes)) {
        return fail(
            EffectPackParseError::manifest_not_text,
            "an effect pack's first slot is not an ASCII manifest");
    }

    EffectPackDocument document;
    document.document_size = static_cast<std::uint64_t>(bytes.size());
    document.manifest_text = manifest_text_of(manifest_bytes);

    std::vector<ManifestLine> lines;
    if (!collect_lines(document.manifest_text, lines)) {
        return fail(
            EffectPackParseError::malformed_manifest_line,
            "a manifest line is not a kind letter followed by an identifier");
    }
    if (lines.empty() || lines.size() > k_max_records) {
        return fail(
            EffectPackParseError::malformed_manifest_line,
            "an effect pack's manifest names no records, or too many");
    }

    const auto records_bytes = bytes.subspan(
        static_cast<std::size_t>(records_entry.offset),
        static_cast<std::size_t>(records_entry.size));
    const auto inner = parse_relative_slot_container(records_bytes, spec);
    if (!inner.ok()) {
        return fail(
            EffectPackParseError::not_a_container,
            "an effect pack's record slot is not a PNST container");
    }

    std::vector<ContainerEntry> populated;
    for (const auto& entry : inner.document->entries) {
        if (entry.populated) {
            populated.push_back(entry);
        }
    }

    document.manifest_line_count = static_cast<std::uint32_t>(lines.size());
    document.record_slot_count = static_cast<std::uint32_t>(populated.size());
    if (document.manifest_line_count != document.record_slot_count) {
        return fail(
            EffectPackParseError::line_count_mismatch,
            "the manifest does not name exactly one record per slot");
    }
    document.manifest_names_every_slot = true;

    bool extents_match = true;
    document.records.reserve(populated.size());
    for (std::size_t index = 0U; index < populated.size(); ++index) {
        const auto& entry = populated[index];
        const auto& line = lines[index];
        const auto expected = Contract::extent_for(line.kind);
        const bool matches =
            expected != 0U && entry.size == static_cast<std::uint64_t>(expected);
        if (expected != 0U && !matches) {
            extents_match = false;
        }
        document.records.push_back(EffectRecord{
            .slot_index = entry.slot_index,
            .kind = line.kind,
            .identifier = line.identifier,
            .name = line.text,
            .offset = records_entry.offset + entry.offset,
            .extent = entry.size,
            .extent_matches_kind = matches,
            .kind_known = Contract::is_known_kind(line.kind),
        });
    }
    document.extents_match_kinds = extents_match;

    if (!document.valid()) {
        return fail(
            EffectPackParseError::invalid_document,
            "effect pack decoded to an inconsistent document");
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
