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

[[nodiscard]] std::uint64_t read_u64(std::span<const std::byte> bytes, const std::size_t offset) noexcept {
    return static_cast<std::uint64_t>(read_u32(bytes, offset)) |
           (static_cast<std::uint64_t>(read_u32(bytes, offset + 4U)) << 32U);
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

bool ModTransformDomain::ok() const noexcept {
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

ModTransformDomain parse_mod_transform_domain(const std::span<const std::byte> bytes) {
    ModTransformDomain result;
    if (bytes.size() < 0x28U ||
        bytes[0U] != std::byte{'M'} || bytes[1U] != std::byte{'O'} ||
        bytes[2U] != std::byte{'D'} || bytes[3U] != std::byte{' '}) {
        return result;
    }
    result.recognized = true;
    result.raw_domain_count = std::to_integer<std::uint8_t>(bytes[0x11U]);
    if (result.raw_domain_count == 0U) {
        result.diagnostics.push_back({"MOD transform-domain count is zero"});
        return result;
    }

    result.document_offset = read_u64(bytes, 0x20U);
    if (result.document_offset > static_cast<std::uint64_t>(bytes.size()) ||
        static_cast<std::uint64_t>(bytes.size()) - result.document_offset < 0x10U) {
        result.diagnostics.push_back({"MOD document pointer is out of range"});
        return result;
    }

    const auto document = static_cast<std::size_t>(result.document_offset);
    const auto table0_rel = static_cast<std::size_t>(read_u32(bytes, document));
    const auto table1_rel = static_cast<std::size_t>(read_u32(bytes, document + 4U));
    const auto table2_rel = static_cast<std::size_t>(read_u32(bytes, document + 8U));
    const auto count = static_cast<std::size_t>(result.raw_domain_count);

    if (table0_rel >= table1_rel || table1_rel >= table2_rel ||
        table1_rel - table0_rel < count || table2_rel - table1_rel < count ||
        document + table2_rel > bytes.size()) {
        result.diagnostics.push_back({"MOD transform-domain table spans are inconsistent"});
        return result;
    }

    result.reference_table.reserve(count);
    result.permutation_table.reserve(count);
    for (std::size_t index = 0U; index < count; ++index) {
        result.reference_table.push_back(std::to_integer<std::uint8_t>(bytes[document + table0_rel + index]));
        result.permutation_table.push_back(std::to_integer<std::uint8_t>(bytes[document + table1_rel + index]));
    }

    std::vector<std::int16_t> inverse(count, static_cast<std::int16_t>(-1));
    result.permutation_is_complete = true;
    for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
        const auto physical_index = static_cast<std::size_t>(result.permutation_table[logical_index]);
        if (physical_index >= count || inverse[physical_index] != static_cast<std::int16_t>(-1)) {
            result.permutation_is_complete = false;
            break;
        }
        inverse[physical_index] = static_cast<std::int16_t>(logical_index);
    }

    if (!result.permutation_is_complete) {
        return result;
    }

    result.derived_hierarchy_candidate.reserve(count);
    result.hierarchy_candidate_is_acyclic = true;
    for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
        const auto raw_reference = result.reference_table[logical_index];
        if (raw_reference == 0xFFU) {
            result.derived_hierarchy_candidate.push_back(static_cast<std::int16_t>(-1));
            continue;
        }
        const auto reference_index = static_cast<std::size_t>(raw_reference);
        if (reference_index >= count || inverse[reference_index] < 0) {
            result.derived_hierarchy_candidate.push_back(static_cast<std::int16_t>(-2));
            result.hierarchy_candidate_is_acyclic = false;
            continue;
        }
        const auto derived = inverse[reference_index];
        result.derived_hierarchy_candidate.push_back(derived);
        if (derived >= static_cast<std::int16_t>(logical_index)) {
            result.hierarchy_candidate_is_acyclic = false;
        }
    }
    return result;
}

ModCompanionCorrelation correlate_mod_companions(const ModTransformDomain& mod,
                                                 const LinkParseResult& links,
                                                 const VolumeParseResult& volumes) noexcept {
    ModCompanionCorrelation result;
    result.domain_count = static_cast<std::size_t>(mod.raw_domain_count);
    result.post_prefix_link_count = links.records.empty() ? 0U : links.records.size() - 1U;
    result.volume_count = volumes.records.size();
    if (!mod.ok() || !links.ok() || !volumes.ok()) {
        return result;
    }

    result.link_middle_fields_fit_domain = !links.records.empty();
    for (std::size_t index = 1U; index < links.records.size(); ++index) {
        const auto& record = links.records[index];
        if (static_cast<std::size_t>(record.field1) >= result.domain_count ||
            static_cast<std::size_t>(record.field2) >= result.domain_count) {
            result.link_middle_fields_fit_domain = false;
            break;
        }
    }

    result.post_prefix_link_count_equals_domain = result.post_prefix_link_count == result.domain_count;
    result.volume_count_equals_domain = result.volume_count == result.domain_count;
    result.complete_cardinality_alignment = result.link_middle_fields_fit_domain &&
                                            result.post_prefix_link_count_equals_domain &&
                                            result.volume_count_equals_domain;
    return result;
}

} // namespace dmc::rengine::formats::so
