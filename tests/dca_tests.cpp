#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/dca_binary.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "dca-test",
            .logical_path = "room/st001cfg_008.ukn",
            .container_chain = "NBZ[0]/PAC[8]",
            .offset = 4096U,
            .size = size,
        },
        .display_name = "st001cfg_008.ukn",
        .format = "dca",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] std::vector<std::byte> fixture(
    std::size_t records,
    std::size_t trailing = 0U) {
    std::vector<std::byte> bytes(
        dmc::rengine::formats::dca::header_size +
            records * dmc::rengine::formats::dca::record_size + trailing,
        std::byte{0});
    bytes[0] = std::byte{'D'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'A'};
    bytes[3] = std::byte{0};
    for (std::size_t index = 4U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(index & 0xFFU);
    }
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::dca::RecordScanner;
    using dmc::rengine::formats::dca::build_binary_document;
    using dmc::rengine::formats::dca::header_size;
    using dmc::rengine::formats::dca::record_size;

    static_assert(header_size == 0x10U);
    static_assert(record_size == 0x410U);

    const auto bytes = fixture(2U, 3U);
    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{bytes});
    assert(scan.recognized);
    assert(scan.ok());
    assert(scan.records.size() == 2U);
    assert(scan.records[0].offset == 0x10U);
    assert(scan.records[1].offset == 0x420U);
    assert(scan.diagnostics.size() == 1U);
    assert(scan.diagnostics[0].severity == ParseSeverity::warning);
    assert(scan.diagnostics[0].code == "dca.trailing-bytes");

    auto document = build_binary_document(
        resource(bytes.size()),
        std::span<const std::byte>{bytes},
        scan);
    assert(document.has_value());
    assert(document->regions().size() == 3U);
    assert(document->fields().size() == 4U);
    assert(document->ownership().size() == 3U);
    assert(document->coverage_bytes() == bytes.size() - 3U);
    const auto unknown = document->unknown_ranges();
    assert(unknown.size() == 1U);
    assert(unknown[0].offset == bytes.size() - 3U);
    assert(unknown[0].size == 3U);

    const auto header_only = fixture(0U);
    const auto empty = RecordScanner::scan(
        std::span<const std::byte>{header_only});
    assert(empty.recognized);
    assert(empty.ok());
    assert(empty.records.empty());
    assert(empty.diagnostics[0].code == "dca.no-records");

    std::vector<std::byte> truncated(8U, std::byte{0});
    truncated[0] = std::byte{'D'};
    truncated[1] = std::byte{'C'};
    truncated[2] = std::byte{'A'};
    truncated[3] = std::byte{0};
    const auto truncated_scan = RecordScanner::scan(
        std::span<const std::byte>{truncated});
    assert(truncated_scan.recognized);
    assert(!truncated_scan.ok());
    assert(truncated_scan.diagnostics[0].code == "dca.truncated-header");

    const std::vector<std::byte> wrong{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'}, std::byte{'E'}};
    const auto unrecognized = RecordScanner::scan(
        std::span<const std::byte>{wrong});
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());
    assert(!build_binary_document(
        resource(wrong.size()),
        std::span<const std::byte>{wrong},
        unrecognized).has_value());

    return 0;
}
