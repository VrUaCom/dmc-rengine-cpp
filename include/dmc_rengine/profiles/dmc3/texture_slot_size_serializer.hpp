#pragma once

#include "dmc_rengine/profiles/dmc3/texture_slot_reintegrator.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct TextureSlotSizeSerializerSafety final {
    std::size_t max_output_bytes{0x40000000U};
};

enum class TextureSlotSizeSerializerStatus : std::uint8_t {
    ok,
    invalid_source_framing,
    no_authored_textures,
    duplicate_texture_input,
    texture_not_found,
    missing_source_hash,
    source_dds_mismatch,
    authored_dds_invalid,
    authored_dds_exception_profile,
    source_auxiliary_unsupported,
    no_size_change,
    output_too_large,
    output_validation_failed,
    output_dds_mismatch,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureSlotSizeSerializerStatus status) noexcept {
    switch (status) {
    case TextureSlotSizeSerializerStatus::ok: return "ok";
    case TextureSlotSizeSerializerStatus::invalid_source_framing:
        return "invalid-source-framing";
    case TextureSlotSizeSerializerStatus::no_authored_textures:
        return "no-authored-textures";
    case TextureSlotSizeSerializerStatus::duplicate_texture_input:
        return "duplicate-texture-input";
    case TextureSlotSizeSerializerStatus::texture_not_found:
        return "texture-not-found";
    case TextureSlotSizeSerializerStatus::missing_source_hash:
        return "missing-source-hash";
    case TextureSlotSizeSerializerStatus::source_dds_mismatch:
        return "source-dds-mismatch";
    case TextureSlotSizeSerializerStatus::authored_dds_invalid:
        return "authored-dds-invalid";
    case TextureSlotSizeSerializerStatus::authored_dds_exception_profile:
        return "authored-dds-exception-profile";
    case TextureSlotSizeSerializerStatus::source_auxiliary_unsupported:
        return "source-auxiliary-unsupported";
    case TextureSlotSizeSerializerStatus::no_size_change:
        return "no-size-change";
    case TextureSlotSizeSerializerStatus::output_too_large:
        return "output-too-large";
    case TextureSlotSizeSerializerStatus::output_validation_failed:
        return "output-validation-failed";
    case TextureSlotSizeSerializerStatus::output_dds_mismatch:
        return "output-dds-mismatch";
    case TextureSlotSizeSerializerStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct TextureSlotSizePatchReceipt final {
    std::uint32_t texture_index{};
    std::uint64_t source_descriptor_offset{};
    std::uint64_t output_descriptor_offset{};
    std::uint32_t source_dds_size{};
    std::uint32_t output_dds_size{};
    std::uint32_t source_sector_span{};
    std::uint32_t output_sector_span{};
    std::string source_dds_sha256;
    std::string output_dds_sha256;

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotSizeSerializerReceipt final {
    TextureSlotFramingKind framing_kind{TextureSlotFramingKind::wrapped_dds};
    std::string source_sha256;
    std::string output_sha256;
    std::string writer_mode{"size-changing-texture-slot-serialization"};
    std::vector<TextureSlotSizePatchReceipt> patches;

    [[nodiscard]] bool valid() const;
};

struct TextureSlotSizeSerializerResult final {
    TextureSlotSizeSerializerStatus status{
        TextureSlotSizeSerializerStatus::invalid_source_framing};
    std::vector<std::byte> bytes;
    std::optional<TextureSlotSizeSerializerReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const;
};

class TextureSlotSizeSerializer final {
public:
    // Rebuilds a validated DMC3 texture physical slot around one or more
    // authored complete DDS images when at least one DDS byte size changes.
    //
    // Authoring is intentionally bounded to the authority available after
    // Passes 78-81:
    // - source framing must pass TextureSlotFramingParser;
    // - authored DDS must pass DdsImageParser standard_corpus profile;
    // - every authored DDS is hash-bound to its bounded source DDS;
    // - a changed source descriptor must have auxiliary_mode/value == 0;
    // - the source secondary-dimension relation (1x or 1/2x) is preserved;
    // - descriptor fields are reconstructed from the authored DDS + preserved
    //   structural relation, never guessed from runtime semantics;
    // - bundle sector spans use the corpus-confirmed ceil((0x70+DDS)/0x800)
    //   rule; a final source span=0 preserves compact-EOF style;
    // - unchanged physical records are copied byte-for-byte;
    // - the complete result must reparse canonically before a receipt exists.
    [[nodiscard]] static TextureSlotSizeSerializerResult rebuild(
        std::span<const std::byte> source_physical_slot,
        std::span<const AuthoredTextureDds> authored_textures,
        TextureSlotSizeSerializerSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
