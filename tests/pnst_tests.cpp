#include "dmc_rengine/formats/pnst.hpp"

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

std::vector<std::byte> make_sparse_pnst() {
    // Evidence-shaped sparse topology: 11 declared physical slots with only
    // slots 0 and 10 populated. Intermediate slots must never be compacted.
    std::vector<std::byte> bytes(0x70U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 11U); // table end = 0x34
    put_u32(bytes, 8U, 0x38U); // valid residue-8 offset
    put_u32(bytes, 8U + 10U * 4U, 0x50U);
    return bytes;
}

} // namespace

int main() {
    namespace formats = dmc::rengine::formats;

    const auto bytes = make_sparse_pnst();
    const auto parsed = formats::PnstParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document.has_value());

    const auto& document = *parsed.document;
    assert(document.format == "PNST");
    assert(document.schema_version == 1U);
    assert(document.declared_slot_count == 11U);
    assert(document.entries.size() == 11U);
    assert(document.entries[0].populated);
    assert(document.entries[0].offset == 0x38U);
    assert(document.entries[0].size == 0x18U);
    assert(document.entries[0].logical_name == "slot_0000.bin");
    assert(document.entries[0].synthetic_name);

    for (std::uint32_t slot = 1U; slot < 10U; ++slot) {
        assert(!document.entries[slot].populated);
        assert(document.entries[slot].offset == 0U);
        assert(document.entries[slot].size == 0U);
        assert(document.entries[slot].synthetic_name);
    }

    assert(document.entries[10].populated);
    assert(document.entries[10].offset == 0x50U);
    assert(document.entries[10].size == 0x20U);
    assert(document.entries[10].logical_name == "slot_0010.bin");

    auto wrong_magic = bytes;
    wrong_magic[1] = std::byte{'A'};
    assert(
        formats::PnstParser::parse(wrong_magic).error ==
        formats::PnstParseError::invalid_magic);

    std::vector<std::byte> short_header(7U, std::byte{0});
    assert(
        formats::PnstParser::parse(short_header).error ==
        formats::PnstParseError::truncated_header);

    auto into_table = bytes;
    put_u32(into_table, 8U, 0x20U);
    assert(
        formats::PnstParser::parse(into_table).error ==
        formats::PnstParseError::slot_offset_before_payload);

    auto at_end = bytes;
    put_u32(at_end, 8U, static_cast<std::uint32_t>(at_end.size()));
    assert(
        formats::PnstParser::parse(at_end).error ==
        formats::PnstParseError::slot_offset_out_of_bounds);

    auto huge_count = bytes;
    put_u32(
        huge_count,
        4U,
        formats::PnstParser::k_max_slot_count + 1U);
    assert(
        formats::PnstParser::parse(huge_count).error ==
        formats::PnstParseError::slot_count_limit);

    return 0;
}
