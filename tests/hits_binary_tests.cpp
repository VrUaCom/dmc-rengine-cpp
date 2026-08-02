#include "dmc_rengine/formats/hits_binary.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_record(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float base) {
    write_u32(bytes, offset, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_u32(
            bytes,
            offset + 4U + index * 4U,
            std::bit_cast<std::uint32_t>(base + static_cast<float>(index)));
    }
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    std::vector<std::byte> bytes(120U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_record(bytes, 8U, 1.0F);
    write_record(bytes, 64U, 20.0F);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "synthetic",
            .logical_path = "stage/collision.hits",
            .container_chain = "PAC[3]",
            .offset = 4096,
            .size = size,
        },
        .display_name = "collision.hits",
        .format = "hits",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;

    const auto bytes = make_fixture();
    const auto scan = RecordScanner::scan(std::span<const std::byte>{bytes});
    assert(scan.ok());

    auto document = build_binary_document(
        resource(bytes.size()),
        std::span<const std::byte>{bytes},
        scan);
    assert(document.has_value());
    assert(document->regions().size() == 3U);
    assert(document->fields().size() == 31U);
    assert(document->ownership().size() == 3U);
    assert(document->coverage_bytes() == 117U);

    const auto unknown = document->unknown_ranges();
    assert(unknown.size() == 1U);
    assert(unknown[0] ==
        dmc::rengine::binary::ByteRange({.offset = 5U, .size = 3U}));
    assert(document->conflicts().empty());
    assert(document->ownership_conflicts().empty());

    const auto first_record = document->find_region("hits-record-00000008");
    assert(first_record != nullptr);
    assert(first_record->range.offset == 8U);
    assert(first_record->range.size == 56U);

    const auto raw_value = document->find_field(
        "hits-record-00000008-value-00");
    assert(raw_value != nullptr);
    assert(raw_value->kind == dmc::rengine::binary::FieldKind::floating_point);
    assert(raw_value->display_value == "1");

    const auto selection = document->selection_at(8U);
    assert(selection.regions.size() == 1U);
    assert(selection.fields.size() == 2U);
    assert(selection.owners.size() == 1U);

    assert(!build_binary_document(
        resource(bytes.size() + 1U),
        std::span<const std::byte>{bytes},
        scan).has_value());

    const std::vector<std::byte> wrong_magic{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'},
        std::byte{'E'}, std::byte{'!'}};
    const auto wrong_scan = RecordScanner::scan(
        std::span<const std::byte>{wrong_magic});
    assert(!build_binary_document(
        resource(wrong_magic.size()),
        std::span<const std::byte>{wrong_magic},
        wrong_scan).has_value());

    return 0;
}
