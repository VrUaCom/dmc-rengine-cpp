#include "dmc_rengine/profiles/dmc3/texture_slot_reintegrator.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::string_view kWriterMode =
    "same-layout-intrinsic-dds-reintegration";
constexpr std::size_t kDdsHeaderSize = 128U;

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] TextureSlotReintegrationResult failure(
    TextureSlotReintegrationStatus status,
    std::string detail) {
    return TextureSlotReintegrationResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] std::span<const std::byte> dds_span(
    std::span<const std::byte> bytes,
    const TextureSlotEntry& entry) {
    return bytes.subspan(
        static_cast<std::size_t>(entry.dds_offset),
        static_cast<std::size_t>(entry.dds_size));
}

[[nodiscard]] bool same_entry(
    const TextureSlotEntry& source,
    const TextureSlotEntry& output) noexcept {
    return source.texture_index == output.texture_index &&
        source.descriptor_offset == output.descriptor_offset &&
        source.dds_offset == output.dds_offset &&
        source.dds_size == output.dds_size &&
        source.dds_payload_size == output.dds_payload_size &&
        source.width == output.width && source.height == output.height &&
        source.mip_map_count == output.mip_map_count &&
        source.compression == output.compression &&
        source.sector_span == output.sector_span;
}

[[nodiscard]] bool same_framing(
    const TextureSlotFramingDocument& source,
    const TextureSlotFramingDocument& output) noexcept {
    if (!source.valid() || !output.valid() || source.kind != output.kind ||
        source.slot_size != output.slot_size ||
        source.textures.size() != output.textures.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < source.textures.size(); ++index) {
        if (!same_entry(source.textures[index], output.textures[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool non_dds_bytes_equal(
    std::span<const std::byte> source,
    std::span<const std::byte> output,
    const TextureSlotFramingDocument& framing) noexcept {
    if (source.size() != output.size() || !framing.valid() ||
        framing.slot_size != source.size()) {
        return false;
    }

    std::size_t cursor = 0U;
    for (const auto& entry : framing.textures) {
        const auto begin = static_cast<std::size_t>(entry.dds_offset);
        const auto end = begin + static_cast<std::size_t>(entry.dds_size);
        if (begin < cursor || end > source.size() ||
            !std::equal(
                source.begin() + static_cast<std::ptrdiff_t>(cursor),
                source.begin() + static_cast<std::ptrdiff_t>(begin),
                output.begin() + static_cast<std::ptrdiff_t>(cursor),
                output.begin() + static_cast<std::ptrdiff_t>(begin))) {
            return false;
        }
        cursor = end;
    }

    return std::equal(
        source.begin() + static_cast<std::ptrdiff_t>(cursor), source.end(),
        output.begin() + static_cast<std::ptrdiff_t>(cursor), output.end());
}

} // namespace

bool TextureDdsPatchReceipt::valid() const noexcept {
    return dds_size != 0U && source_sha256.size() == 64U &&
        output_sha256.size() == 64U && source_sha256 != output_sha256;
}

bool TextureSlotReintegrationReceipt::valid() const {
    if (source_sha256.size() != 64U || output_sha256.size() != 64U ||
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

bool TextureSlotReintegrationResult::ok() const {
    if (status != TextureSlotReintegrationStatus::ok ||
        !receipt.has_value() || !receipt->valid() || bytes.empty()) {
        return false;
    }
    return receipt->output_sha256 == sha256_of(
        std::span<const std::byte>{bytes.data(), bytes.size()});
}

TextureSlotReintegrationResult TextureSlotReintegrator::rebuild(
    std::span<const std::byte> source_physical_slot,
    std::span<const AuthoredTextureDds> authored_textures,
    TextureSlotFramingSafety safety) {
    const auto source_parsed = TextureSlotFramingParser::parse(
        source_physical_slot, safety);
    if (!source_parsed.ok()) {
        return failure(
            TextureSlotReintegrationStatus::invalid_source_framing,
            "Source physical slot is not a validated DMC3 texture framing: " +
                std::string{source_parsed.detail});
    }
    if (authored_textures.empty()) {
        return failure(
            TextureSlotReintegrationStatus::no_authored_textures,
            "At least one authored intrinsic DDS image is required.");
    }

    std::unordered_set<std::uint32_t> seen_inputs;
    seen_inputs.reserve(authored_textures.size());
    for (const auto& authored : authored_textures) {
        if (!seen_inputs.insert(authored.texture_index).second) {
            return failure(
                TextureSlotReintegrationStatus::duplicate_texture_input,
                "The authored DDS set contains the same texture index more than once.");
        }
        if (authored.texture_index >= source_parsed.document.textures.size()) {
            return failure(
                TextureSlotReintegrationStatus::texture_not_found,
                "An authored DDS texture index does not exist in the source framing.");
        }
        if (authored.expected_source_sha256.size() != 64U) {
            return failure(
                TextureSlotReintegrationStatus::missing_source_hash,
                "Every authored DDS must carry an exact 64-character source SHA-256.");
        }

        const auto& entry =
            source_parsed.document.textures[authored.texture_index];
        const auto source_dds = dds_span(source_physical_slot, entry);
        if (sha256_of(source_dds) != authored.expected_source_sha256) {
            return failure(
                TextureSlotReintegrationStatus::source_dds_mismatch,
                "An authored DDS source hash does not match the bounded source DDS bytes.");
        }
        if (authored.bytes.size() != entry.dds_size) {
            return failure(
                TextureSlotReintegrationStatus::dds_size_changed,
                "Pass 79 permits only byte-size-preserving intrinsic DDS edits.");
        }
        if (source_dds.size() < kDdsHeaderSize ||
            !std::equal(
                source_dds.begin(),
                source_dds.begin() + static_cast<std::ptrdiff_t>(kDdsHeaderSize),
                authored.bytes.begin(),
                authored.bytes.begin() + static_cast<std::ptrdiff_t>(kDdsHeaderSize))) {
            return failure(
                TextureSlotReintegrationStatus::dds_header_changed,
                "Pass 79 freezes the complete 128-byte DDS header; only compressed payload bytes may change.");
        }
    }

    std::vector<std::byte> output{
        source_physical_slot.begin(), source_physical_slot.end()};
    std::vector<TextureDdsPatchReceipt> patches;
    patches.reserve(authored_textures.size());

    for (const auto& authored : authored_textures) {
        const auto& entry =
            source_parsed.document.textures[authored.texture_index];
        const auto source_dds = dds_span(source_physical_slot, entry);
        if (std::equal(
                source_dds.begin(), source_dds.end(), authored.bytes.begin(),
                authored.bytes.end())) {
            continue;
        }

        const auto destination =
            output.begin() + static_cast<std::ptrdiff_t>(entry.dds_offset);
        std::copy(authored.bytes.begin(), authored.bytes.end(), destination);
        patches.push_back(TextureDdsPatchReceipt{
            .texture_index = authored.texture_index,
            .dds_offset = entry.dds_offset,
            .dds_size = entry.dds_size,
            .source_sha256 = sha256_of(source_dds),
            .output_sha256 = sha256_of(std::span<const std::byte>{
                authored.bytes.data(), authored.bytes.size()}),
        });
    }

    if (patches.empty()) {
        return failure(
            TextureSlotReintegrationStatus::no_changes,
            "All authored DDS images are byte-identical to their bounded sources.");
    }

    const auto output_span = std::span<const std::byte>{
        output.data(), output.size()};
    const auto output_parsed = TextureSlotFramingParser::parse(output_span, safety);
    if (!output_parsed.ok()) {
        return failure(
            TextureSlotReintegrationStatus::output_validation_failed,
            "Reintegrated physical slot failed canonical texture framing validation: " +
                std::string{output_parsed.detail});
    }
    if (!same_framing(source_parsed.document, output_parsed.document)) {
        return failure(
            TextureSlotReintegrationStatus::framing_changed,
            "Intrinsic DDS reintegration changed physical texture framing metadata or geometry.");
    }
    if (!non_dds_bytes_equal(
            source_physical_slot, output_span, source_parsed.document)) {
        return failure(
            TextureSlotReintegrationStatus::non_dds_bytes_changed,
            "Bytes outside intrinsic DDS ranges changed during reintegration.");
    }

    TextureSlotReintegrationReceipt receipt{
        .framing_kind = source_parsed.document.kind,
        .source_sha256 = sha256_of(source_physical_slot),
        .output_sha256 = sha256_of(output_span),
        .writer_mode = std::string{kWriterMode},
        .patches = std::move(patches),
    };
    if (!receipt.valid()) {
        return failure(
            TextureSlotReintegrationStatus::invalid_receipt,
            "Texture reintegration produced an internally invalid receipt.");
    }

    TextureSlotReintegrationResult result{
        .status = TextureSlotReintegrationStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
    if (!result.ok()) {
        return failure(
            TextureSlotReintegrationStatus::invalid_receipt,
            "Texture reintegration result failed its self-validation contract.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
