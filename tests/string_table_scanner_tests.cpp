#include "dmc_rengine/exe/string_table_scanner.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::exe::PeImage;
using dmc::rengine::exe::PeKind;
using dmc::rengine::exe::PeSection;
using dmc::rengine::exe::StringTableScanner;
using dmc::rengine::exe::StringTableScanOptions;

// Synthetic image with one read-only data section at RVA 0x2000 (file 0x400)
// and one writable section, so the scanner's section filter is exercised.

void put_text(std::vector<std::byte>& bytes, std::size_t offset, std::string_view text) {
    for (std::size_t index = 0; index < text.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(text[index]);
    }
}

[[nodiscard]] PeImage make_image() {
    PeImage image;
    image.kind = PeKind::pe32_plus;
    image.image_base = 0x140000000ULL;
    image.size_of_image = 0x4000U;
    image.section_count = 2U;
    image.sections.push_back(PeSection{".rdata", 0x400U, 0x2000U, 0x400U, 0x400U, 0x40000040U});
    // Writable data must be skipped: a table there is not read-only content.
    image.sections.push_back(PeSection{".data", 0x200U, 0x3000U, 0x200U, 0x800U, 0xC0000040U});
    return image;
}

/// Twelve names at a 16-byte stride, NUL-padded.
[[nodiscard]] std::vector<std::byte> make_pure_array() {
    std::vector<std::byte> bytes(0xA00U, std::byte{0});
    for (std::size_t index = 0; index < 12U; ++index) {
        put_text(bytes, 0x400U + index * 16U, "name" + std::to_string(index) + ".pac");
    }
    return bytes;
}

void a_fixed_width_array_is_recovered() {
    const auto image = make_image();
    const auto bytes = make_pure_array();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.runs.size() == 1U);

    const auto& run = result.runs[0];
    assert(run.base_rva == 0x2000U);
    assert(run.stride == 16U);
    assert(run.entries == 12U);
    // "name0.pac" is 9 characters; "name10.pac" and "name11.pac" are 10.
    assert(run.longest_name == 10U);
    assert(run.pure_name_array());
    assert(run.first_name == "name0.pac");
    assert(run.span_bytes() == 192U);
    assert(result.entries_in_runs == 12U);
}

void records_with_payload_are_distinguished_from_pure_arrays() {
    auto bytes = make_pure_array();

    // Give every element a non-zero byte after its terminator: now each element
    // is a record with a leading name, not a bare string.
    for (std::size_t index = 0; index < 12U; ++index) {
        bytes[0x400U + index * 16U + 14U] = std::byte{0x7F};
    }

    const auto image = make_image();
    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.runs.size() == 1U);
    assert(!result.runs[0].pure_name_array());
    assert(result.runs[0].records_with_payload == 12U);
}

void a_run_shorter_than_the_minimum_is_not_a_table() {
    std::vector<std::byte> bytes(0xA00U, std::byte{0});
    for (std::size_t index = 0; index < 4U; ++index) {
        put_text(bytes, 0x400U + index * 16U, "short.pac");
    }

    const auto image = make_image();
    assert(StringTableScanner::scan(std::span<const std::byte>{bytes}, image).runs.empty());

    // Lowering the threshold accepts it.
    StringTableScanOptions relaxed;
    relaxed.minimum_entries = 4U;
    const auto accepted =
        StringTableScanner::scan(std::span<const std::byte>{bytes}, image, relaxed);
    assert(accepted.runs.size() == 1U);
    assert(accepted.runs[0].entries == 4U);
}

void irregular_spacing_is_rejected() {
    std::vector<std::byte> bytes(0xA00U, std::byte{0});
    // Same names, but the spacing wanders, so no single stride validates. The
    // gaps all exceed the 14 bytes a terminated "wandering.pac" needs, so the
    // names stay separate candidates rather than merging into one run.
    const std::size_t offsets[]{0x400U, 0x410U, 0x424U, 0x434U, 0x44CU,
                                0x45CU, 0x470U, 0x480U, 0x498U, 0x4A8U};
    for (const auto offset : offsets) {
        put_text(bytes, offset, "wandering.pac");
    }

    const auto image = make_image();
    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.candidate_names == 10U);
    assert(result.runs.empty());
}

void a_name_filling_the_stride_breaks_the_run() {
    auto bytes = make_pure_array();

    // Overwrite one element so its name occupies all 16 bytes: nothing would
    // separate it from the next element, so the run must stop there.
    put_text(bytes, 0x400U + 5U * 16U, "0123456789abcdef");

    const auto image = make_image();

    // The break leaves only five elements before it, so the run is only visible
    // with a threshold below the default.
    StringTableScanOptions relaxed;
    relaxed.minimum_entries = 4U;
    const auto result =
        StringTableScanner::scan(std::span<const std::byte>{bytes}, image, relaxed);
    assert(!result.runs.empty());
    assert(result.runs[0].base_rva == 0x2000U);
    assert(result.runs[0].entries == 5U);

    // At the default threshold neither the head nor the tail qualifies.
    assert(StringTableScanner::scan(std::span<const std::byte>{bytes}, image).runs.empty());
}

