#pragma once

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class TextureSlotReintegrationStatus : std::uint8_t {
    ok,
    invalid_source_framing,
    no_authored_textures,
    duplicate_texture_input,
    texture_not_found,
    missing_source_hash,
    source_dds_mismatch,
    dds_size_changed,
    no_changes,
    output_validation_failed,
    framing_changed,
    non_dds_bytes_changed,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureSlotReintegrationStatus status) noexcept {
    switch (status) {
    case TextureSlotReintegrationStatus::ok: return "ok";
    case TextureSlotReintegrationStatus::invalid_source_framing:
        return "invalid-source-framing";
    case TextureSlotReintegrationStatus::no_authored_textures:
        return "no-authored-textures";
    case TextureSlotReintegrationStatus::duplicate_texture_input:
        return "duplicate-texture-input";
    case TextureSlotReintegrationStatus::texture_not_found:
        return "texture-not-found";
    case TextureSlotReintegrationStatus::missing_source_hash:
        return "missing-source-hash";
    case TextureSlotReintegrationStatus::source_dds_mismatch:
        return "source-dds-mismatch";
    case TextureSlotReintegrationStatus::dds_size_changed:
        return "dds-size-changed";
    case TextureSlotReintegrationStatus::no_changes: return "no-changes";
    case TextureSlotReintegrationStatus::output_validation_failed:
        return "output-validation-failed";
    case TextureSlotReintegrationStatus::framing_changed:
        return "framing-changed";
    case TextureSlotReintegrationStatus::non_dds_bytes_changed:
        return "non-dds-bytes-changed";
    case TextureSlotReintegrationStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct AuthoredTextureDds final {
    std::uint32_t texture_index{};
    std::string expected_source_sha256;
    std::vector<std::byte> bytes;
};

struct TextureDdsPatchReceipt final {
    std::uint32_t texture_index{};
    std::uint64_t dds_offset{};
    std::uint64_t dds_size{};
    std::string source_sha256;
    std::string output_sha256;

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotReintegrationReceipt final {
    TextureSlotFramingKind framing_kind{TextureSlotFramingKind::wrapped_dds};
    std::string source_sha256;
    std::string output_sha256;
    std::string writer_mode{"same-layout-intrinsic-dds-reintegration"};
    std::vector<TextureDdsPatchReceipt> patches;

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotReintegrationResult final {
    TextureSlotReintegrationStatus status{
        TextureSlotReintegrationStatus::invalid_source_framing};
    std::vector<std::byte> bytes;
    std::optional<TextureSlotReintegrationReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept;
};

class TextureSlotReintegrator final {
public:
    // Reintegrates authored intrinsic DDS bytes into an already-validated DMC3
    // texture physical slot without changing its physical layout. This first
    // bounded writer intentionally requires exact DDS byte-size preservation.
    // Descriptor bytes, bundle header bytes and alignment padding are copied
    // from the source and must remain byte-identical after reintegration.
    //
    // Size-changing/dimension-changing texture serialization is deliberately
    // outside this contract until the remaining variable descriptor fields
    // have independent authority.
    [[nodiscard]] static TextureSlotReintegrationResult rebuild(
        std::span<const std::byte> source_physical_slot,
        std::span<const AuthoredTextureDds> authored_textures,
        TextureSlotFramingSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
