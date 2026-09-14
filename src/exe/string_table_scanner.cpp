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
        //
        // The name need not terminate inside the element. Requiring that hid
        // the plainest pool there is: one whose next string runs straight
        // through the element boundary, which a field of a record never does.
        std::uint32_t run = 0U;
        while (index + run < stride &&
               printable(std::to_integer<unsigned char>(bytes[offset + index + run]))) {
            ++run;
        }
        const bool reaches_boundary = index + run == stride;
        element.payload_is_text =
            run >= minimum_name_length &&
            (reaches_boundary ||
             std::to_integer<unsigned char>(bytes[offset + index + run]) == 0U);
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

/// Re-derives a run's aggregate facts over its first `entries` elements.
void summarise(std::span<const std::byte> bytes, std::size_t offset, StringTableRun& run,
               std::uint32_t entries, std::uint32_t minimum_name_length,
               std::size_t section_begin) {
    run.longest_name = 0U;
    run.records_with_payload = 0U;
    run.text_payload_records = 0U;
    run.first_payload_offset = 0U;
    run.payload_offset_consistent = true;

    for (std::uint32_t index = 0U; index < entries; ++index) {
        const auto element = read_element(bytes, offset + index * run.stride, run.stride,
                                          minimum_name_length, section_begin);
        if (!element.valid) {
            break;
        }
        run.longest_name = std::max(run.longest_name, element.name_length);
        if (!element.payload_after_terminator) {
            continue;
        }
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
}

/// Settles the two scans against each other where they describe the same bytes.
///
/// A record whose fields are all one width contains a constant-stride grid, so
/// the stride scan reports the record's interior as a table of its own. That is
/// not a contradiction and the run is kept, but it is marked, because counting
/// it as an independent table counts the same bytes twice.
///
/// Where the two disagree is the run's extent. A grid has nothing to stop it at
/// a field of a different width whose name is short enough to terminate inside
/// the grid's stride, so it runs on into the next field, the next record, or
/// out of the record array altogether. The record's field widths repeat with a
/// measured period and are the stronger reading, so the run is cut back to the
/// block of equal-width fields it started in.
void reconcile_record_interiors(std::span<const std::byte> bytes, const PeSection& section,
                                std::size_t section_begin,
                                const StringTableScanOptions& options,
                                StringTableScanResult& result, std::size_t first_run) {
    for (std::size_t index = first_run; index < result.runs.size(); ++index) {
        auto& run = result.runs[index];

        for (const auto& record : result.records) {
            if (record.record_bytes == 0U || run.base_rva < record.base_rva) {
                continue;
            }
            const auto delta = static_cast<std::uint64_t>(run.base_rva) - record.base_rva;
            if (delta >= record.span_bytes()) {
                continue;
            }

            const auto offset_in_record = static_cast<std::uint32_t>(delta % record.record_bytes);
            std::size_t field = record.field_offsets.size();
            for (std::size_t candidate = 0; candidate < record.field_offsets.size(); ++candidate) {
                if (record.field_offsets[candidate] == offset_in_record &&
                    record.field_widths[candidate] == run.stride) {
                    field = candidate;
                    break;
                }
            }
            if (field == record.field_offsets.size()) {
                continue;
            }

            std::uint32_t block = 0U;
            while (field + block < record.field_widths.size() &&
                   record.field_widths[field + block] == run.stride) {
                ++block;
            }

            run.interior_of_record_rva = record.base_rva;
            run.interior_field_index = static_cast<std::uint32_t>(field);
            if (run.entries > block) {
                run.overrun_elements = run.entries - block;
                result.entries_in_runs -= run.overrun_elements;
                run.entries = block;
                const auto offset = section_begin + (run.base_rva - section.virtual_address);
                summarise(bytes, offset, run, run.entries, options.minimum_name_length,
                          section_begin);
            }
            break;
        }
    }

    // Nothing is erased here. The minimum entry count exists to keep a stride
    // *hypothesis* from being reported on the strength of a few coincidences; a
    // block cut to the width of a record's field pattern is not a hypothesis but
    // a reading two scans agree on, and a seven-field block is a seven-field
    // block whatever the minimum says.
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
        const auto first_run_of_section = result.runs.size();
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
            std::vector<Element> elements;
            while (elements.size() < kMaxRunEntries) {
                const auto offset = begin + starts[position] + elements.size() * stride;
                if (offset + stride > begin + size) {
                    break;
                }

                const auto element =
                    read_element(bytes, offset, stride, options.minimum_name_length, begin);
                if (!element.valid) {
                    break;
                }

                const auto name = detail::read_cstring(bytes, offset, stride);
                if (elements.empty()) {
                    run.first_name = name.value_or(std::string{});
                }
                extensions.push_back(extension_of(name.value_or(std::string{})));
                elements.push_back(element);
            }

            // A grid can run off the end of its table into the packed string
            // pool that follows it, because a pool of short names is padded to
            // the same alignment the grid uses and so keeps validating. What
            // gives it away is where the payload sits: table elements are name
            // plus NUL padding, so payload appears only once the grid has left
            // the table, as an unbroken run of elements at the end.
            //
            // Trimming needs that run to be a strict suffix. Payload from the
            // first element onwards is a record array whose second field this
            // scan cannot see, and trimming it would delete a real table.
            //
            // What is left after trimming is not second-guessed here. A head
            // too short to be a table fails the minimum below and the run goes
            // entirely, which is the right answer: three names and a pool is
            // evidence of a pool, not of a three-element table.
            auto entries = elements.size();
            {
                std::size_t first_payload = entries;
                std::size_t payload_count = 0U;
                std::size_t text_count = 0U;
                for (std::size_t index = 0U; index < entries; ++index) {
                    if (!elements[index].payload_after_terminator) {
                        continue;
                    }
                    first_payload = std::min(first_payload, index);
                    ++payload_count;
                    text_count += elements[index].payload_is_text ? 1U : 0U;
                }

                const bool suffix = payload_count != 0U && first_payload + payload_count == entries;
                if (suffix && text_count != 0U && first_payload != 0U) {
                    run.absorbed_elements = static_cast<std::uint32_t>(entries - first_payload);
                    entries = first_payload;
                }
            }

            if (entries < options.minimum_entries) {
                ++position;
                continue;
            }

            elements.resize(entries);
            extensions.resize(entries);
            for (const auto& element : elements) {
                run.longest_name = std::max(run.longest_name, element.name_length);
                if (!element.payload_after_terminator) {
                    continue;
                }
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

            // Reject a grid laid over a packed string pool. In a pool the
            // bytes after a name are further names, and each element's payload
            // starts wherever the previous name happened to end. A genuine
            // field is the name plus NUL padding; a genuine record puts its
            // next field at the same offset every time.
            //
            // This check matters most at large strides, where "a name
            // terminated inside the field" is satisfied by almost any dense
            // text and validates nothing on its own.
            if (run.text_payload_records != 0U && !run.payload_offset_consistent) {
                ++position;
                continue;
            }

            run.entries = static_cast<std::uint32_t>(entries);
            detect_content_period(extensions, options.maximum_content_period, run);
            result.entries_in_runs += entries;
            result.runs.push_back(std::move(run));

            // Skip the candidates this run consumed. Absorbed elements are not
            // consumed: the pool they came from is left for the scan to read on
            // its own terms.
            const auto run_end = starts[position] + entries * stride;
            while (position < starts.size() && starts[position] < run_end) {
                ++position;
            }
        }

        reconcile_record_interiors(bytes, section, begin, options, result, first_run_of_section);
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
