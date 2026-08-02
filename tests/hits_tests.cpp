#include "dmc_rengine/formats/hits.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_record(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float base) {
    write_u32(bytes, offset, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_f32(bytes, offset + 4U + index * 4U,
                  base + static_cast<float>(index));
    }
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    std::vector<std::byte> bytes(120U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_record(bytes, 8U, 10.0F);
    write_record(bytes, 64U, -5.0F);
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::record_marker;
    using dmc::rengine::formats::hits::record_size;

    static_assert(record_size == 56U);

    const auto bytes = make_fixture();
    const auto result = RecordScanner::scan(
        std::span<const std::byte>{bytes});
    assert(result.recognized);
    assert(result.ok());
    assert(result.records.size() == 2U);
    assert(result.records[0].offset == 8U);
    assert(result.records[0].marker == record_marker);
    assert(result.records[0].values[0] == 10.0F);
    assert(result.records[0].values[12] == 22.0F);
    assert(result.records[1].offset == 64U);
    assert(result.records[1].values[0] == -5.0F);
    assert(result.records[1].values[12] == 7.0F);

    const std::vector<std::byte> wrong_magic{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'},
        std::byte{'E'}, std::byte{'!'}};
    const auto unrecognized = RecordScanner::scan(
        std::span<const std::byte>{wrong_magic});
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());
    assert(unrecognized.records.empty());

    std::vector<std::byte> magic_only{
        std::byte{'H'}, std::byte{'I'}, std::byte{'T'},
        std::byte{'S'}, std::byte{'$'}};
    const auto empty = RecordScanner::scan(
        std::span<const std::byte>{magic_only});
    assert(empty.recognized);
    assert(empty.ok());
    assert(empty.records.empty());
    assert(empty.diagnostics.size() == 1U);
    assert(empty.diagnostics[0].severity == ParseSeverity::warning);
    assert(empty.diagnostics[0].code == "hits.no_records");

    auto truncated = magic_only;
    truncated.resize(5U + 4U + 20U, std::byte{0});
    write_u32(truncated, 5U, record_marker);
    const auto truncated_result = RecordScanner::scan(
        std::span<const std::byte>{truncated});
    assert(truncated_result.recognized);
    assert(!truncated_result.ok());
    assert(truncated_result.records.empty());
    assert(truncated_result.diagnostics[0].severity == ParseSeverity::error);
    assert(truncated_result.diagnostics[0].code == "hits.truncated_record");

    return 0;
}
