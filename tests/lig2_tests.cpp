#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/lig2_binary.hpp"

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
            .source_id = "lig2-test",
            .logical_path = "room/st001cfg_001.lig",
            .container_chain = "NBZ[0]/PAC[1]",
            .offset = 8192U,
            .size = size,
        },
        .display_name = "st001cfg_001.lig",
        .format = "lig2",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] std::vector<std::byte> fixture(
    std::size_t records,
    std::size_t trailing = 0U) {
    std::vector<std::byte> bytes(
        dmc::rengine::formats::lig2::header_size +
            records * dmc::rengine::formats::lig2::record_size + trailing,
        std::byte{0});
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>((index * 7U) & 0xFFU);
    }
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::lig2::RecordScanner;
    using dmc::rengine::formats::lig2::build_binary_document;
    using dmc::rengine::formats::lig2::expected_dmc3_file_size;
    using dmc::rengine::formats::lig2::expected_dmc3_record_count;
    using dmc::rengine::formats::lig2::header_size;
    using dmc::rengine::formats::lig2::record_size;

    static_assert(header_size == 0x20U);
    static_assert(record_size == 0x30U);
    static_assert(expected_dmc3_record_count == 48U);
    static_assert(expected_dmc3_file_size == 2336U);

    const auto bytes = fixture(expected_dmc3_record_count);
    assert(bytes.size() == expected_dmc3_file_size);
    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{bytes});
    assert(scan.recognized);
    assert(scan.ok());
    assert(scan.records.size() == expected_dmc3_record_count);
    assert(scan.diagnostics.empty());
    assert(scan.records.front().offset == 0x20U);
    assert(scan.records.back().offset == 0x8F0U);

    auto document = build_binary_document(
        resource(bytes.size()),
        std::span<const std::byte>{bytes},
        scan);
    assert(document.has_value());
    assert(document->regions().size() == 49U);
    assert(document->fields().size() == 49U);
    assert(document->ownership().size() == 49U);
    assert(document->coverage_bytes() == bytes.size());
    assert(document->unknown_ranges().empty());

    const auto short_corpus = fixture(2U, 5U);
    const auto short_scan = RecordScanner::scan(
        std::span<const std::byte>{short_corpus});
    assert(short_scan.recognized);
    assert(short_scan.ok());
    assert(short_scan.records.size() == 2U);
    assert(short_scan.diagnostics.size() == 2U);
    assert(short_scan.diagnostics[0].severity == ParseSeverity::warning);
    assert(short_scan.diagnostics[0].code == "lig2.trailing-bytes");
    assert(short_scan.diagnostics[1].code ==
        "lig2.unexpected-record-count");

    auto short_document = build_binary_document(
        resource(short_corpus.size()),
        std::span<const std::byte>{short_corpus},
        short_scan);
    assert(short_document.has_value());
    assert(short_document->unknown_ranges().size() == 1U);
    assert(short_document->unknown_ranges()[0].size == 5U);

    const std::vector<std::byte> truncated(0x10U, std::byte{0});
    const auto truncated_scan = RecordScanner::scan(
        std::span<const std::byte>{truncated});
    assert(!truncated_scan.recognized);
    assert(!truncated_scan.ok());
    assert(truncated_scan.diagnostics[0].code ==
        "lig2.truncated-header");
    assert(!build_binary_document(
        resource(truncated.size()),
        std::span<const std::byte>{truncated},
        truncated_scan).has_value());

    return 0;
}
