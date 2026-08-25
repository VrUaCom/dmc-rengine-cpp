#pragma once

#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct RelativeSlotPathReflowSafety final {
    // Product hardening only: bounds user-supplied recursive authoring paths.
    // This is not claimed as an original DMC3 recursion limit.
    std::size_t max_depth{64U};
    RelativeSlotPackedReflowSafety parent_reflow{};
};

enum class RelativeSlotPathReflowStatus : std::uint8_t {
    ok,
    invalid_root,
    empty_slot_path,
    path_too_deep,
    parse_failed,
    expansion_failed,
    slot_out_of_range,
    empty_slot,
    parent_reflow_failed,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    RelativeSlotPathReflowStatus status) noexcept {
    switch (status) {
    case RelativeSlotPathReflowStatus::ok: return "ok";
    case RelativeSlotPathReflowStatus::invalid_root: return "invalid-root";
    case RelativeSlotPathReflowStatus::empty_slot_path: return "empty-slot-path";
    case RelativeSlotPathReflowStatus::path_too_deep: return "path-too-deep";
    case RelativeSlotPathReflowStatus::parse_failed: return "parse-failed";
    case RelativeSlotPathReflowStatus::expansion_failed: return "expansion-failed";
    case RelativeSlotPathReflowStatus::slot_out_of_range: return "slot-out-of-range";
    case RelativeSlotPathReflowStatus::empty_slot: return "empty-slot";
    case RelativeSlotPathReflowStatus::parent_reflow_failed:
        return "parent-reflow-failed";
    case RelativeSlotPathReflowStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct RelativeSlotPathLevelReceipt final {
    gdspaces::ResourceId parent;
    unsigned int slot_index{};
    std::string parser_format;
    std::string source_sha256;
    std::string output_sha256;
    RelativeSlotPackedReflowReceipt reflow;

    [[nodiscard]] bool valid() const;
};

struct RelativeSlotPathReflowReceipt final {
    gdspaces::ResourceId root;
    std::vector<unsigned int> slot_path;
    std::string source_sha256;
    std::string replacement_sha256;
    std::string output_sha256;
    std::vector<RelativeSlotPathLevelReceipt> levels;

    [[nodiscard]] bool valid() const;
};

struct RelativeSlotPathReflowResult final {
    RelativeSlotPathReflowStatus status{RelativeSlotPathReflowStatus::invalid_root};
    std::vector<std::byte> bytes;
    std::optional<RelativeSlotPathReflowReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const;
};

class RelativeSlotPathReflowWriter final {
public:
    // Rebuilds one populated PAC/PNST child addressed by a root-to-leaf slot
    // path. Each ancestor is reparsed, expanded and size-changing-reflowed
    // bottom-up through RelativeSlotPackedReflowWriter. Unchanged siblings keep
    // the underlying writer's byte-preservation and alias-partition guarantees.
    [[nodiscard]] static RelativeSlotPathReflowResult rebuild(
        const gdspaces::ResourcePayload& root,
        std::span<const unsigned int> slot_path,
        std::span<const std::byte> replacement_bytes,
        RelativeSlotPathReflowSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
