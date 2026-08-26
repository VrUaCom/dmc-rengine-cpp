#include "dmc_rengine/formats/ptx.hpp"

#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] bool image_signature_at(
    std::span<const std::byte> bytes,
    std::uint64_t offset) noexcept {
    static constexpr unsigned char signature[]{'D', 'D', 'S', ' '};
    if (offset > bytes.size() - sizeof(signature)) {
        return false;
    }
    for (std::size_t index = 0; index < sizeof(signature); ++index) {
        if (std::to_integer<unsigned char>(
                bytes[static_cast<std::size_t>(offset) + index]) !=
            signature[index]) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string synthetic_texture_name(std::uint32_t index) {
    std::ostringstream output;
    output << "texture_" << std::setfill('0') << std::setw(4) << index
           << ".dds";
    return output.str();
}

[[nodiscard]] PtxParseResult fail(PtxParseError error, std::string message) {
    return PtxParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

PtxParseResult PtxParser::parse(std::span<const std::byte> bytes) {
    if (bytes.size() < k_sector_bytes + k_sector_bytes) {
        return fail(
            PtxParseError::truncated_header,
            "texture pack requires a header sector and at least one block");
    }

    const auto texture_count = read_u32_le(bytes, 0U);
    if (texture_count == 0U || texture_count > k_max_texture_count) {
        return fail(
            PtxParseError::texture_count_limit,
            "declared texture count cannot be described by the header sector");
    }

    const auto table_end =
        4U + static_cast<std::size_t>(texture_count) * 4U;
    if (table_end > k_sector_bytes) {
        return fail(
            PtxParseError::truncated_size_table,
            "sector size table does not fit inside the header sector");
    }

    std::vector<std::uint32_t> sectors(texture_count, 0U);
    std::uint64_t total_sectors = 0U;
    for (std::uint32_t index = 0U; index < texture_count; ++index) {
        const auto value = read_u32_le(bytes, 4U + static_cast<std::size_t>(index) * 4U);
        if (value == 0U) {
            return fail(
                PtxParseError::invalid_sector_size,
                "a declared texture block occupies no sectors");
        }
        sectors[index] = value;
        total_sectors += value;
    }

    // The closure check. Header sector plus every declared block must be the
    // stored length exactly — not at most, not rounded. This is what makes a
    // magic-free format safe to recognize, so it is an error and never a
    // warning.
    if (total_sectors >
        (std::numeric_limits<std::uint64_t>::max() / k_sector_bytes) - 1U) {
        return fail(
            PtxParseError::size_table_does_not_close,
            "declared sector total overflows the addressable range");
    }
    const auto described = (total_sectors + 1U) * k_sector_bytes;
    if (described != static_cast<std::uint64_t>(bytes.size())) {
        return fail(
            PtxParseError::size_table_does_not_close,
            "declared sector total does not reproduce the stored length");
    }

    ContainerDocument document;
    document.format = "ptx";
    document.schema_version = 1U;
    document.declared_slot_count = texture_count;
    document.container_size = static_cast<std::uint64_t>(bytes.size());
    document.entries.resize(texture_count);

    std::uint64_t block_offset = k_sector_bytes;
    for (std::uint32_t index = 0U; index < texture_count; ++index) {
        const auto block_size =
            static_cast<std::uint64_t>(sectors[index]) * k_sector_bytes;
        if (block_size <= k_descriptor_bytes) {
            return fail(
                PtxParseError::invalid_sector_size,
                "a declared texture block is smaller than its descriptor");
        }

        const auto image_offset = block_offset + k_descriptor_bytes;
        if (!image_signature_at(bytes, image_offset)) {
            return fail(
                PtxParseError::missing_image_signature,
                "a declared texture block does not open with a DDS image");
        }

        auto& entry = document.entries[index];
        entry.slot_index = index;
        // The per-texture descriptor is container framing, like a local file
        // header, so the addressable child is the image it introduces. The
        // block's trailing sector padding stays inside the child, because that
        // padding is part of what the container stores for this texture and
        // dropping it would make the extracted bytes a reconstruction rather
        // than a copy.
        entry.offset = image_offset;
        entry.size = block_size - k_descriptor_bytes;
        entry.logical_name = synthetic_texture_name(index);
        entry.populated = true;
        entry.synthetic_name = true;

        block_offset += block_size;
    }

    if (!document.valid()) {
        return fail(
            PtxParseError::invalid_document,
            "texture pack decoded to an invalid ContainerDocument");
    }

    return PtxParseResult{
        .document = std::move(document),
        .error = PtxParseError::none,
        .message = {},
    };
}

bool PtxParser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    // Recognition and materialization must never disagree about what a texture
    // pack is, so the probe is the parser.
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats
