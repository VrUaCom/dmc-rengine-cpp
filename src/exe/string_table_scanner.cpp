#include "dmc_rengine/exe/string_table_scanner.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
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
    /// Offset within the element of the first non-zero byte after the
    /// terminator, valid only when payload is present.
    std::uint32_t payload_offset{};
    /// The payload begins a printable, terminated string.
    bool payload_is_text{false};
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
        if (std::to_integer<unsigned char>(bytes[offset + index]) == 0U) {
            continue;
        }

        element.payload_after_terminator = true;
        element.payload_offset = index;

        // Does the payload begin another name? A packed pool and a multi-field
        // record both look like text here; the caller separates them by whether
        // the offset is the same in every element.
        std::uint32_t run = 0U;
        while (index + run < stride &&
               printable(std::to_integer<unsigned char>(bytes[offset + index + run]))) {
            ++run;
        }
        element.payload_is_text = run >= minimum_name_length && index + run < stride &&
            std::to_integer<unsigned char>(bytes[offset + index + run]) == 0U;
        break;
    }

    element.name_length = length;
    element.valid = true;
    return element;
}

/// Smallest period at which the extension sequence repeats.
///
/// A record holding several name fields is invisible to a scan that sees only
/// the innermost spacing, but its field *kinds* repeat, and that is measurable
/// without interpreting anything.
void detect_content_period(const std::vector<std::string>& extensions, std::uint32_t limit,
                           StringTableRun& run) {
    if (extensions.size() < 2U) {
        return;
    }

    const auto maximum = std::min<std::size_t>(limit, extensions.size() / 2U);
    for (std::size_t period = 1U; period <= maximum; ++period) {
        bool fits = true;
        for (std::size_t index = period; index < extensions.size(); ++index) {
            if (extensions[index] != extensions[index % period]) {
                fits = false;
                break;
            }
        }
        if (!fits) {
            continue;
        }

        run.content_period = static_cast<std::uint32_t>(period);
        run.period_extensions.assign(extensions.begin(),
                                     extensions.begin() + static_cast<std::ptrdiff_t>(period));
        return;
    }
}

/// Lower-cased extension of a name, empty when it has none.
[[nodiscard]] std::string extension_of(std::string_view name) {
    const auto dot = name.rfind('.');
    if (dot == std::string_view::npos || dot + 1U >= name.size()) {
        return {};
    }

    const auto tail = name.substr(dot + 1U);
    if (tail.size() > 5U) {
        return {};
    }

    std::string lowered{tail};
    for (auto& character : lowered) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return lowered;
}

/// Is the name at `offset` terminated within `width` and long enough to count?
[[nodiscard]] bool field_holds_name(std::span<const std::byte> bytes, std::size_t offset,
                                    std::uint32_t width, std::uint32_t minimum_name_length) {
    if (width < 2U || !detail::has_range(bytes, offset, width)) {
        return false;
    }

    std::uint32_t length = 0U;
    while (length < width) {
        const auto value = std::to_integer<unsigned char>(bytes[offset + length]);
        if (value == 0U) {
            break;
        }
        if (!printable(value)) {
            return false;
        }
        ++length;
    }
    return length >= minimum_name_length && length < width;
}

