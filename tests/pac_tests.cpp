#include "dmc_rengine/formats/pac.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

std::vector<std::byte> make_pac() {
    // Five declared slots. Slot 1 is empty and slot 3 aliases slot 0's
    // physical span. Extents are a bounded product extraction rule based on
    // the next greater distinct populated offset, not slot-table ordering.
    std::vector<std::byte> bytes(0x50U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 5U); // table end = 0x1c
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0U);
    put_u32(bytes, 16U, 0x30U);
    put_u32(bytes, 20U, 0x20U);
    put_u32(bytes, 24U, 0x40U);
    return bytes;
}

} // namespace

int main() {
    namespace formats = dmc::rengine::formats;

    const auto bytes = make_pac();
    const auto parsed = formats::PacParser::parse(bytes);
    assert(parsed.ok());

    const auto& document = *parsed.document;
    assert(document.format == "PAC");
    assert(document.schema_version == 1U);
    assert(document.declared_slot_count == 5U);
    assert(document.container_size == 0x50U);
    assert(document.entries.size() == 5U);

    assert(document.entries[0].populated);
    assert(document.entries[0].offset == 0x20U);
    assert(document.entries[0].size == 0x10U);
    assert(document.entries[0].logical_name == "slot_0000.bin");
    assert(document.entries[0].synthetic_name);

    assert(!document.entries[1].populated);
    assert(document.entries[1].logical_name == "slot_0001.empty");
    assert(document.entries[1].synthetic_name);

    assert(document.entries[2].populated);
    assert(document.entries[2].offset == 0x30U);
    assert(document.entries[2].size == 0x10U);

    assert(document.entries[3].populated);
    assert(document.entries[3].offset == 0x20U);
    assert(document.entries[3].size == 0x10U);

    assert(document.entries[4].populated);
    assert(document.entries[4].offset == 0x40U);
    assert(document.entries[4].size == 0x10U);

    auto bad_magic = bytes;
    bad_magic[0] = std::byte{'X'};
    assert(
        formats::PacParser::parse(bad_magic).error ==
        formats::PacParseError::invalid_magic);

    std::vector<std::byte> short_header(7U, std::byte{0});
    assert(
        formats::PacParser::parse(short_header).error ==
        formats::PacParseError::truncated_header);

    auto truncated_table = bytes;
    put_u32(truncated_table, 4U, 100U);
    assert(
        formats::PacParser::parse(truncated_table).error ==
        formats::PacParseError::truncated_offset_table);

    auto into_table = bytes;
    put_u32(into_table, 8U, 0x10U);
    assert(
        formats::PacParser::parse(into_table).error ==
        formats::PacParseError::slot_offset_before_payload);

    auto at_end = bytes;
    put_u32(at_end, 8U, static_cast<std::uint32_t>(at_end.size()));
    assert(
        formats::PacParser::parse(at_end).error ==
        formats::PacParseError::slot_offset_out_of_bounds);

    auto huge_count = bytes;
    put_u32(
        huge_count,
        4U,
        formats::PacParser::k_max_slot_count + 1U);
    assert(
        formats::PacParser::parse(huge_count).error ==
        formats::PacParseError::slot_count_limit);

    // Real Pass-14/17 corpus evidence confirms zero-count PACs exist and are
    // structurally valid; this synthetic regression locks the product rule.
    std::vector<std::byte> empty(8U, std::byte{0});
    empty[0] = std::byte{'P'};
    empty[1] = std::byte{'A'};
    empty[2] = std::byte{'C'};
    empty[3] = std::byte{0};
    put_u32(empty, 4U, 0U);
    const auto empty_result = formats::PacParser::parse(empty);
    assert(empty_result.ok());
    assert(empty_result.document->entries.empty());

    // No universal alignment rejection: latest corpus evidence has populated
    // offsets with residue 8 mod 16. A structurally bounded unaligned payload
    // therefore remains valid.
    std::vector<std::byte> residue8(0x30U, std::byte{0});
    residue8[0] = std::byte{'P'};
    residue8[1] = std::byte{'A'};
    residue8[2] = std::byte{'C'};
    residue8[3] = std::byte{0};
    put_u32(residue8, 4U, 1U);
    put_u32(residue8, 8U, 0x18U);
    const auto residue_result = formats::PacParser::parse(residue8);
    assert(residue_result.ok());
    assert(residue_result.document->entries[0].offset == 0x18U);

    return 0;
}
