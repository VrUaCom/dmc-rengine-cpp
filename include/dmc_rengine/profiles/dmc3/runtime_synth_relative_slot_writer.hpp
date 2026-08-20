#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class ExactChildAuthorityKind : std::uint8_t {
    format_writer_receipt,
    loose_resource,
    external_exact_resource,
    container_extracted_span,
};

[[nodiscard]] constexpr std::string_view to_string(
    ExactChildAuthorityKind kind) noexcept {
    switch (kind) {
    case ExactChildAuthorityKind::format_writer_receipt:
        return "format-writer-receipt";
    case ExactChildAuthorityKind::loose_resource:
        return "loose-resource";
    case ExactChildAuthorityKind::external_exact_resource:
        return "external-exact-resource";
    case ExactChildAuthorityKind::container_extracted_span:
        return "container-extracted-span";
    }
    return "container-extracted-span";
}

enum class ExactChildExtentKind : std::uint8_t {
    intrinsic_resource,
    writer_defined_complete_image,
    source_span_preserved,
    container_inferred_span,
};

[[nodiscard]] constexpr std::string_view to_string(
    ExactChildExtentKind kind) noexcept {
    switch (kind) {
    case ExactChildExtentKind::intrinsic_resource:
        return "intrinsic-resource";
    case ExactChildExtentKind::writer_defined_complete_image:
        return "writer-defined-complete-image";
    case ExactChildExtentKind::source_span_preserved:
        return "source-span-preserved";
    case ExactChildExtentKind::container_inferred_span:
        return "container-inferred-span";
    }
    return "container-inferred-span";
}

struct ExactChildImage final {
    std::uint32_t slot_index{};
    ExactChildAuthorityKind authority_kind{
        ExactChildAuthorityKind::container_extracted_span};
    ExactChildExtentKind extent_kind{
        ExactChildExtentKind::container_inferred_span};
    std::string authority_id;
    std::string writer_mode;
    std::string sha256;
    std::vector<std::byte> bytes;

    [[nodiscard]] bool valid_envelope() const noexcept {
        return !authority_id.empty() && sha256.size() == 64U && !bytes.empty();
    }
};

struct RuntimeSynthSafetyLimits final {
    std::size_t max_output_bytes{0x40000000U};
};

enum class RuntimeSynthStatus : std::uint8_t {
    ok,
    invalid_source,
    unsupported_format,
    source_parse_failed,
    alias_topology_unsupported,
    duplicate_child_input,
    child_for_empty_slot,
    unknown_child_slot,
    missing_child,
    invalid_child_authority,
    forbidden_extracted_span,
    unproven_intrinsic_extent,
    child_hash_mismatch,
    output_too_large,
    output_parse_failed,
    topology_changed,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeSynthStatus status) noexcept {
    switch (status) {
    case RuntimeSynthStatus::ok: return "ok";
    case RuntimeSynthStatus::invalid_source: return "invalid-source";
    case RuntimeSynthStatus::unsupported_format: return "unsupported-format";
    case RuntimeSynthStatus::source_parse_failed: return "source-parse-failed";
    case RuntimeSynthStatus::alias_topology_unsupported:
        return "alias-topology-unsupported";
    case RuntimeSynthStatus::duplicate_child_input: return "duplicate-child-input";
    case RuntimeSynthStatus::child_for_empty_slot: return "child-for-empty-slot";
    case RuntimeSynthStatus::unknown_child_slot: return "unknown-child-slot";
    case RuntimeSynthStatus::missing_child: return "missing-child";
    case RuntimeSynthStatus::invalid_child_authority:
        return "invalid-child-authority";
    case RuntimeSynthStatus::forbidden_extracted_span:
        return "forbidden-extracted-span";
    case RuntimeSynthStatus::unproven_intrinsic_extent:
        return "unproven-intrinsic-extent";
    case RuntimeSynthStatus::child_hash_mismatch: return "child-hash-mismatch";
    case RuntimeSynthStatus::output_too_large: return "output-too-large";
    case RuntimeSynthStatus::output_parse_failed: return "output-parse-failed";
    case RuntimeSynthStatus::topology_changed: return "topology-changed";
    case RuntimeSynthStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct RuntimeSynthChildReceipt final {
    std::uint32_t slot_index{};
    ExactChildAuthorityKind authority_kind{
        ExactChildAuthorityKind::container_extracted_span};
    ExactChildExtentKind extent_kind{
        ExactChildExtentKind::container_inferred_span};
    std::string authority_id;
    std::string writer_mode;
    std::string input_sha256;
    std::uint64_t intrinsic_size{};
    std::uint64_t emitted_offset{};

    [[nodiscard]] bool valid() const noexcept;
};

struct RuntimeSynthReceipt final {
    gdspaces::ResourceId source_resource;
    std::string source_sha256;
    std::string output_sha256;
    std::string writer_mode{"runtime-synth-relative-slot"};
    RelativeSlotTopology source_topology;
    RelativeSlotTopology output_topology;
    std::vector<RuntimeSynthChildReceipt> children;

    [[nodiscard]] bool valid() const noexcept;
};

struct RuntimeSynthResult final {
    RuntimeSynthStatus status{RuntimeSynthStatus::invalid_source};
    std::vector<std::byte> bytes;
    std::optional<RuntimeSynthReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept;
};

class RuntimeSynthRelativeSlotWriter final {
public:
    // Rebuilds an existing PAC/PNST into the recovered runtime-synthesized
    // relative-slot layout. Slot count/occupancy are preserved; every
    // populated slot requires explicit bytes plus independently justified
    // intrinsic extent authority.
    [[nodiscard]] static RuntimeSynthResult rebuild(
        const gdspaces::ResourcePayload& source,
        std::span<const ExactChildImage> exact_children,
        RuntimeSynthSafetyLimits safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
