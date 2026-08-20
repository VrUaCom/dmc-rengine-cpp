#pragma once

#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class TextureSlotPackedReflowStatus : std::uint8_t {
    ok,
    invalid_source_framing,
    no_authored_textures,
    duplicate_texture_input,
    texture_not_found,
    missing_source_hash,
    source_dds_mismatch,
    source_dds_invalid,
    authored_dds_invalid,
    compression_change_unsupported,
    unresolved_auxiliary_metadata,
    unsupported_secondary_relation,
    no_changes,
    output_too_large,
    output_validation_failed,
    output_texture_mismatch,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureSlotPackedReflowStatus status) noexcept {
    switch (status) {
    case TextureSlotPackedReflowStatus::ok: return "ok";
    case TextureSlotPackedReflowStatus::invalid_source_framing: return "invalid-source-framing";
    case TextureSlotPackedReflowStatus::no_authored_textures: return "no-authored-textures";
    case TextureSlotPackedReflowStatus::duplicate_texture_input: return "duplicate-texture-input";
    case TextureSlotPackedReflowStatus::texture_not_found: return "texture-not-found";
    case TextureSlotPackedReflowStatus::missing_source_hash: return "missing-source-hash";
    case TextureSlotPackedReflowStatus::source_dds_mismatch: return "source-dds-mismatch";
    case TextureSlotPackedReflowStatus::source_dds_invalid: return "source-dds-invalid";
    case TextureSlotPackedReflowStatus::authored_dds_invalid: return "authored-dds-invalid";
    case TextureSlotPackedReflowStatus::compression_change_unsupported: return "compression-change-unsupported";
    case TextureSlotPackedReflowStatus::unresolved_auxiliary_metadata: return "unresolved-auxiliary-metadata";
    case TextureSlotPackedReflowStatus::unsupported_secondary_relation: return "unsupported-secondary-relation";
    case TextureSlotPackedReflowStatus::no_changes: return "no-changes";
    case TextureSlotPackedReflowStatus::output_too_large: return "output-too-large";
    case TextureSlotPackedReflowStatus::output_validation_failed: return "output-validation-failed";
    case TextureSlotPackedReflowStatus::output_texture_mismatch: return "output-texture-mismatch";
    case TextureSlotPackedReflowStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct TextureSlotPackedReflowSafety final {
    std::size_t max_output_bytes{0x40000000U};
    Dmc3DdsSafety dds{};
    TextureSlotFramingSafety framing{};
};

struct AuthoredPackedTextureDds final {
    std::uint32_t texture_index{};
    std::string expected_source_sha256;
    std::vector<std::byte> bytes;
};

struct TextureSlotPackedTextureReceipt final {
    std::uint32_t texture_index{};
    std::uint64_t source_dds_size{};
    std::uint64_t output_dds_size{};
    std::uint32_t source_width{};
    std::uint32_t source_height{};
    std::uint32_t output_width{};
    std::uint32_t output_height{};
    std::string source_sha256;
    std::string output_sha256;

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotPackedReflowReceipt final {
    TextureSlotFramingKind framing_kind{TextureSlotFramingKind::wrapped_dds};
    std::uint64_t source_size{};
    std::uint64_t output_size{};
    std::string source_sha256;
    std::string output_sha256;
    std::string writer_mode{"size-changing-texture-packed-reflow"};
    std::vector<TextureSlotPackedTextureReceipt> patches;

    [[nodiscard]] bool valid() const;
};

struct TextureSlotPackedReflowResult final {
    TextureSlotPackedReflowStatus status{
        TextureSlotPackedReflowStatus::invalid_source_framing};
    std::vector<std::byte> bytes;
    std::optional<TextureSlotPackedReflowReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const;
};

class TextureSlotPackedReflowWriter final {
public:
    // Size-changing DMC3 texture-slot authoring for the evidence-backed safe
    // subset. Both bounded source DDS bytes and authored DDS bytes must pass
    // Dmc3DdsProfile. The authored DDS keeps the source DXT compression kind
    // and the source descriptor auxiliary pair must be exactly zero. The
    // source secondary-dimension relation (same or half) is preserved. Bundle
    // record spans are rebuilt from the Pass 80/82 corpus rule;
    // compact-final vs aligned-final class is preserved.
    [[nodiscard]] static TextureSlotPackedReflowResult rebuild(
        std::span<const std::byte> source_physical_slot,
        std::span<const AuthoredPackedTextureDds> authored_textures,
        TextureSlotPackedReflowSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
