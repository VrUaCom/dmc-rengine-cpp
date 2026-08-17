#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

[[nodiscard]] std::vector<std::byte> make_sparse_pnst() {
    // Mirrors the evidence shape of an 11-slot sparse PNST with only original
    // slots 0 and 10 populated. Physical identity must not compact to 0/1.
    std::vector<std::byte> bytes(0x60U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 11U);
    put_u32(bytes, 8U, 0x40U);
    put_u32(bytes, 8U + 10U * 4U, 0x50U);
    return bytes;
}

} // namespace

int main() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto bytes = make_sparse_pnst();
    const auto parsed = formats::PnstParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->format == "PNST");
    assert(parsed.document->declared_slot_count == 11U);
    assert(parsed.document->entries.size() == 11U);

    assert(parsed.document->entries[0].populated);
    assert(parsed.document->entries[0].offset == 0x40U);
    assert(parsed.document->entries[0].size == 0x10U);
    assert(parsed.document->entries[0].logical_name == "slot_0000.bin");

    for (std::uint32_t slot = 1U; slot < 10U; ++slot) {
        const auto& entry = parsed.document->entries[slot];
        assert(entry.slot_index == slot);
        assert(!entry.populated);
        assert(entry.logical_name != "");
        assert(entry.synthetic_name);
    }

    assert(parsed.document->entries[10].populated);
    assert(parsed.document->entries[10].slot_index == 10U);
    assert(parsed.document->entries[10].offset == 0x50U);
    assert(parsed.document->entries[10].size == 0x10U);
    assert(parsed.document->entries[10].logical_name == "slot_0010.bin");

    // Magic identity outranks the misleading .pac extension.
    const auto classification = gdspaces::ResourceClassifier::classify(
        "DMC3/st001_005.pac",
        std::span<const std::byte>{bytes});
    assert(classification.format == "pnst");
    assert(classification.magic_confirmed);
    assert(classification.container);

    auto pac_magic = bytes;
    pac_magic[1] = std::byte{'A'};
    pac_magic[2] = std::byte{'C'};
    pac_magic[3] = std::byte{0};
    assert(
        formats::PnstParser::parse(pac_magic).error ==
        formats::PnstParseError::invalid_magic);

    auto into_table = bytes;
    put_u32(into_table, 8U, 0x20U);
    assert(
        formats::PnstParser::parse(into_table).error ==
        formats::PnstParseError::slot_offset_before_payload);

    return 0;
}
