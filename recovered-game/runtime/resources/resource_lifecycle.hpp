#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::recovered::dmc3::runtime::resources {

// Recovered from the canonical e454... executable. These names describe the
// observed runtime state boundary without asserting unrecovered callback,
// ownership, cache-key, or refcount semantics.
enum class ResourceLoadState : std::uint32_t {
    free_or_unstarted = 0U,
    io_scheduled_or_loading = 1U,
    io_complete_pending_postprocess = 2U,
    ready_postprocessed = 3U,
    teardown_or_cancel_pending = 4U,
};

struct ResourceEntryLayout final {
    std::uint32_t stride{};
    std::uint32_t group_index_offset{};
    std::uint32_t state_offset{};
    std::uint32_t subtype_index_offset{};
    std::uint32_t source_descriptor_offset{};
    std::uint32_t loaded_payload_offset{};
    std::uint32_t owned_state_offset{};

    [[nodiscard]] bool valid() const noexcept;
};

struct ResourcePoolGroup final {
    std::uint32_t index{};
    std::uint32_t first_entry{};
    std::uint32_t entry_count{};

    [[nodiscard]] std::uint32_t end_entry() const noexcept {
        return first_entry + entry_count;
    }
};

struct ResourcePoolLayout final {
    std::uint64_t base_va{};
    std::uint32_t entry_count{};
    ResourceEntryLayout entry;
    std::array<ResourcePoolGroup, 7> groups{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const ResourcePoolGroup* group(
        std::uint32_t index) const noexcept;
};

enum class PostLoadFormat {
    mod,
    efm,
    scm,
    shw,
};

struct PostLoadFixup final {
    PostLoadFormat format{PostLoadFormat::mod};
    std::uint64_t function_va{};
    std::string_view symbol;
};

struct ResourceRuntimeEvidenceModel final {
    std::string_view artifact_sha256;
    std::string_view evidence_packet_id;
    ResourcePoolLayout pool;
    std::array<PostLoadFixup, 4> typed_fixups{};

    // Directly observed state chain for successful byte acquisition and typed
    // post-load normalization. Entry into state 4 is known to represent
    // teardown/cancellation, but the exact source-state domain remains open.
    std::array<ResourceLoadState, 4> successful_state_chain{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const PostLoadFixup* fixup(
        PostLoadFormat format) const noexcept;
};

[[nodiscard]] constexpr std::string_view to_string(
    ResourceLoadState state) noexcept {
    switch (state) {
    case ResourceLoadState::free_or_unstarted:
        return "free-or-unstarted";
    case ResourceLoadState::io_scheduled_or_loading:
        return "io-scheduled-or-loading";
    case ResourceLoadState::io_complete_pending_postprocess:
        return "io-complete-pending-postprocess";
    case ResourceLoadState::ready_postprocessed:
        return "ready-postprocessed";
    case ResourceLoadState::teardown_or_cancel_pending:
        return "teardown-or-cancel-pending";
    }
    return "unknown";
}

[[nodiscard]] constexpr std::string_view to_string(
    PostLoadFormat format) noexcept {
    switch (format) {
    case PostLoadFormat::mod: return "MOD";
    case PostLoadFormat::efm: return "EFM";
    case PostLoadFormat::scm: return "SCM";
    case PostLoadFormat::shw: return "SHW";
    }
    return "unknown";
}

// Canonical Wave-2 evidence model. This is recovered game-source metadata, not
// a GDSpaces product implementation and not a claim that the full resource
// manager class/ownership ABI has been reconstructed.
[[nodiscard]] const ResourceRuntimeEvidenceModel&
wave2_resource_runtime_model() noexcept;

} // namespace dmc::recovered::dmc3::runtime::resources
