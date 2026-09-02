#include "dmc_rengine/formats/so.hpp"

#include <bit>
#include <utility>

namespace dmc::rengine::formats::so {
namespace {

[[nodiscard]] std::uint16_t read_u16(std::span<const std::byte> bytes, const std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset])) |
           static_cast<std::uint16_t>(static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U);
}

[[nodiscard]] std::uint32_t read_u32(std::span<const std::byte> bytes, const std::size_t offset) noexcept {
    return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset])) |
           (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U) |
           (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 2U])) << 16U) |
           (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 3U])) << 24U);
}

[[nodiscard]] float read_f32(std::span<const std::byte> bytes, const std::size_t offset) noexcept {
    return std::bit_cast<float>(read_u32(bytes, offset));
}

[[nodiscard]] Vec4 read_vec4(std::span<const std::byte> bytes, const std::size_t offset) noexcept {
    return Vec4{
        read_f32(bytes, offset),
        read_f32(bytes, offset + 4U),
        read_f32(bytes, offset + 8U),
        read_f32(bytes, offset + 12U),
    };
}

[[nodiscard]] bool parse_indexed_block(std::span<const std::byte> bytes,
                                       const std::size_t base,
                                       const std::size_t extent,
                                       const std::size_t header_size,
                                       IndexedBlock& out,
                                       std::vector<Diagnostic>& diagnostics) {
    if (extent < header_size + 2U || base > bytes.size() || extent > bytes.size() - base) {
        diagnostics.push_back({"SO indexed block is truncated"});
        return false;
    }

    out.type = read_u16(bytes, base);
    out.base_offset = base;
    out.extent_size = extent;

    out.header_words.clear();
    for (std::size_t offset = 0U; offset < header_size; offset += 2U) {
        out.header_words.push_back(read_u16(bytes, base + offset));
    }

    const auto first_entry_offset = static_cast<std::size_t>(read_u16(bytes, base + header_size));
    if (first_entry_offset < header_size || ((first_entry_offset - header_size) % 2U) != 0U || first_entry_offset >= extent) {
        diagnostics.push_back({"SO entry-offset table does not close on its first entry"});
        return false;
    }

    const auto count = (first_entry_offset - header_size) / 2U;
    if (count == 0U || count > (extent - header_size) / 2U) {
        diagnostics.push_back({"SO entry-offset count is invalid"});
        return false;
    }

    out.entry_offsets.clear();
    out.entry_offsets.reserve(count);
    std::uint16_t previous = 0U;
    for (std::size_t index = 0U; index < count; ++index) {
        const auto value = read_u16(bytes, base + header_size + index * 2U);
        if (static_cast<std::size_t>(value) >= extent || (index != 0U && value <= previous)) {
            diagnostics.push_back({"SO entry offsets are not strictly increasing inside the block"});
            return false;
        }
        out.entry_offsets.push_back(value);
        previous = value;
    }

    if (out.entry_offsets.front() != first_entry_offset) {
        diagnostics.push_back({"SO first entry offset is inconsistent with offset-table closure"});
        return false;
    }

    return true;
}

} // namespace

bool GraphParseResult::ok() const noexcept {
    return recognized && diagnostics.empty();
}

bool LinkParseResult::ok() const noexcept {
    return recognized && diagnostics.empty();
}

bool VolumeParseResult::ok() const noexcept {
    return recognized && diagnostics.empty();
}

GraphParseResult parse_graph(const std::span<const std::byte> bytes) {
    GraphParseResult result;
    if (bytes.size() < type6_header_size + 2U || read_u16(bytes, 0U) != 6U) {
        return result;
    }
    result.recognized = true;

    const auto type8_base = static_cast<std::size_t>(read_u16(bytes, 2U));
    if (type8_base <= type6_header_size || type8_base + type8_header_size + 2U > bytes.size()) {
        result.diagnostics.push_back({"SO type-6 boundary does not point to an in-range second block"});
        return result;
    }
    if (read_u16(bytes, type8_base) != 8U) {
        result.diagnostics.push_back({"SO type-6 boundary does not point to the observed type-8 companion block"});
        return result;
    }

    IndexedBlock type6;
    if (!parse_indexed_block(bytes, 0U, type8_base, type6_header_size, type6, result.diagnostics)) {
        return result;
    }

    IndexedBlock type8;
    if (!parse_indexed_block(bytes,
                             type8_base,
                             bytes.size() - type8_base,
                             type8_header_size,
                             type8,
                             result.diagnostics)) {
        return result;
    }

    result.blocks.push_back(std::move(type6));
    result.blocks.push_back(std::move(type8));
    return result;
}

LinkParseResult parse_links(const std::span<const std::byte> bytes) {
    LinkParseResult result;
    if (bytes.empty() || (bytes.size() % link_record_size) != 0U) {
        return result;
    }
    result.recognized = true;
    result.records.reserve(bytes.size() / link_record_size);
    for (std::size_t offset = 0U; offset < bytes.size(); offset += link_record_size) {
        result.records.push_back(LinkRecord{
            std::to_integer<std::uint8_t>(bytes[offset]),
            std::to_integer<std::uint8_t>(bytes[offset + 1U]),
            std::to_integer<std::uint8_t>(bytes[offset + 2U]),
            std::to_integer<std::uint8_t>(bytes[offset + 3U]),
        });
    }
    return result;
}

VolumeParseResult parse_volumes(const std::span<const std::byte> bytes) {
    VolumeParseResult result;
    if (bytes.empty() || (bytes.size() % volume_record_size) != 0U) {
        return result;
    }
    result.recognized = true;
    result.records.reserve(bytes.size() / volume_record_size);

    for (std::size_t offset = 0U; offset < bytes.size(); offset += volume_record_size) {
        VolumeRecord record;
        record.type = read_u32(bytes, offset);
        for (std::size_t i = 0U; i < record.prefix_unknown.size(); ++i) {
            record.prefix_unknown[i] = bytes[offset + 4U + i];
        }
        record.vector0 = read_vec4(bytes, offset + 0x10U);
        record.vector1 = read_vec4(bytes, offset + 0x20U);
        record.vector2 = read_vec4(bytes, offset + 0x30U);
        record.vector3 = read_vec4(bytes, offset + 0x40U);
        result.records.push_back(record);
    }
    return result;
}

CompanionCorrelation correlate_companions(const LinkParseResult& links,
                                          const VolumeParseResult& volumes) noexcept {
    CompanionCorrelation result;
    result.link_record_count = links.records.size();
    result.volume_record_count = volumes.records.size();
    result.one_header_plus_one_link_per_volume =
        links.ok() && volumes.ok() && links.records.size() == volumes.records.size() + 1U;
    return result;
}

} // namespace dmc::rengine::formats::so
