#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::string_view kWriterMode = "size-changing-texture-packed-reflow";

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

void write_u32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] TextureSlotPackedReflowResult failure(
    TextureSlotPackedReflowStatus status,
    std::string detail) {
    return TextureSlotPackedReflowResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] std::span<const std::byte> bounded_dds(
    std::span<const std::byte> bytes,
    const TextureSlotEntry& entry) {
    return bytes.subspan(
        static_cast<std::size_t>(entry.dds_offset),
        static_cast<std::size_t>(entry.dds_size));
}

[[nodiscard]] Dmc3DdsCompression to_dds_compression(
    TextureCompressionKind compression) noexcept {
    return compression == TextureCompressionKind::dxt1
        ? Dmc3DdsCompression::dxt1
        : Dmc3DdsCompression::dxt5;
}

[[nodiscard]] TextureCompressionKind to_texture_compression(
    Dmc3DdsCompression compression) noexcept {
    return compression == Dmc3DdsCompression::dxt1
        ? TextureCompressionKind::dxt1
        : TextureCompressionKind::dxt5;
}

[[nodiscard]] std::vector<std::byte> build_descriptor(
    const TextureSlotEntry& source,
    const Dmc3DdsDocument& authored) {
    const bool secondary_same =
        source.secondary_width == source.width &&
        source.secondary_height == source.height;
    const auto secondary_width = secondary_same
        ? authored.width
        : authored.width / 2U;
    const auto secondary_height = secondary_same
        ? authored.height
        : authored.height / 2U;

    std::vector<std::byte> descriptor(
        TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    const bool dxt5 = authored.compression == Dmc3DdsCompression::dxt5;
    write_u32_le(
        descriptor, 0x08U,
        0x20000U | (authored.mip_map_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    write_u32_le(descriptor, 0x0CU, 0xAAE4U);
    write_u32_le(descriptor, 0x10U, (authored.height << 16U) | authored.width);
    write_u32_le(descriptor, 0x14U, 1U);
    write_u32_le(descriptor, 0x18U, authored.width * (dxt5 ? 4U : 2U));
    write_u32_le(descriptor, 0x20U, 0x40U);
    write_u32_le(descriptor, 0x38U, authored.payload_size);
    write_u32_le(descriptor, 0x3CU, 0U);
    write_u32_le(descriptor, 0x40U, 0U);
    write_u32_le(
        descriptor, 0x44U,
        (secondary_height << 16U) | secondary_width);
    write_u32_le(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_width)));
    write_u32_le(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_height)));
    write_u32_le(descriptor, 0x60U, dxt5 ? 4U : 0U);
    write_u32_le(descriptor, 0x64U, authored.total_size);
    write_u32_le(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] bool secondary_relation_supported(
    const TextureSlotEntry& source,
    const Dmc3DdsDocument& authored,
    Dmc3DdsSafety safety) noexcept {
    const bool same =
        source.secondary_width == source.width &&
        source.secondary_height == source.height;
    const bool half =
        source.secondary_width * 2U == source.width &&
        source.secondary_height * 2U == source.height;
    if (!same && !half) {
        return false;
    }
    if (same) {
        return true;
    }
    if ((authored.width & 1U) != 0U || (authored.height & 1U) != 0U) {
        return false;
    }
    return authored.width / 2U >= safety.min_dimension &&
        authored.height / 2U >= safety.min_dimension;
}

[[nodiscard]] std::size_t ceil_sector_span(std::size_t record_size) noexcept {
    return (record_size + TextureSlotFramingParser::k_sector_size - 1U) /
        TextureSlotFramingParser::k_sector_size;
}

[[nodiscard]] bool append_bounded(
    std::vector<std::byte>& output,
    std::span<const std::byte> bytes,
    std::size_t max_output) {
    if (bytes.size() > max_output || output.size() > max_output - bytes.size()) {
        return false;
    }
    output.insert(output.end(), bytes.begin(), bytes.end());
    return true;
}

[[nodiscard]] bool append_zeros(
    std::vector<std::byte>& output,
    std::size_t count,
    std::size_t max_output) {
    if (count > max_output || output.size() > max_output - count) {
        return false;
    }
    output.insert(output.end(), count, std::byte{0});
    return true;
}

