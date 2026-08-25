#include "dmc_rengine/profiles/dmc3/relative_slot_path_reflow_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] RelativeSlotPathReflowResult failure(
    RelativeSlotPathReflowStatus status,
    std::string detail) {
    return RelativeSlotPathReflowResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

struct LevelBuild final {
    RelativeSlotPathReflowStatus status{RelativeSlotPathReflowStatus::invalid_root};
    std::vector<std::byte> bytes;
    std::vector<RelativeSlotPathLevelReceipt> levels;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RelativeSlotPathReflowStatus::ok;
    }
};

[[nodiscard]] LevelBuild level_failure(
    RelativeSlotPathReflowStatus status,
    std::string detail) {
    return LevelBuild{
        .status = status,
        .bytes = {},
        .levels = {},
        .detail = std::move(detail),
    };
}

[[nodiscard]] const RelativeSlotPackedSpanReceipt* changed_span(
    const RelativeSlotPackedReflowReceipt& receipt) noexcept {
    const auto found = std::find_if(
        receipt.spans.begin(), receipt.spans.end(),
        [](const RelativeSlotPackedSpanReceipt& span) {
            return span.changed;
        });
    if (found == receipt.spans.end()) {
        return nullptr;
    }
    const auto next = std::find_if(
        std::next(found), receipt.spans.end(),
        [](const RelativeSlotPackedSpanReceipt& span) {
            return span.changed;
        });
    return next == receipt.spans.end() ? &*found : nullptr;
}

[[nodiscard]] LevelBuild rebuild_level(
    const gdspaces::ResourcePayload& parent,
    std::span<const unsigned int> slot_path,
    std::size_t depth,
    std::span<const std::byte> replacement_bytes,
    RelativeSlotPackedReflowSafety safety) {
    const auto registry = make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    if (!parsed.ok() ||
        (parsed.document.format != "PAC" && parsed.document.format != "PNST")) {
        return level_failure(
            RelativeSlotPathReflowStatus::parse_failed,
            "Slot-path reflow requires PAC/PNST at every traversed level.");
    }

    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    if (!expansion.usable()) {
        return level_failure(
            RelativeSlotPathReflowStatus::expansion_failed,
            "Canonical container expansion failed for a traversed level.");
    }

    const auto slot = slot_path[depth];
    if (slot >= expansion.children.size()) {
        return level_failure(
            RelativeSlotPathReflowStatus::slot_out_of_range,
            "Slot-path component is outside the declared slot table.");
    }
    const auto& child = expansion.children[slot];
    if (!child.entry.populated || !child.payload.readable()) {
        return level_failure(
            RelativeSlotPathReflowStatus::empty_slot,
            "Slot-path component refers to an empty or unreadable slot.");
    }

    std::vector<std::byte> authored_bytes;
    std::vector<RelativeSlotPathLevelReceipt> nested_levels;
    if (depth + 1U == slot_path.size()) {
        authored_bytes.assign(replacement_bytes.begin(), replacement_bytes.end());
    } else {
        const auto nested = rebuild_level(
            child.payload, slot_path, depth + 1U, replacement_bytes, safety);
        if (!nested.ok()) {
            return level_failure(
                RelativeSlotPathReflowStatus::nested_rebuild_failed,
                "Nested slot-path rebuild failed: " + nested.detail);
        }
        authored_bytes = nested.bytes;
        nested_levels = nested.levels;
    }

    const auto source_sha = sha256_of(
        std::span<const std::byte>{child.payload.bytes.data(), child.payload.bytes.size()});
    const auto output_sha = sha256_of(
        std::span<const std::byte>{authored_bytes.data(), authored_bytes.size()});
    const AuthoredChildImage authored{
        .resource = child.payload.resource.id,
        .source_sha256 = source_sha,
        .output_sha256 = output_sha,
        .revision = 1U,
        .writer_mode = "slot-path-reflow",
        .bytes = authored_bytes,
    };
    const std::array<AuthoredChildImage, 1U> edits{authored};
    const auto rebuilt = RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, edits, safety);
    if (!rebuilt.ok()) {
        return level_failure(
            RelativeSlotPathReflowStatus::parent_reflow_failed,
            "Parent packed reflow failed: " + rebuilt.detail);
    }

    RelativeSlotPathLevelReceipt level{
        .parent = parent.resource.id,
        .slot_index = slot,
        .parser_format = expansion.parser_format,
        .source_sha256 = rebuilt.receipt->source_sha256,
        .output_sha256 = rebuilt.receipt->output_sha256,
        .reflow = *rebuilt.receipt,
    };

    std::vector<RelativeSlotPathLevelReceipt> levels;
    levels.reserve(1U + nested_levels.size());
    levels.push_back(std::move(level));
    levels.insert(
        levels.end(),
        std::make_move_iterator(nested_levels.begin()),
        std::make_move_iterator(nested_levels.end()));

    return LevelBuild{
        .status = RelativeSlotPathReflowStatus::ok,
        .bytes = rebuilt.bytes,
        .levels = std::move(levels),
        .detail = {},
    };
}

} // namespace

