#include "dmc_rengine/profiles/dmc3/texture_slot_size_serializer.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::string_view kWriterMode =
    "size-changing-texture-slot-serialization";

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] TextureSlotSizeSerializerResult failure(
    TextureSlotSizeSerializerStatus status,
    std::string detail) {
    return TextureSlotSizeSerializerResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::span<const std::byte> dds_span(
    std::span<const std::byte> bytes,
    const TextureSlotEntry& entry) {
    return bytes.subspan(
        static_cast<std::size_t>(entry.dds_offset),
        static_cast<std::size_t>(entry.dds_size));
}

[[nodiscard]] bool append_checked(
    std::vector<std::byte>& output,
    std::span<const std::byte> bytes,
    std::size_t max_output_bytes) {
    if (bytes.size() > max_output_bytes ||
        output.size() > max_output_bytes - bytes.size()) {
        return false;
    }
    output.insert(output.end(), bytes.begin(), bytes.end());
    return true;
}

[[nodiscard]] bool append_zeroes_checked(
    std::vector<std::byte>& output,
    std::size_t count,
    std::size_t max_output_bytes) {
    if (count > max_output_bytes || output.size() > max_output_bytes - count) {
        return false;
    }
    output.insert(output.end(), count, std::byte{0});
    return true;
}

[[nodiscard]] std::uint32_t sector_span_for_size(std::size_t record_size) {
    const auto sector = TextureSlotFramingParser::k_sector_size;
    return static_cast<std::uint32_t>((record_size + sector - 1U) / sector);
}

[[nodiscard]] std::optional<std::vector<std::byte>> make_descriptor(
    const TextureSlotEntry& source_entry,
    const DdsImageDocument& authored) {
    if (source_entry.auxiliary_mode != 0U || source_entry.auxiliary_value != 0U ||
        authored.profile != DdsHeaderProfile::standard_corpus ||
        authored.width > 0xFFFFU || authored.height > 0xFFFFU) {
        return std::nullopt;
    }

    const bool secondary_half =
        source_entry.secondary_width * 2U == source_entry.width &&
        source_entry.secondary_height * 2U == source_entry.height;
    std::uint32_t secondary_width = authored.width;
    std::uint32_t secondary_height = authored.height;
    if (secondary_half) {
        if (authored.width < 2U || authored.height < 2U ||
            (authored.width & 1U) != 0U || (authored.height & 1U) != 0U) {
            return std::nullopt;
        }
        secondary_width /= 2U;
        secondary_height /= 2U;
    }

    const bool dxt5 = authored.compression == DdsCompressionKind::dxt5;
    std::vector<std::byte> descriptor(
        TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (authored.mip_map_count << 8U) |
            (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(
        descriptor, 0x10U,
        (authored.height << 16U) | authored.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, authored.width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, authored.payload_size);
    put_u32(
        descriptor, 0x44U,
        (secondary_height << 16U) | secondary_width);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_width)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(
        descriptor, 0x64U,
        static_cast<std::uint32_t>(authored.total_size));
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

} // namespace

bool TextureSlotSizePatchReceipt::valid() const noexcept {
    return source_dds_size != 0U && output_dds_size != 0U &&
        source_dds_sha256.size() == 64U && output_dds_sha256.size() == 64U &&
        source_dds_sha256 != output_dds_sha256;
}

bool TextureSlotSizeSerializerReceipt::valid() const {
    if (source_sha256.size() != 64U || output_sha256.size() != 64U ||
        source_sha256 == output_sha256 || writer_mode != kWriterMode ||
        patches.empty()) {
        return false;
    }
    bool saw_size_change = false;
    std::unordered_set<std::uint32_t> seen;
    seen.reserve(patches.size());
    for (const auto& patch : patches) {
        if (!patch.valid() || !seen.insert(patch.texture_index).second) {
            return false;
        }
        saw_size_change = saw_size_change ||
            patch.source_dds_size != patch.output_dds_size;
    }
    return saw_size_change;
}

bool TextureSlotSizeSerializerResult::ok() const {
    if (status != TextureSlotSizeSerializerStatus::ok ||
        !receipt.has_value() || !receipt->valid() || bytes.empty()) {
        return false;
    }
    return receipt->output_sha256 == sha256_of(
        std::span<const std::byte>{bytes.data(), bytes.size()});
}

TextureSlotSizeSerializerResult TextureSlotSizeSerializer::rebuild(
    std::span<const std::byte> source_physical_slot,
    std::span<const AuthoredTextureDds> authored_textures,
    TextureSlotSizeSerializerSafety safety) {
    if (safety.max_output_bytes == 0U) {
        return failure(
            TextureSlotSizeSerializerStatus::output_too_large,
            "Texture size serializer requires a non-zero output byte budget.");
    }

    const auto source = TextureSlotFramingParser::parse(source_physical_slot);
    if (!source.ok()) {
        return failure(
            TextureSlotSizeSerializerStatus::invalid_source_framing,
            "Source physical slot failed canonical DMC3 texture framing validation.");
    }
    if (authored_textures.empty()) {
        return failure(
            TextureSlotSizeSerializerStatus::no_authored_textures,
            "At least one authored DDS image is required.");
    }

    std::vector<const AuthoredTextureDds*> authored_by_index(
        source.document.textures.size(), nullptr);
    std::vector<DdsImageDocument> authored_documents(
        source.document.textures.size());
    std::vector<bool> authored_changed(source.document.textures.size(), false);
    bool saw_size_change = false;

    for (const auto& authored : authored_textures) {
        if (authored.texture_index >= source.document.textures.size()) {
            return failure(
                TextureSlotSizeSerializerStatus::texture_not_found,
                "An authored texture index does not exist in the source framing.");
        }
        auto*& slot = authored_by_index[authored.texture_index];
        if (slot != nullptr) {
            return failure(
                TextureSlotSizeSerializerStatus::duplicate_texture_input,
                "The authored DDS set contains the same texture index more than once.");
        }
        slot = &authored;
        if (authored.expected_source_sha256.size() != 64U) {
            return failure(
                TextureSlotSizeSerializerStatus::missing_source_hash,
                "Every authored DDS must carry an exact source SHA-256.");
        }

        const auto& source_entry = source.document.textures[authored.texture_index];
        const auto source_dds = dds_span(source_physical_slot, source_entry);
        const auto source_sha = sha256_of(source_dds);
        if (source_sha != authored.expected_source_sha256) {
            return failure(
                TextureSlotSizeSerializerStatus::source_dds_mismatch,
                "An authored DDS source hash does not match its bounded source bytes.");
        }
        if (authored.bytes.size() > safety.max_output_bytes) {
            return failure(
                TextureSlotSizeSerializerStatus::output_too_large,
                "An authored DDS exceeds the texture serializer output byte budget.");
        }

        const auto authored_result = DdsImageParser::parse(
            std::span<const std::byte>{authored.bytes.data(), authored.bytes.size()});
        if (!authored_result.ok()) {
            return failure(
                TextureSlotSizeSerializerStatus::authored_dds_invalid,
                "Authored DDS failed the canonical DMC3 DDS structural contract.");
        }
        if (authored_result.document.profile != DdsHeaderProfile::standard_corpus) {
            return failure(
                TextureSlotSizeSerializerStatus::authored_dds_exception_profile,
                "Pass 82 does not generalize the one observed depth=1 DDS profile for authoring.");
        }

        const bool changed = !std::equal(
            source_dds.begin(), source_dds.end(), authored.bytes.begin(),
            authored.bytes.end());
        authored_changed[authored.texture_index] = changed;
        authored_documents[authored.texture_index] = authored_result.document;
        if (changed &&
            (source_entry.auxiliary_mode != 0U ||
             source_entry.auxiliary_value != 0U)) {
            return failure(
                TextureSlotSizeSerializerStatus::source_auxiliary_unsupported,
                "Pass 82 refuses to reserialize a changed descriptor with unresolved non-zero auxiliary metadata.");
        }
        saw_size_change = saw_size_change ||
            (changed && authored.bytes.size() != source_dds.size());
    }

    if (!saw_size_change) {
        return failure(
            TextureSlotSizeSerializerStatus::no_size_change,
            "Pass 82 requires at least one authored DDS byte-size change; same-size edits remain Pass 79 authority.");
    }

    std::vector<std::byte> output;
    std::vector<TextureSlotSizePatchReceipt> patches;
    patches.reserve(authored_textures.size());

    if (source.document.kind == TextureSlotFramingKind::wrapped_dds) {
        const auto* authored = authored_by_index[0U];
        if (authored == nullptr || !authored_changed[0U]) {
            return failure(
                TextureSlotSizeSerializerStatus::no_size_change,
                "A wrapped DDS size-changing rebuild requires texture index 0 to change.");
        }
        const auto descriptor = make_descriptor(
            source.document.textures[0U], authored_documents[0U]);
        if (!descriptor.has_value()) {
            return failure(
                TextureSlotSizeSerializerStatus::source_auxiliary_unsupported,
                "Source descriptor relation cannot be serialized safely for the authored DDS.");
        }
        if (!append_checked(output, *descriptor, safety.max_output_bytes) ||
            !append_checked(
                output,
                std::span<const std::byte>{authored->bytes.data(), authored->bytes.size()},
                safety.max_output_bytes)) {
            return failure(
                TextureSlotSizeSerializerStatus::output_too_large,
                "Rebuilt wrapped texture exceeds the output byte budget.");
        }

        const auto source_dds = dds_span(
            source_physical_slot, source.document.textures[0U]);
        patches.push_back(TextureSlotSizePatchReceipt{
            .texture_index = 0U,
            .source_descriptor_offset = 0U,
            .output_descriptor_offset = 0U,
            .source_dds_size = source.document.textures[0U].dds_size,
            .output_dds_size = static_cast<std::uint32_t>(authored->bytes.size()),
            .source_sector_span = 0U,
            .output_sector_span = 0U,
            .source_dds_sha256 = sha256_of(source_dds),
            .output_dds_sha256 = sha256_of(std::span<const std::byte>{
                authored->bytes.data(), authored->bytes.size()}),
        });
    } else {
        if (source_physical_slot.size() < TextureSlotFramingParser::k_bundle_header_size ||
            TextureSlotFramingParser::k_bundle_header_size > safety.max_output_bytes) {
            return failure(
                TextureSlotSizeSerializerStatus::output_too_large,
                "Texture bundle header exceeds the output byte budget.");
        }
        output.assign(
            source_physical_slot.begin(),
            source_physical_slot.begin() + static_cast<std::ptrdiff_t>(
                TextureSlotFramingParser::k_bundle_header_size));

        for (std::size_t index = 0U;
             index < source.document.textures.size(); ++index) {
            const auto& source_entry = source.document.textures[index];
            const bool final = index + 1U == source.document.textures.size();
            const auto source_record_begin =
                static_cast<std::size_t>(source_entry.descriptor_offset);
            const auto source_record_end = final
                ? source_physical_slot.size()
                : static_cast<std::size_t>(
                      source.document.textures[index + 1U].descriptor_offset);
            if (source_record_begin > source_record_end ||
                source_record_end > source_physical_slot.size()) {
                return failure(
                    TextureSlotSizeSerializerStatus::invalid_source_framing,
                    "Source bundle record geometry is internally inconsistent.");
            }

            const auto output_descriptor_offset = output.size();
            std::uint32_t output_sector_span = source_entry.sector_span;
            const auto* authored = authored_by_index[index];
            const bool changed = authored != nullptr && authored_changed[index];

            if (!changed) {
                if (!append_checked(
                        output,
                        source_physical_slot.subspan(
                            source_record_begin,
                            source_record_end - source_record_begin),
                        safety.max_output_bytes)) {
                    return failure(
                        TextureSlotSizeSerializerStatus::output_too_large,
                        "Copied unchanged texture record exceeds the output byte budget.");
                }
            } else {
                const auto descriptor = make_descriptor(
                    source_entry, authored_documents[index]);
                if (!descriptor.has_value()) {
                    return failure(
                        TextureSlotSizeSerializerStatus::source_auxiliary_unsupported,
                        "Source descriptor relation cannot be serialized safely for the authored DDS.");
                }
                const auto record_size = descriptor->size() + authored->bytes.size();
                const bool compact_final = final && source_entry.sector_span == 0U;
                if (compact_final) {
                    output_sector_span = 0U;
                } else {
                    const auto span = sector_span_for_size(record_size);
                    if (span == 0U ||
                        span > std::numeric_limits<std::size_t>::max() /
                            TextureSlotFramingParser::k_sector_size) {
                        return failure(
                            TextureSlotSizeSerializerStatus::output_too_large,
                            "Rebuilt texture record sector span overflows host size.");
                    }
                    output_sector_span = span;
                }

                if (!append_checked(output, *descriptor, safety.max_output_bytes) ||
                    !append_checked(
                        output,
                        std::span<const std::byte>{
                            authored->bytes.data(), authored->bytes.size()},
                        safety.max_output_bytes)) {
                    return failure(
                        TextureSlotSizeSerializerStatus::output_too_large,
                        "Rebuilt texture record exceeds the output byte budget.");
                }
                if (!compact_final) {
                    const auto target_record_size =
                        static_cast<std::size_t>(output_sector_span) *
                        TextureSlotFramingParser::k_sector_size;
                    if (record_size > target_record_size ||
                        !append_zeroes_checked(
                            output, target_record_size - record_size,
                            safety.max_output_bytes)) {
                        return failure(
                            TextureSlotSizeSerializerStatus::output_too_large,
                            "Rebuilt texture record padding exceeds the output byte budget.");
                    }
                }

                const auto source_dds = dds_span(source_physical_slot, source_entry);
                patches.push_back(TextureSlotSizePatchReceipt{
                    .texture_index = static_cast<std::uint32_t>(index),
                    .source_descriptor_offset = source_entry.descriptor_offset,
                    .output_descriptor_offset = output_descriptor_offset,
                    .source_dds_size = source_entry.dds_size,
                    .output_dds_size = static_cast<std::uint32_t>(authored->bytes.size()),
                    .source_sector_span = source_entry.sector_span,
                    .output_sector_span = output_sector_span,
                    .source_dds_sha256 = sha256_of(source_dds),
                    .output_dds_sha256 = sha256_of(std::span<const std::byte>{
                        authored->bytes.data(), authored->bytes.size()}),
                });
            }

            put_u32(
                output,
                4U + index * sizeof(std::uint32_t),
                output_sector_span);
        }
    }

    const auto output_span = std::span<const std::byte>{
        output.data(), output.size()};
    const auto reparsed = TextureSlotFramingParser::parse(output_span);
    if (!reparsed.ok() || reparsed.document.kind != source.document.kind ||
        reparsed.document.textures.size() != source.document.textures.size()) {
        return failure(
            TextureSlotSizeSerializerStatus::output_validation_failed,
            "Rebuilt texture slot failed canonical framing validation.");
    }

    for (const auto& patch : patches) {
        const auto* authored = authored_by_index[patch.texture_index];
        const auto& output_entry = reparsed.document.textures[patch.texture_index];
        const auto output_dds = dds_span(output_span, output_entry);
        if (authored == nullptr || output_dds.size() != authored->bytes.size() ||
            !std::equal(
                output_dds.begin(), output_dds.end(), authored->bytes.begin(),
                authored->bytes.end())) {
            return failure(
                TextureSlotSizeSerializerStatus::output_dds_mismatch,
                "Canonical reparse did not reproduce an authored DDS byte image exactly.");
        }
    }

    TextureSlotSizeSerializerReceipt receipt{
        .framing_kind = source.document.kind,
        .source_sha256 = sha256_of(source_physical_slot),
        .output_sha256 = sha256_of(output_span),
        .writer_mode = std::string{kWriterMode},
        .patches = std::move(patches),
    };
    if (!receipt.valid()) {
        return failure(
            TextureSlotSizeSerializerStatus::invalid_receipt,
            "Size-changing texture serializer produced an invalid receipt.");
    }

    TextureSlotSizeSerializerResult result{
        .status = TextureSlotSizeSerializerStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
    if (!result.ok()) {
        return failure(
            TextureSlotSizeSerializerStatus::invalid_receipt,
            "Size-changing texture serializer result failed self-validation.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