struct PreparedTexture final {
    bool changed{};
    std::vector<std::byte> descriptor;
    std::vector<std::byte> dds;
    Dmc3DdsDocument output_document;
};

} // namespace

bool TextureSlotPackedTextureReceipt::valid() const noexcept {
    return source_dds_size != 0U && output_dds_size != 0U &&
        source_width != 0U && source_height != 0U &&
        output_width != 0U && output_height != 0U &&
        source_sha256.size() == 64U && output_sha256.size() == 64U &&
        source_sha256 != output_sha256;
}

bool TextureSlotPackedReflowReceipt::valid() const {
    if (source_size == 0U || output_size == 0U ||
        source_sha256.size() != 64U || output_sha256.size() != 64U ||
        source_sha256 == output_sha256 || writer_mode != kWriterMode ||
        patches.empty()) {
        return false;
    }
    std::unordered_set<std::uint32_t> seen;
    seen.reserve(patches.size());
    for (const auto& patch : patches) {
        if (!patch.valid() || !seen.insert(patch.texture_index).second) {
            return false;
        }
    }
    return true;
}

bool TextureSlotPackedReflowResult::ok() const {
    if (status != TextureSlotPackedReflowStatus::ok || !receipt.has_value() ||
        !receipt->valid() || bytes.empty()) {
        return false;
    }
    return receipt->output_size == bytes.size() &&
        receipt->output_sha256 == sha256_of(
            std::span<const std::byte>{bytes.data(), bytes.size()});
}

