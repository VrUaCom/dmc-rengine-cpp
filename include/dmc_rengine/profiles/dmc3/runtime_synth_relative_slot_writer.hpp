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

struct RuntimeSynthResult;

class ExactChildImage final {
public:
    ExactChildImage(const ExactChildImage&) = default;
    ExactChildImage(ExactChildImage&&) noexcept = default;
    ExactChildImage& operator=(const ExactChildImage&) = default;
    ExactChildImage& operator=(ExactChildImage&&) noexcept = default;
    ~ExactChildImage() = default;

    // Caller-owned exact resource authority. The factory computes the hash and
    // only accepts independently intrinsic standalone resource providers.
    [[nodiscard]] static std::optional<ExactChildImage> from_intrinsic_resource(
        std::uint32_t slot_index,
        ExactChildAuthorityKind authority_kind,
        std::string authority_id,
        std::vector<std::byte> bytes);

    // Typed complete-image authority. A caller cannot self-declare a writer
    // receipt: this factory accepts only a currently valid successful
    // RuntimeSynthResult and derives all provenance/hash fields from it.
    [[nodiscard]] static std::optional<ExactChildImage>
    from_verified_runtime_synth_result(
        std::uint32_t slot_index,
        const RuntimeSynthResult& result);

    [[nodiscard]] std::uint32_t slot_index() const noexcept {
        return slot_index_;
    }
    [[nodiscard]] ExactChildAuthorityKind authority_kind() const noexcept {
        return authority_kind_;
    }
    [[nodiscard]] ExactChildExtentKind extent_kind() const noexcept {
        return extent_kind_;
    }
    [[nodiscard]] const std::string& authority_id() const noexcept {
        return authority_id_;
    }
    [[nodiscard]] const std::string& writer_mode() const noexcept {
        return writer_mode_;
    }
    [[nodiscard]] const std::string& sha256() const noexcept {
        return sha256_;
    }
    [[nodiscard]] const std::vector<std::byte>& bytes() const noexcept {
        return bytes_;
    }
    [[nodiscard]] bool valid_envelope() const noexcept;

private:
    ExactChildImage(
        std::uint32_t slot_index,
        ExactChildAuthorityKind authority_kind,
        ExactChildExtentKind extent_kind,
        std::string authority_id,
        std::string writer_mode,
        std::string sha256,
        std::vector<std::byte> bytes);

    std::uint32_t slot_index_{};
    ExactChildAuthorityKind authority_kind_{
        ExactChildAuthorityKind::container_extracted_span};
    ExactChildExtentKind extent_kind_{
        ExactChildExtentKind::container_inferred_span};
    std::string authority_id_;
    std::string writer_mode_;
    std::string sha256_;
    std::vector<std::byte> bytes_;
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

class RuntimeSynthChildReceipt final {
public:
    RuntimeSynthChildReceipt(const RuntimeSynthChildReceipt&) = default;
    RuntimeSynthChildReceipt(RuntimeSynthChildReceipt&&) noexcept = default;
    RuntimeSynthChildReceipt& operator=(const RuntimeSynthChildReceipt&) = default;
    RuntimeSynthChildReceipt& operator=(RuntimeSynthChildReceipt&&) noexcept = default;
    ~RuntimeSynthChildReceipt() = default;

    [[nodiscard]] std::uint32_t slot_index() const noexcept {
        return slot_index_;
    }
    [[nodiscard]] ExactChildAuthorityKind authority_kind() const noexcept {
        return authority_kind_;
    }
    [[nodiscard]] ExactChildExtentKind extent_kind() const noexcept {
        return extent_kind_;
    }
    [[nodiscard]] const std::string& authority_id() const noexcept {
        return authority_id_;
    }
    [[nodiscard]] const std::string& writer_mode() const noexcept {
        return writer_mode_;
    }
    [[nodiscard]] const std::string& input_sha256() const noexcept {
        return input_sha256_;
    }
    [[nodiscard]] std::uint64_t intrinsic_size() const noexcept {
        return intrinsic_size_;
    }
    [[nodiscard]] std::uint64_t emitted_offset() const noexcept {
        return emitted_offset_;
    }

    [[nodiscard]] bool valid() const noexcept;

private:
    friend class RuntimeSynthRelativeSlotWriter;

    RuntimeSynthChildReceipt(
        std::uint32_t slot_index,
        ExactChildAuthorityKind authority_kind,
        ExactChildExtentKind extent_kind,
        std::string authority_id,
        std::string writer_mode,
        std::string input_sha256,
        std::uint64_t intrinsic_size,
        std::uint64_t emitted_offset);

    std::uint32_t slot_index_{};
    ExactChildAuthorityKind authority_kind_{
        ExactChildAuthorityKind::container_extracted_span};
    ExactChildExtentKind extent_kind_{
        ExactChildExtentKind::container_inferred_span};
    std::string authority_id_;
    std::string writer_mode_;
    std::string input_sha256_;
    std::uint64_t intrinsic_size_{};
    std::uint64_t emitted_offset_{};
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
    // relative-slot layout. Slot count/occupancy are preserved. Exact child
    // inputs are capability objects created by factories, so complete-image
    // writer provenance cannot be self-declared with forgeable strings.
    [[nodiscard]] static RuntimeSynthResult rebuild(
        const gdspaces::ResourcePayload& source,
        std::span<const ExactChildImage> exact_children,
        RuntimeSynthSafetyLimits safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
