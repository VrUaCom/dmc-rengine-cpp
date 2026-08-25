#include "dmc_rengine/profiles/dmc3/relative_slot_path_reflow_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <span>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

struct RecursiveResult final {
    RelativeSlotPathReflowStatus status{RelativeSlotPathReflowStatus::invalid_root};
    std::vector<std::byte> bytes;
    std::vector<RelativeSlotPathLevelReceipt> levels;
    std::string detail;
};

[[nodiscard]] RecursiveResult rebuild_level(
    const gdspaces::ResourcePayload& parent,
    std::span<const unsigned int> slot_path,
    std::span<const std::byte> replacement_bytes,
    RelativeSlotPackedReflowSafety safety) {
    const auto parsers = make_container_parser_registry();
    const auto parsed = parsers.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    if (!parsed.ok() ||
        (parsed.document.format != "PAC" && parsed.document.format != "PNST")) {
        return {
            .status = RelativeSlotPathReflowStatus::parse_failed,
            .detail = "path parent is not a canonical PAC/PNST relative-slot container",
        };
    }

    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    if (!expansion.usable()) {
        return {
            .status = RelativeSlotPathReflowStatus::expansion_failed,
            .detail = "path parent expansion is unusable",
        };
    }

    const auto slot_index = slot_path.front();
    if (slot_index >= expansion.children.size()) {
        return {
            .status = RelativeSlotPathReflowStatus::slot_out_of_range,
            .detail = "slot path index is outside parent topology",
        };
    }

    const auto& child = expansion.children[slot_index];
    if (!child.entry.populated || !child.payload.readable()) {
        return {
            .status = RelativeSlotPathReflowStatus::empty_slot,
            .detail = "slot path addresses an empty or unreadable child",
        };
    }

    std::vector<std::byte> authored_bytes;
    std::vector<RelativeSlotPathLevelReceipt> nested_levels;
    if (slot_path.size() == 1U) {
        authored_bytes.assign(replacement_bytes.begin(), replacement_bytes.end());
    } else {
        auto nested = rebuild_level(
            child.payload,
            slot_path.subspan(1U),
            replacement_bytes,
            safety);
        if (nested.status != RelativeSlotPathReflowStatus::ok) {
            return nested;
        }
        authored_bytes = std::move(nested.bytes);
        nested_levels = std::move(nested.levels);
    }

    AuthoredChildImage authored{
        .resource = child.payload.resource.id,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            authored_bytes.data(), authored_bytes.size()}),
        .revision = 1U,
        .writer_mode = slot_path.size() == 1U
            ? "slot-path-leaf-image"
            : "slot-path-nested-image",
        .bytes = authored_bytes,
    };
    const std::vector<AuthoredChildImage> authored_children{std::move(authored)};
    auto rebuilt = RelativeSlotPackedReflowWriter::rebuild(
        parent,
        expansion,
        authored_children,
        safety);
    if (!rebuilt.ok()) {
        return {
            .status = RelativeSlotPathReflowStatus::parent_reflow_failed,
            .detail = std::string{"parent reflow failed: "} +
                std::string{to_string(rebuilt.status)} +
                (rebuilt.detail.empty() ? std::string{} : ": " + rebuilt.detail),
        };
    }

    RelativeSlotPathLevelReceipt level{
        .parent = parent.resource.id,
        .slot_index = slot_index,
        .parser_format = expansion.parser_format,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            parent.bytes.data(), parent.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            rebuilt.bytes.data(), rebuilt.bytes.size()}),
        .reflow = *rebuilt.receipt,
    };

    std::vector<RelativeSlotPathLevelReceipt> levels;
    levels.reserve(1U + nested_levels.size());
    levels.push_back(std::move(level));
    for (auto& nested_level : nested_levels) {
        levels.push_back(std::move(nested_level));
    }

    return {
        .status = RelativeSlotPathReflowStatus::ok,
        .bytes = std::move(rebuilt.bytes),
        .levels = std::move(levels),
    };
}

[[nodiscard]] const RelativeSlotTopologyEntry* selected_entry(
    const RelativeSlotPathLevelReceipt& level) noexcept {
    const auto iterator = std::find_if(
        level.reflow.source_topology.entries.begin(),
        level.reflow.source_topology.entries.end(),
        [&](const RelativeSlotTopologyEntry& entry) {
            return entry.slot_index == level.slot_index;
        });
    return iterator == level.reflow.source_topology.entries.end()
        ? nullptr
        : &*iterator;
}