/// Finds records whose name-field widths repeat with a fixed pattern.
///
/// The gap sequence between consecutive names is the signal. A uniform pattern
/// is a constant-stride run and is left to the other scan; a pattern with
/// differing widths is a record this one can see and that one cannot.
void detect_record_runs(std::span<const std::byte> bytes, const PeSection& section,
                        std::size_t section_begin, const std::vector<std::size_t>& starts,
                        const StringTableScanOptions& options, StringTableScanResult& result) {
    if (starts.size() < 4U) {
        return;
    }

    std::vector<std::size_t> gaps(starts.size() - 1U);
    for (std::size_t index = 0; index + 1U < starts.size(); ++index) {
        gaps[index] = starts[index + 1U] - starts[index];
    }

    std::size_t position = 0U;
    while (position + 1U < gaps.size()) {
        std::uint32_t chosen_period = 0U;
        std::size_t chosen_fields = 0U;

        const auto limit = std::min<std::uint32_t>(options.maximum_fields_per_record,
                                                   static_cast<std::uint32_t>(gaps.size()));
        for (std::uint32_t period = 2U; period <= limit; ++period) {
            if (position + period > gaps.size()) {
                break;
            }

            // A pattern whose widths are all equal is a constant-stride run.
            bool uniform = true;
            for (std::uint32_t index = 1U; index < period; ++index) {
                if (gaps[position + index] != gaps[position]) {
                    uniform = false;
                    break;
                }
            }
            if (uniform) {
                continue;
            }

            std::size_t matched = 0U;
            while (position + matched < gaps.size() &&
                   gaps[position + matched] == gaps[position + matched % period] &&
                   field_holds_name(bytes, section_begin + starts[position + matched],
                                    static_cast<std::uint32_t>(gaps[position + matched]),
                                    options.minimum_name_length)) {
                ++matched;
            }

            const std::size_t fields = matched + 1U;
            if (fields / period < options.minimum_records) {
                continue;
            }
            if (fields > chosen_fields) {
                chosen_period = period;
                chosen_fields = fields;
            }
        }

        if (chosen_period == 0U) {
            ++position;
            continue;
        }

        // The search maximises coverage, which can settle on a multiple of the
        // real period: a four-field record repeated four times matches a
        // sixteen-field pattern just as well. Reduce to the smallest period the
        // widths and extensions both repeat at. Any divisor that fits here also
        // fits the whole run, because the run matched modulo the larger period.
        {
            std::vector<std::uint32_t> widths(chosen_period);
            std::vector<std::string> kinds(chosen_period);
            for (std::uint32_t field = 0U; field < chosen_period; ++field) {
                const auto width = static_cast<std::uint32_t>(gaps[position + field]);
                widths[field] = width;
                const auto name = detail::read_cstring(
                    bytes, section_begin + starts[position + field], width);
                kinds[field] = extension_of(name.value_or(std::string{}));
            }

            for (std::uint32_t candidate = 1U; candidate < chosen_period; ++candidate) {
                if (chosen_period % candidate != 0U) {
                    continue;
                }

                bool repeats = true;
                for (std::uint32_t field = candidate; field < chosen_period; ++field) {
                    if (widths[field] != widths[field % candidate] ||
                        kinds[field] != kinds[field % candidate]) {
                        repeats = false;
                        break;
                    }
                }
                if (repeats) {
                    chosen_period = candidate;
                    break;
                }
            }
        }

        // A reduced period of one means uniform fields: that is a
        // constant-stride run, reported by the other scan.
        if (chosen_period < 2U) {
            position += chosen_fields;
            continue;
        }

        const auto records = static_cast<std::uint32_t>(chosen_fields / chosen_period);
        if (records < options.minimum_records) {
            ++position;
            continue;
        }
        NameRecordRun run;
        run.base_rva = static_cast<std::uint32_t>(section.virtual_address + starts[position]);
        run.fields_per_record = chosen_period;
        run.records = records;

        std::uint32_t offset_in_record = 0U;
        for (std::uint32_t field = 0U; field < chosen_period; ++field) {
            const auto width = static_cast<std::uint32_t>(gaps[position + field]);
            run.field_offsets.push_back(offset_in_record);
            run.field_widths.push_back(width);
            offset_in_record += width;

            const auto name = detail::read_cstring(
                bytes, section_begin + starts[position + field], width);
            run.field_extensions.push_back(extension_of(name.value_or(std::string{})));
            if (field == 0U) {
                run.first_name = name.value_or(std::string{});
            }
        }
        run.record_bytes = offset_in_record;

        result.records.push_back(std::move(run));
        position += records * chosen_period;
    }
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

        detect_record_runs(bytes, section, begin, starts, options, result);

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

            std::vector<std::string> extensions;
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

                const auto name = detail::read_cstring(bytes, offset, stride);
                if (entries == 0U) {
                    run.first_name = name.value_or(std::string{});
                }
                extensions.push_back(extension_of(name.value_or(std::string{})));
                run.longest_name = std::max(run.longest_name, element.name_length);
                if (element.payload_after_terminator) {
                    if (run.records_with_payload == 0U) {
                        run.first_payload_offset = element.payload_offset;
                    } else if (element.payload_offset != run.first_payload_offset) {
                        run.payload_offset_consistent = false;
                    }
                    ++run.records_with_payload;
                    if (element.payload_is_text) {
                        ++run.text_payload_records;
                    }
                }
                ++entries;
            }

            if (entries < options.minimum_entries) {
                ++position;
                continue;
            }

            run.entries = static_cast<std::uint32_t>(entries);
            extensions.resize(entries);
            detect_content_period(extensions, options.maximum_content_period, run);
            result.entries_in_runs += entries;
            result.runs.push_back(std::move(run));

            // Skip the candidates this run consumed.
            const auto run_end = starts[position] + entries * stride;
            while (position < starts.size() && starts[position] < run_end) {
                ++position;
            }
        }
    }

    std::sort(result.records.begin(), result.records.end(),
              [](const NameRecordRun& left, const NameRecordRun& right) {
                  if (left.records != right.records) {
                      return left.records > right.records;
                  }
                  return left.base_rva < right.base_rva;
              });

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