TextureSlotPackedReflowResult TextureSlotPackedReflowWriter::rebuild(
    std::span<const std::byte> source_physical_slot,
    std::span<const AuthoredPackedTextureDds> authored_textures,
    TextureSlotPackedReflowSafety safety) {
    if (safety.max_output_bytes == 0U) {
        return failure(
            TextureSlotPackedReflowStatus::output_too_large,
            "Texture reflow requires a non-zero output budget.");
    }

    const auto source = TextureSlotFramingParser::parse(
        source_physical_slot, safety.framing);
    if (!source.ok()) {
        return failure(
            TextureSlotPackedReflowStatus::invalid_source_framing,
            "Source physical slot failed canonical DMC3 texture framing: " +
                std::string{source.detail});
    }
    if (authored_textures.empty()) {
        return failure(
            TextureSlotPackedReflowStatus::no_authored_textures,
            "At least one authored DDS image is required.");
    }

    std::unordered_map<std::uint32_t, const AuthoredPackedTextureDds*> inputs;
    inputs.reserve(authored_textures.size());
    for (const auto& authored : authored_textures) {
        if (!inputs.emplace(authored.texture_index, &authored).second) {
            return failure(
                TextureSlotPackedReflowStatus::duplicate_texture_input,
                "The authored set contains the same texture index more than once.");
        }
        if (authored.texture_index >= source.document.textures.size()) {
            return failure(
                TextureSlotPackedReflowStatus::texture_not_found,
                "An authored texture index does not exist in the source framing.");
        }
        if (authored.expected_source_sha256.size() != 64U) {
            return failure(
                TextureSlotPackedReflowStatus::missing_source_hash,
                "Every authored texture must carry an exact source DDS SHA-256.");
        }
    }

    std::vector<PreparedTexture> prepared;
    prepared.reserve(source.document.textures.size());
    std::vector<TextureSlotPackedTextureReceipt> patches;
    patches.reserve(authored_textures.size());

    for (const auto& source_entry : source.document.textures) {
        const auto source_dds = bounded_dds(source_physical_slot, source_entry);
        const auto source_dds_hash = sha256_of(source_dds);
        const auto input = inputs.find(source_entry.texture_index);
        if (input == inputs.end()) {
            const auto descriptor_begin =
                static_cast<std::size_t>(source_entry.descriptor_offset);
            prepared.push_back(PreparedTexture{
                .changed = false,
                .descriptor = std::vector<std::byte>{
                    source_physical_slot.begin() + static_cast<std::ptrdiff_t>(descriptor_begin),
                    source_physical_slot.begin() + static_cast<std::ptrdiff_t>(
                        descriptor_begin + TextureSlotFramingParser::k_descriptor_size)},
                .dds = std::vector<std::byte>{source_dds.begin(), source_dds.end()},
                .output_document = Dmc3DdsDocument{
                    .width = source_entry.width,
                    .height = source_entry.height,
                    .mip_map_count = source_entry.mip_map_count,
                    .compression = to_dds_compression(source_entry.compression),
                    .payload_size = source_entry.dds_payload_size,
                    .total_size = source_entry.dds_size,
                },
            });
            continue;
        }

        const auto& authored = *input->second;
        if (source_dds_hash != authored.expected_source_sha256) {
            return failure(
                TextureSlotPackedReflowStatus::source_dds_mismatch,
                "An authored source hash does not match the bounded source DDS bytes.");
        }
        const auto authored_result = Dmc3DdsProfile::parse(
            std::span<const std::byte>{authored.bytes.data(), authored.bytes.size()},
            safety.dds);
        if (!authored_result.ok()) {
            return failure(
                TextureSlotPackedReflowStatus::authored_dds_invalid,
                "Authored DDS does not satisfy the Pass 81 DMC3 DDS profile: " +
                    std::string{authored_result.detail});
        }
        if (to_texture_compression(authored_result.document.compression) !=
            source_entry.compression) {
            return failure(
                TextureSlotPackedReflowStatus::compression_change_unsupported,
                "Pass 82 preserves the source DXT compression kind.");
        }
        if (std::equal(
                source_dds.begin(), source_dds.end(), authored.bytes.begin(),
                authored.bytes.end())) {
            const auto descriptor_begin =
                static_cast<std::size_t>(source_entry.descriptor_offset);
            prepared.push_back(PreparedTexture{
                .changed = false,
                .descriptor = std::vector<std::byte>{
                    source_physical_slot.begin() + static_cast<std::ptrdiff_t>(descriptor_begin),
                    source_physical_slot.begin() + static_cast<std::ptrdiff_t>(
                        descriptor_begin + TextureSlotFramingParser::k_descriptor_size)},
                .dds = authored.bytes,
                .output_document = authored_result.document,
            });
            continue;
        }
        if (source_entry.auxiliary_mode != 0U || source_entry.auxiliary_value != 0U) {
            return failure(
                TextureSlotPackedReflowStatus::unresolved_auxiliary_metadata,
                "Size-changing authoring is fail-closed for nonzero unresolved descriptor auxiliary metadata.");
        }
        if (!secondary_relation_supported(
                source_entry, authored_result.document, safety.dds)) {
            return failure(
                TextureSlotPackedReflowStatus::unsupported_secondary_relation,
                "Authored dimensions cannot preserve the source same/half secondary-dimension relation inside the evidenced domain.");
        }

        const auto output_hash = sha256_of(std::span<const std::byte>{
            authored.bytes.data(), authored.bytes.size()});
        prepared.push_back(PreparedTexture{
            .changed = true,
            .descriptor = build_descriptor(source_entry, authored_result.document),
            .dds = authored.bytes,
            .output_document = authored_result.document,
        });
        patches.push_back(TextureSlotPackedTextureReceipt{
            .texture_index = source_entry.texture_index,
            .source_dds_size = source_entry.dds_size,
            .output_dds_size = authored_result.document.total_size,
            .source_width = source_entry.width,
            .source_height = source_entry.height,
            .output_width = authored_result.document.width,
            .output_height = authored_result.document.height,
            .source_sha256 = source_dds_hash,
            .output_sha256 = output_hash,
        });
    }

    if (patches.empty()) {
        return failure(
            TextureSlotPackedReflowStatus::no_changes,
            "All authored DDS images are byte-identical to their bounded source images.");
    }

    std::vector<std::byte> output;
    if (source.document.kind == TextureSlotFramingKind::wrapped_dds) {
        const auto& texture = prepared.front();
        if (!append_bounded(
                output,
                std::span<const std::byte>{
                    texture.descriptor.data(), texture.descriptor.size()},
                safety.max_output_bytes) ||
            !append_bounded(
                output,
                std::span<const std::byte>{texture.dds.data(), texture.dds.size()},
                safety.max_output_bytes)) {
            return failure(
                TextureSlotPackedReflowStatus::output_too_large,
                "Rebuilt wrapped DDS exceeds the configured output budget.");
        }
    } else {
        if (source.document.textures.size() >
            (TextureSlotFramingParser::k_bundle_header_size - 4U) / 4U) {
            return failure(
                TextureSlotPackedReflowStatus::output_validation_failed,
                "Source bundle texture count exceeds the writable 0x800-byte header table.");
        }
        output.assign(TextureSlotFramingParser::k_bundle_header_size, std::byte{0});
        write_u32_le(
            output, 0U,
            static_cast<std::uint32_t>(source.document.textures.size()));

        for (std::size_t index = 0U; index < prepared.size(); ++index) {
            const auto& texture = prepared[index];
            const bool final = index + 1U == prepared.size();
            const bool compact_final =
                final && source.document.textures[index].sector_span == 0U;
            const auto record_size = texture.descriptor.size() + texture.dds.size();
            const auto sectors = compact_final ? 0U : ceil_sector_span(record_size);
            if (sectors > std::numeric_limits<std::uint32_t>::max()) {
                return failure(
                    TextureSlotPackedReflowStatus::output_too_large,
                    "Texture bundle sector span exceeds the 32-bit header domain.");
            }
            write_u32_le(
                output, 4U + index * 4U,
                static_cast<std::uint32_t>(sectors));
            if (!append_bounded(
                    output,
                    std::span<const std::byte>{
                        texture.descriptor.data(), texture.descriptor.size()},
                    safety.max_output_bytes) ||
                !append_bounded(
                    output,
                    std::span<const std::byte>{
                        texture.dds.data(), texture.dds.size()},
                    safety.max_output_bytes)) {
                return failure(
                    TextureSlotPackedReflowStatus::output_too_large,
                    "Texture bundle record exceeds the configured output budget.");
            }
            if (!compact_final) {
                const auto padded_size =
                    sectors * TextureSlotFramingParser::k_sector_size;
                const auto padding = padded_size - record_size;
                if (!append_zeros(output, padding, safety.max_output_bytes)) {
                    return failure(
                        TextureSlotPackedReflowStatus::output_too_large,
                        "Texture bundle padding exceeds the configured output budget.");
                }
            }
        }
    }

    const auto output_span = std::span<const std::byte>{output.data(), output.size()};
    const auto reparsed = TextureSlotFramingParser::parse(output_span, safety.framing);
    if (!reparsed.ok() || reparsed.document.kind != source.document.kind ||
        reparsed.document.textures.size() != source.document.textures.size()) {
        return failure(
            TextureSlotPackedReflowStatus::output_validation_failed,
            "Rebuilt texture slot failed canonical framing validation.");
    }

    for (std::size_t index = 0U; index < reparsed.document.textures.size(); ++index) {
        const auto output_dds = bounded_dds(output_span, reparsed.document.textures[index]);
        const auto& expected = prepared[index];
        if (output_dds.size() != expected.dds.size() ||
            !std::equal(output_dds.begin(), output_dds.end(), expected.dds.begin(), expected.dds.end())) {
            return failure(
                TextureSlotPackedReflowStatus::output_texture_mismatch,
                "Rebuilt output does not contain the exact expected DDS bytes at a texture index.");
        }
        const auto& output_entry = reparsed.document.textures[index];
        if (output_entry.width != expected.output_document.width ||
            output_entry.height != expected.output_document.height ||
            output_entry.mip_map_count != expected.output_document.mip_map_count ||
            output_entry.compression != to_texture_compression(expected.output_document.compression)) {
            return failure(
                TextureSlotPackedReflowStatus::output_texture_mismatch,
                "Reparsed output texture metadata differs from the authored DDS document.");
        }
    }

    TextureSlotPackedReflowReceipt receipt{
        .framing_kind = source.document.kind,
        .source_size = source_physical_slot.size(),
        .output_size = output.size(),
        .source_sha256 = sha256_of(source_physical_slot),
        .output_sha256 = sha256_of(output_span),
        .writer_mode = std::string{kWriterMode},
        .patches = std::move(patches),
    };
    if (!receipt.valid()) {
        return failure(
            TextureSlotPackedReflowStatus::invalid_receipt,
            "Size-changing texture reflow produced an invalid receipt.");
    }

    TextureSlotPackedReflowResult result{
        .status = TextureSlotPackedReflowStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
    if (!result.ok()) {
        return failure(
            TextureSlotPackedReflowStatus::invalid_receipt,
            "Size-changing texture reflow result failed self-validation.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
