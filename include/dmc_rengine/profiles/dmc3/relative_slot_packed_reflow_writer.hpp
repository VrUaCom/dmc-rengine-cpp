#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct RelativeSlotPackedReflowSafety final {
    std::size_t max_output_bytes{0x40000000U};
};

enum class RelativeSlotPackedReflowStatus : std::uint8_t {
    ok,
    invalid_parent,
    invalid_expansion,
    no_authored_children,
    invalid_authored_image,
    duplicate_child_input,
    child_not_found,
    child_source_mismatch,
    child_output_mismatch,
    child_writer_validation_failed,
    parent_span_mismatch,
    alias_conflict,
    no_changes,
    output_too_large,
    output_parse_failed,
    topology_changed,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    RelativeSlotPackedReflowStatus status) noexcept {
    switch (status) {
    case RelativeSlotPackedReflowStatus::ok: return "ok";
    case RelativeSlotPackedReflowStatus::invalid_parent: return "invalid-parent";
    case RelativeSlotPackedReflowStatus::invalid_expansion: return "invalid-expansion";
    case RelativeSlotPackedReflowStatus::no_authored_children:
        return "no-authored-children";
    case RelativeSlotPackedReflowStatus::invalid_authored_image:
        return "invalid-authored-image";
    case RelativeSlotPackedReflowStatus::duplicate_child_input:
        return "duplicate-child-input";
    case RelativeSlotPackedReflowStatus::child_not_found: return "child-not-found";
    case RelativeSlotPackedReflowStatus::child_source_mismatch:
        return "child-source-mismatch";
    case RelativeSlotPackedReflowStatus::child_output_mismatch:
        return "child-output-mismatch";
    case RelativeSlotPackedReflowStatus::child_writer_validation_failed:
        return "child-writer-validation-failed";
    case RelativeSlotPackedReflowStatus::parent_span_mismatch:
        return "parent-span-mismatch";
    case RelativeSlotPackedReflowStatus::alias_conflict: return "alias-conflict";
    case RelativeSlotPackedReflowStatus::no_changes: return "no-changes";
    case RelativeSlotPackedReflowStatus::output_too_large:
        return "output-too-large";
    case RelativeSlotPackedReflowStatus::output_parse_failed:
        return "output-parse-failed";
    case RelativeSlotPackedReflowStatus::topology_changed:
        return "topology-changed";
    case RelativeSlotPackedReflowStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct RelativeSlotPackedSpanReceipt final {
    std::uint64_t source_offset{};
    std::uint64_t source_size{};
    std::uint64_t output_offset{};
    std::uint64_t output_size{};
    bool changed{};
    std::string source_sha256;
    std::string output_sha256;
    std::vector<gdspaces::ResourceId> affected_aliases;
    std::vector<gdspaces::ResourceId> authored_aliases;

    [[nodiscard]] bool valid() const noexcept;
};

struct RelativeSlotPackedReflowReceipt final {
    gdspaces::ResourceId parent;
    std::string source_sha256;
    std::string output_sha256;
    std::string writer_mode{"packed-relative-slot-reflow"};
    RelativeSlotTopology source_topology;
    RelativeSlotTopology output_topology;
    std::vector<RelativeSlotPackedSpanReceipt> spans;

    [[nodiscard]] bool valid() const noexcept;
};

struct RelativeSlotPackedReflowResult final {
    RelativeSlotPackedReflowStatus status{
        RelativeSlotPackedReflowStatus::invalid_parent};
    std::vector<std::byte> bytes;
    std::optional<RelativeSlotPackedReflowReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept;
};

class RelativeSlotPackedReflowWriter final {
public:
    // Reflows an already-expanded PAC/PNST parent after one or more authored
    // child images change size. Slot count, empty slots, physical-alias
    // partition and the pre-payload protected-prefix length remain invariant.
    // Unchanged physical spans are copied byte-for-byte. Changed container
    // children are independently reparsed and must preserve their own bounded
    // relative-slot identity relation; writer_mode strings are never trusted as
    // validation authority.
    [[nodiscard]] static RelativeSlotPackedReflowResult rebuild(
        const gdspaces::ResourcePayload& parent,
        const gdspaces::ContainerExpansion& expansion,
        std::span<const AuthoredChildImage> authored_children,
        RelativeSlotPackedReflowSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