bool RelativeSlotPathLevelReceipt::valid() const {
    return parent.valid() &&
        (parser_format == "PAC" || parser_format == "PNST") &&
        source_sha256.size() == 64U && output_sha256.size() == 64U &&
        reflow.valid() && reflow.parent == parent &&
        reflow.source_sha256 == source_sha256 &&
        reflow.output_sha256 == output_sha256 &&
        reflow.source_topology.format == parser_format;
}

bool RelativeSlotPathReflowReceipt::valid() const {
    if (!root.valid() || slot_path.empty() ||
        source_sha256.size() != 64U || replacement_sha256.size() != 64U ||
        output_sha256.size() != 64U || levels.size() != slot_path.size() ||
        levels.empty() || !levels.front().valid() ||
        levels.front().parent != root ||
        levels.front().source_sha256 != source_sha256 ||
        levels.front().output_sha256 != output_sha256) {
        return false;
    }

    for (std::size_t index = 0U; index < levels.size(); ++index) {
        const auto& level = levels[index];
        if (!level.valid() || level.slot_index != slot_path[index]) {
            return false;
        }
        const auto* span = changed_span(level.reflow);
        if (span == nullptr || span->authored_aliases.empty()) {
            return false;
        }
        if (index + 1U < levels.size()) {
            const auto& nested = levels[index + 1U];
            if (span->output_sha256 != nested.output_sha256 ||
                std::find(
                    span->authored_aliases.begin(), span->authored_aliases.end(),
                    nested.parent) == span->authored_aliases.end()) {
                return false;
            }
        } else if (span->output_sha256 != replacement_sha256) {
            return false;
        }
    }
    return true;
}

bool RelativeSlotPathReflowResult::ok() const {
    return status == RelativeSlotPathReflowStatus::ok && receipt.has_value() &&
        receipt->valid() && receipt->output_sha256 == sha256_of(
            std::span<const std::byte>{bytes.data(), bytes.size()});
}

RelativeSlotPathReflowResult RelativeSlotPathReflowWriter::rebuild(
    const gdspaces::ResourcePayload& root,
    std::span<const unsigned int> slot_path,
    std::span<const std::byte> replacement_bytes,
    RelativeSlotPackedReflowSafety safety) {
    if (!root.readable() ||
        root.resource.id.size != static_cast<std::uint64_t>(root.bytes.size()) ||
        replacement_bytes.empty() || safety.max_output_bytes == 0U) {
        return failure(
            RelativeSlotPathReflowStatus::invalid_root,
            "Slot-path reflow requires an exact readable root, non-empty replacement and non-zero output budget.");
    }
    if (slot_path.empty()) {
        return failure(
            RelativeSlotPathReflowStatus::empty_slot_path,
            "Slot-path reflow requires at least one slot component.");
    }

    const auto built = rebuild_level(
        root, slot_path, 0U, replacement_bytes, safety);
    if (!built.ok()) {
        return failure(built.status, built.detail);
    }

    RelativeSlotPathReflowReceipt receipt{
        .root = root.resource.id,
        .slot_path = std::vector<unsigned int>{slot_path.begin(), slot_path.end()},
        .source_sha256 = sha256_of(
            std::span<const std::byte>{root.bytes.data(), root.bytes.size()}),
        .replacement_sha256 = sha256_of(replacement_bytes),
        .output_sha256 = sha256_of(
            std::span<const std::byte>{built.bytes.data(), built.bytes.size()}),
        .levels = built.levels,
    };
    if (!receipt.valid()) {
        return failure(
            RelativeSlotPathReflowStatus::invalid_receipt,
            "Nested slot-path reflow produced an internally inconsistent receipt chain.");
    }

    RelativeSlotPathReflowResult result{
        .status = RelativeSlotPathReflowStatus::ok,
        .bytes = built.bytes,
        .receipt = std::move(receipt),
        .detail = {},
    };
    if (!result.ok()) {
        return failure(
            RelativeSlotPathReflowStatus::invalid_receipt,
            "Nested slot-path reflow output does not match its receipt SHA-256.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
