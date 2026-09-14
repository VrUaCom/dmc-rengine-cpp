#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

/// A run of fixed-width, NUL-padded name fields in read-only data.
///
/// Resource names are frequently stored this way rather than as individually
/// referenced constants: the array is indexed, so only the code that walks it
/// refers to the names at all. Recovering the stride and extent is what turns
/// an opaque blob of text into an addressable table.
struct StringTableRun final {
    std::uint32_t base_rva{};
    /// Distance between consecutive name fields.
    std::uint32_t stride{};
    std::uint32_t entries{};
    std::uint32_t longest_name{};

    /// Elements whose bytes after the terminator are not all zero.
    std::uint32_t records_with_payload{};
    /// Of those, elements whose payload begins another printable terminated
    /// string. Text payload usually means one of two very different things, and
    /// the two measurements below are what separate them.
    std::uint32_t text_payload_records{};
    /// Offset of the first payload byte in the first element that has one.
    std::uint32_t first_payload_offset{};
    /// True when every payload-bearing element starts its payload at the same
    /// offset. Consistent text payload is a record with several name fields;
    /// inconsistent text payload is an alignment-padded string pool that merely
    /// happens to fall on a regular spacing.
    bool payload_offset_consistent{true};

    /// First name in the run, as a sample of what the table holds.
    std::string first_name;

    /// Smallest period at which the elements' extensions repeat, or zero when
    /// no period up to the search limit fits. A period above one is how a
    /// multi-field record shows itself to a scan that only sees the innermost
    /// spacing: a 9-element period of one archive name followed by eight text
    /// names is a record with nine name fields, read one field at a time.
    std::uint32_t content_period{};
    /// Extension class at each position of one period, lower-cased, empty where
    /// an element's name carries no extension.
    std::vector<std::string> period_extensions;

    [[nodiscard]] bool pure_name_array() const noexcept {
        return records_with_payload == 0U;
    }

    /// Every element carries payload at the same offset: the shape of a record
    /// array rather than of a pool that happens to line up.
    [[nodiscard]] bool uniform_records() const noexcept {
        return records_with_payload == entries && entries != 0U && payload_offset_consistent;
    }

    [[nodiscard]] std::uint64_t span_bytes() const noexcept {
        return static_cast<std::uint64_t>(stride) * entries;
    }

    friend bool operator==(const StringTableRun&, const StringTableRun&) = default;
};

/// A run of records whose name fields repeat with a fixed *pattern* of widths.
///
/// A constant-stride scan cannot see these: the cutscene localisation record in
/// the canonical DMC3 image has a 32-byte leading field followed by eight
/// 40-byte fields, so its gap sequence is 32 then eight 40s, and any
/// single-stride hypothesis breaks at every record boundary. Looking for a
/// period in the gap sequence instead finds the record whole.
struct NameRecordRun final {
    std::uint32_t base_rva{};
    /// Sum of one period's field widths: the record stride.
    std::uint32_t record_bytes{};
    std::uint32_t fields_per_record{};
    std::uint32_t records{};

    /// Field offsets within the record, starting at zero.
    std::vector<std::uint32_t> field_offsets;
    /// Field widths, in the same order.
    std::vector<std::uint32_t> field_widths;
    /// Lower-cased extension observed in each field position, empty where a
    /// field's names carry none.
    std::vector<std::string> field_extensions;

    /// First name of the first record, as a sample of what the table holds.
    std::string first_name;

    [[nodiscard]] std::uint64_t span_bytes() const noexcept {
        return static_cast<std::uint64_t>(record_bytes) * records;
    }

    friend bool operator==(const NameRecordRun&, const NameRecordRun&) = default;
};

struct StringTableScanOptions final {
    /// A run shorter than this is noise rather than a table.
    std::uint32_t minimum_entries{8U};
    std::uint32_t minimum_name_length{4U};
    std::uint32_t maximum_stride{4096U};
    /// Largest content period to look for. A run must hold at least two full
    /// periods for one to be reported.
    std::uint32_t maximum_content_period{32U};
    /// Largest number of name fields a record may have.
    std::uint32_t maximum_fields_per_record{32U};
    /// A record run must hold at least this many records.
    std::uint32_t minimum_records{4U};
};

struct StringTableScanResult final {
    /// Runs ordered by entry count, longest first.
    std::vector<StringTableRun> runs;
    /// Multi-field record runs, ordered by record count, longest first. Only
    /// patterns of two or more fields appear here; a single-field pattern is a
    /// constant-stride run and is reported above.
    std::vector<NameRecordRun> records;
    std::size_t candidate_names{};
    std::size_t entries_in_runs{};
    std::vector<std::string> warnings;
};

/// Finds fixed-width name arrays in an image's read-only data.
///
/// A run is accepted only when every element holds a printable, NUL-terminated
/// name that fits inside the stride. That is a strong constraint: unrelated
/// text at a coincidental spacing fails it within a couple of elements.
class StringTableScanner final {
public:
    [[nodiscard]] static StringTableScanResult scan(std::span<const std::byte> bytes,
                                                    const PeImage& image,
                                                    const StringTableScanOptions& options = {});
};

} // namespace dmc::rengine::exe
