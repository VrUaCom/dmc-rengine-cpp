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

    /// Records whose bytes after the terminator are not all zero. Zero means a
    /// pure name array; a nonzero count means each element is a record with a
    /// leading name field and further payload.
    std::uint32_t records_with_payload{};

    /// First name in the run, as a sample of what the table holds.
    std::string first_name;

    [[nodiscard]] bool pure_name_array() const noexcept {
        return records_with_payload == 0U;
    }

    [[nodiscard]] std::uint64_t span_bytes() const noexcept {
        return static_cast<std::uint64_t>(stride) * entries;
    }

    friend bool operator==(const StringTableRun&, const StringTableRun&) = default;
};

struct StringTableScanOptions final {
    /// A run shorter than this is noise rather than a table.
    std::uint32_t minimum_entries{8U};
    std::uint32_t minimum_name_length{4U};
    std::uint32_t maximum_stride{4096U};
};

struct StringTableScanResult final {
    /// Runs ordered by entry count, longest first.
    std::vector<StringTableRun> runs;
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
