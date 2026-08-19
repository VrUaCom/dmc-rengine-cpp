#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct AuthoredChildImage final {
    gdspaces::ResourceId resource;
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t revision{};
    std::string writer_mode;
    std::vector<std::byte> bytes;

    [[nodiscard]] bool valid() const noexcept;
};

enum class NestedReintegrationStatus : std::uint8_t {
    ok,
    invalid_parent,
    invalid_expansion,
    no_authored_children,
    invalid_authored_image,
    duplicate_child_input,
    child_not_found,
    child_source_mismatch,
    child_output_mismatch,
    child_size_changed,
    child_writer_validation_failed,
    parent_span_mismatch,
    alias_conflict,
    no_changes,
    parent_edit_failed,
    parent_rebuild_failed,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    NestedReintegrationStatus status) noexcept {
    switch (status) {
    case NestedReintegrationStatus::ok: return "ok";
    case NestedReintegrationStatus::invalid_parent: return "invalid-parent";
    case NestedReintegrationStatus::invalid_expansion: return "invalid-expansion";
    case NestedReintegrationStatus::no_authored_children: return "no-authored-children";
    case NestedReintegrationStatus::invalid_authored_image: return "invalid-authored-image";
    case NestedReintegrationStatus::duplicate_child_input: return "duplicate-child-input";
    case NestedReintegrationStatus::child_not_found: return "child-not-found";
    case NestedReintegrationStatus::child_source_mismatch: return "child-source-mismatch";
    case NestedReintegrationStatus::child_output_mismatch: return "child-output-mismatch";
    case NestedReintegrationStatus::child_size_changed: return "child-size-changed";
    case NestedReintegrationStatus::child_writer_validation_failed:
        return "child-writer-validation-failed";
    case NestedReintegrationStatus::parent_span_mismatch: return "parent-span-mismatch";
    case NestedReintegrationStatus::alias_conflict: return "alias-conflict";
    case NestedReintegrationStatus::no_changes: return "no-changes";
    case NestedReintegrationStatus::parent_edit_failed: return "parent-edit-failed";
    case NestedReintegrationStatus::parent_rebuild_failed: return "parent-rebuild-failed";
    case NestedReintegrationStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct NestedSpanReceipt final {
    std::uint64_t offset{};
    std::uint64_t size{};
    std::string source_sha256;
    std::string output_sha256;
    std::vector<gdspaces::ResourceId> affected_aliases;
    std::vector<gdspaces::ResourceId> authored_aliases;

    [[nodiscard]] bool valid() const noexcept;
};

struct NestedReintegrationReceipt final {
    gdspaces::ResourceId parent;
    std::string parent_source_sha256;
    std::string parent_output_sha256;
    std::uint64_t parent_revision{};
    std::vector<NestedSpanReceipt> spans;
    LayoutPreservingRebuildReceipt parent_rebuild;

    [[nodiscard]] bool valid() const noexcept;
};

struct NestedReintegrationResult final {
    NestedReintegrationStatus status{NestedReintegrationStatus::invalid_parent};
    std::vector<std::byte> bytes;
    std::optional<NestedReintegrationReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == NestedReintegrationStatus::ok && receipt.has_value() &&
            receipt->valid() && receipt->parent_output_sha256 ==
                receipt->parent_rebuild.output_sha256 &&
            bytes.size() == receipt->parent_rebuild.output_topology.container_size;
    }
};

class NestedRelativeSlotReintegrator final {
public:
    // Reintegrates already-writer-validated, same-size child images into the
    // materialized PAC/PNST parent byte domain. It never translates nested
    // offsets into compressed archive storage coordinates.
    [[nodiscard]] static NestedReintegrationResult reintegrate(
        const gdspaces::ResourcePayload& parent,
        const gdspaces::ContainerExpansion& expansion,
        std::span<const AuthoredChildImage> authored_children);
};

} // namespace dmc::rengine::profiles::dmc3
