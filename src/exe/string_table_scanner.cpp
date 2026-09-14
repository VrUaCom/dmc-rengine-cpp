#include "dmc_rengine/exe/string_table_scanner.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <cstdint>

namespace dmc::rengine::exe {
namespace {

constexpr std::size_t kMaxRunEntries = 1U << 20U;

[[nodiscard]] bool is_read_only_data(const PeSection& section) noexcept {
    constexpr std::uint32_t initialized_data = 0x00000040U;
    constexpr std::uint32_t executable = 0x20000000U;
    constexpr std::uint32_t writable = 0x80000000U;
    return section.raw_size > 0U && (section.characteristics & initialized_data) != 0U &&
           (section.characteristics & executable) == 0U &&
           (section.characteristics & writable) == 0U;
}

[[nodiscard]] std::size_t readable_size(std::span<const std::byte> bytes,
                                        const PeSection& section) noexcept {
    const auto begin = static_cast<std::size_t>(section.raw_offset);
    if (begin >= bytes.size()) {
        return 0U;
    }
    return std::min(bytes.size() - begin, static_cast<std::size_t>(section.raw_size));
}

[[nodiscard]] bool printable(unsigned char value) noexcept {
    return value >= 0x20U && value <= 0x7EU;
}

/// One element of a candidate run: a printable name terminated inside `stride`.
struct Element final {
    std::uint32_t name_length{};
    bool payload_after_terminator{false};
    bool valid{false};
};

/// Reads one element, requiring it to begin where a string begins.
///
/// Without the boundary check a stride can lock onto a phantom grid that slices
/// through real names: landing four bytes into "wandering.pac" yields the
/// perfectly printable, perfectly terminated "ering.pac" and the run marches on
/// over data that is not a table at all.
[[nodiscard]] Element read_element(std::span<const std::byte> bytes, std::size_t offset,
                                   std::uint32_t stride, std::uint32_t minimum_name_length,
                                   std::size_t section_begin) {
    Element element;
    if (!detail::has_range(bytes, offset, stride)) {
        return element;
    }

    if (offset > section_begin &&
        std::to_integer<unsigned char>(bytes[offset - 1U]) != 0U) {
        return element;
    }

    std::uint32_t length = 0U;
    while (length < stride) {
        const auto value = std::to_integer<unsigned char>(bytes[offset + length]);
        if (value == 0U) {
            break;
        }
        if (!printable(value)) {
            return element;
        }
        ++length;
    }

    // The terminator must fall inside the stride, so a name exactly filling the
    // field is rejected: nothing would separate it from the next element.
    if (length >= stride || length < minimum_name_length) {
        return element;
    }

    for (std::uint32_t index = length; index < stride; ++index) {
        if (std::to_integer<unsigned char>(bytes[offset + index]) != 0U) {
            element.payload_after_terminator = true;
            break;
        }
    }

    element.name_length = length;
    element.valid = true;
    return element;
}

} // namespace

StringTableScanResult StringTableScanner::scan(std::span<const std::byte> bytes,
                                               const PeImage& image,
                                               const StringTableScanOptions& options) {
    StringTableScanResult result;
    if (options.minimum_entries < 2U || options.maximum_stride < 2U) {
        result.warnings.emplace_back("String table scan options exclude every possible run.");
        return result;
    }

    for (const auto& section : image.sections) {
        if (!is_read_only_data(section)) {
            continue;
        }

        const auto begin = static_cast<std::size_t>(section.raw_offset);
        const auto size = readable_size(bytes, section);

        // Candidate name starts: a printable run that terminates and is long
        // enough to be a name.
        std::vector<std::size_t> starts;
        std::size_t index = 0U;
        while (index < size) {
            if (!printable(std::to_integer<unsigned char>(bytes[begin + index]))) {
                ++index;
                continue;
            }

            std::size_t length = 0U;
            while (index + length < size &&
                   printable(std::to_integer<unsigned char>(bytes[begin + index + length]))) {
                ++length;
            }

            const bool terminated = index + length < size &&
                std::to_integer<unsigned char>(bytes[begin + index + length]) == 0U;
            if (terminated && length >= options.minimum_name_length) {
                starts.push_back(index);
            }
            index += length + 1U;
        }
        result.candidate_names += starts.size();

        // Walk the candidates in order, taking the distance to the next as the
        // stride hypothesis and extending while every element validates.
        std::size_t position = 0U;
        while (position + 1U < starts.size()) {
            const auto stride64 = starts[position + 1U] - starts[position];
            if (stride64 < 2U || stride64 > options.maximum_stride) {
                ++position;
                continue;
            }

            const auto stride = static_cast<std::uint32_t>(stride64);
            StringTableRun run;
            run.base_rva = static_cast<std::uint32_t>(section.virtual_address + starts[position]);
            run.stride = stride;

            std::size_t entries = 0U;
            while (entries < kMaxRunEntries) {
                const auto offset = begin + starts[position] + entries * stride;
                if (offset + stride > begin + size) {
                    break;
                }

                const auto element =
                    read_element(bytes, offset, stride, options.minimum_name_length, begin);
                if (!element.valid) {
                    break;
                }

                if (entries == 0U) {
                    const auto name = detail::read_cstring(bytes, offset, stride);
                    run.first_name = name.value_or(std::string{});
                }
                run.longest_name = std::max(run.longest_name, element.name_length);
                if (element.payload_after_terminator) {
                    ++run.records_with_payload;
                }
                ++entries;
            }

            if (entries < options.minimum_entries) {
                ++position;
                continue;
            }

            run.entries = static_cast<std::uint32_t>(entries);
            result.entries_in_runs += entries;
            result.runs.push_back(std::move(run));

            // Skip the candidates this run consumed.
            const auto run_end = starts[position] + entries * stride;
            while (position < starts.size() && starts[position] < run_end) {
                ++position;
            }
        }
    }

    std::sort(result.runs.begin(), result.runs.end(),
              [](const StringTableRun& left, const StringTableRun& right) {
                  if (left.entries != right.entries) {
                      return left.entries > right.entries;
                  }
                  return left.base_rva < right.base_rva;
              });
    return result;
}

} // namespace dmc::rengine::exe