void writable_sections_are_skipped() {
    std::vector<std::byte> bytes(0xA00U, std::byte{0});
    // A perfectly good table, but in the writable section at file 0x800.
    for (std::size_t index = 0; index < 12U; ++index) {
        put_text(bytes, 0x800U + index * 16U, "ignored.pac");
    }

    const auto image = make_image();
    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.runs.empty());
    assert(result.candidate_names == 0U);
}

/// Writes `records` records whose field widths follow `widths`.
[[nodiscard]] std::vector<std::byte> make_record_table(
    const std::vector<std::size_t>& widths, std::size_t records,
    const std::vector<std::string>& extensions) {
    std::vector<std::byte> bytes(0xA00U, std::byte{0});
    std::size_t offset = 0x400U;
    for (std::size_t record = 0; record < records; ++record) {
        for (std::size_t field = 0; field < widths.size(); ++field) {
            put_text(bytes, offset, "f" + std::to_string(field) + "r" + std::to_string(record) +
                                        extensions[field]);
            offset += widths[field];
        }
    }
    return bytes;
}

void a_record_with_differing_field_widths_is_recovered() {
    const std::vector<std::size_t> widths{16U, 24U};
    const auto bytes = make_record_table(widths, 6U, {".pac", ".txt"});
    const auto image = make_image();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.records.size() == 1U);

    const auto& run = result.records[0];
    assert(run.base_rva == 0x2000U);
    assert(run.fields_per_record == 2U);
    assert(run.records == 6U);
    assert(run.record_bytes == 40U);
    assert(run.field_offsets[0] == 0U);
    assert(run.field_offsets[1] == 16U);
    assert(run.field_widths[0] == 16U);
    assert(run.field_widths[1] == 24U);
    assert(run.field_extensions[0] == "pac");
    assert(run.field_extensions[1] == "txt");
    assert(run.span_bytes() == 240U);
}

void a_multiple_of_the_real_period_is_reduced() {
    // Two records' worth of pattern written as one: the search may settle on
    // four fields, and the reduction must bring it back to two.
    const std::vector<std::size_t> widths{16U, 24U, 16U, 24U};
    const auto bytes = make_record_table(widths, 6U, {".pac", ".txt", ".pac", ".txt"});
    const auto image = make_image();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.records.size() == 1U);
    assert(result.records[0].fields_per_record == 2U);
    assert(result.records[0].record_bytes == 40U);
    assert(result.records[0].records == 12U);
}

void uniform_field_widths_are_left_to_the_stride_scan() {
    // Every field the same width is a constant-stride run, not a record.
    const std::vector<std::size_t> widths{16U, 16U};
    const auto bytes = make_record_table(widths, 8U, {".pac", ".pac"});
    const auto image = make_image();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.records.empty());
    assert(!result.runs.empty());
    assert(result.runs[0].stride == 16U);
}

void too_few_records_is_not_a_table() {
    const std::vector<std::size_t> widths{16U, 24U};
    const auto bytes = make_record_table(widths, 3U, {".pac", ".txt"});
    const auto image = make_image();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.records.empty());
}

void the_content_period_of_a_uniform_run_is_one() {
    const auto image = make_image();
    const auto bytes = make_pure_array();

    const auto result = StringTableScanner::scan(std::span<const std::byte>{bytes}, image);
    assert(result.runs.size() == 1U);
    // Every element ends in .pac, so the extension sequence is uniform.
    assert(result.runs[0].content_period == 1U);
    assert(result.runs[0].period_extensions.size() == 1U);
    assert(result.runs[0].period_extensions[0] == "pac");
}

void degenerate_options_are_refused() {
    const auto image = make_image();
    const auto bytes = make_pure_array();

    StringTableScanOptions impossible;
    impossible.minimum_entries = 1U;
    const auto result =
        StringTableScanner::scan(std::span<const std::byte>{bytes}, image, impossible);
    assert(result.runs.empty());
    assert(!result.warnings.empty());
}

} // namespace

int main() {
    a_fixed_width_array_is_recovered();
    records_with_payload_are_distinguished_from_pure_arrays();
    a_run_shorter_than_the_minimum_is_not_a_table();
    irregular_spacing_is_rejected();
    a_name_filling_the_stride_breaks_the_run();
    writable_sections_are_skipped();
    a_record_with_differing_field_widths_is_recovered();
    a_multiple_of_the_real_period_is_reduced();
    uniform_field_widths_are_left_to_the_stride_scan();
    too_few_records_is_not_a_table();
    the_content_period_of_a_uniform_run_is_one();
    degenerate_options_are_refused();
    return 0;
}