[[nodiscard]] const RelativeSlotPackedSpanReceipt* selected_span(
    const RelativeSlotPathLevelReceipt& level) noexcept {
    const auto* entry = selected_entry(level);
    if (entry == nullptr || !entry->populated) {
        return nullptr;
    }
    const auto iterator = std::find_if(
        level.reflow.spans.begin(),
        level.reflow.spans.end(),
        [&](const RelativeSlotPackedSpanReceipt& span) {
            return span.changed && span.source_offset == entry->offset &&
                span.source_size == entry->size;
        });
    return iterator == level.reflow.spans.end() ? nullptr : &*iterator;
}

[[nodiscard]] bool span_rebuilds_child(
    const RelativeSlotPackedSpanReceipt& span,
    const RelativeSlotPathLevelReceipt& child) {
    if (!span.changed || span.source_sha256 != child.source_sha256 ||
        span.output_sha256 != child.output_sha256) {
        return false;
    }
    return std::find(
        span.affected_aliases.begin(),
        span.affected_aliases.end(),
        child.parent) != span.affected_aliases.end();
}

} // namespace

bool RelativeSlotPathLevelReceipt::valid() const {
    return parent.valid() && !parser_format.empty() &&
        !source_sha256.empty() && !output_sha256.empty() && reflow.valid() &&
        reflow.parent == parent && reflow.source_sha256 == source_sha256 &&
        reflow.output_sha256 == output_sha256 &&
        parser_format == reflow.source_topology.format &&
        selected_span(*this) != nullptr;
}

bool RelativeSlotPathReflowReceipt::valid() const {
    if (!root.valid() || slot_path.empty() || levels.size() != slot_path.size() ||
        source_sha256.empty() || replacement_sha256.empty() || output_sha256.empty()) {
        return false;
    }
    for (std::size_t index = 0U; index < levels.size(); ++index) {
        if (!levels[index].valid() || levels[index].slot_index != slot_path[index]) {
            return false;
        }
    }
    if (levels.front().parent != root ||
        levels.front().source_sha256 != source_sha256 ||
        levels.front().output_sha256 != output_sha256) {
        return false;
    }

    for (std::size_t index = 0U; index + 1U < levels.size(); ++index) {
        const auto& parent = levels[index];
        const auto& child = levels[index + 1U];
        const auto* span = selected_span(parent);
        if (span == nullptr || !span_rebuilds_child(*span, child)) {
            return false;
        }
    }

    const auto* leaf_span = selected_span(levels.back());
    return leaf_span != nullptr &&
        leaf_span->output_sha256 == replacement_sha256;
}

bool RelativeSlotPathReflowResult::ok() const {
    return status == RelativeSlotPathReflowStatus::ok && receipt.has_value() &&
        receipt->valid() &&
        sha256_of(std::span<const std::byte>{bytes.data(), bytes.size()}) ==
            receipt->output_sha256;
}

RelativeSlotPathReflowResult RelativeSlotPathReflowWriter::rebuild(
    const gdspaces::ResourcePayload& root,
    std::span<const unsigned int> slot_path,
    std::span<const std::byte> replacement_bytes,
    RelativeSlotPathReflowSafety safety) {
    if (!root.readable()) {
        return {
            .status = RelativeSlotPathReflowStatus::invalid_root,
            .detail = "root payload is unreadable",
        };
    }
    if (slot_path.empty()) {
        return {
            .status = RelativeSlotPathReflowStatus::empty_slot_path,
            .detail = "slot path must address at least one child",
        };
    }
    if (safety.max_depth == 0U || slot_path.size() > safety.max_depth) {
        return {
            .status = RelativeSlotPathReflowStatus::path_too_deep,
            .detail = "slot path exceeds product recursion safety budget",
        };
    }

    auto recursive = rebuild_level(
        root, slot_path, replacement_bytes, safety.parent_reflow);
    if (recursive.status != RelativeSlotPathReflowStatus::ok) {
        return {
            .status = recursive.status,
            .detail = std::move(recursive.detail),
        };
    }

    RelativeSlotPathReflowReceipt receipt{
        .root = root.resource.id,
        .slot_path = std::vector<unsigned int>{slot_path.begin(), slot_path.end()},
        .source_sha256 = sha256_of(std::span<const std::byte>{
            root.bytes.data(), root.bytes.size()}),
        .replacement_sha256 = sha256_of(replacement_bytes),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            recursive.bytes.data(), recursive.bytes.size()}),
        .levels = std::move(recursive.levels),
    };
    if (!receipt.valid()) {
        return {
            .status = RelativeSlotPathReflowStatus::invalid_receipt,
            .detail = "slot-path receipt failed self-validation",
        };
    }

    return {
        .status = RelativeSlotPathReflowStatus::ok,
        .bytes = std::move(recursive.bytes),
        .receipt = std::move(receipt),
    };
}

} // namespace dmc::rengine::profiles::dmc3
