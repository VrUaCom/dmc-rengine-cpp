#include "dmc_rengine/exe/byte_window.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] ExeByteWindowResult fail(
    ExeByteWindowError error,
    std::string message) {
    return ExeByteWindowResult{
        .window = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] bool file_range_fits(
    std::span<const std::byte> file_bytes,
    std::uint32_t file_offset,
    std::size_t size) noexcept {
    const auto offset = static_cast<std::size_t>(file_offset);
    return offset <= file_bytes.size() && size <= file_bytes.size() - offset;
}

[[nodiscard]] std::uint32_t file_backed_virtual_extent(
    const PeSection& section) noexcept {
    const auto virtual_extent =
        section.virtual_size == 0U ? section.raw_size : section.virtual_size;
    return std::min(section.raw_size, virtual_extent);
}

[[nodiscard]] bool section_maps_file_backed_va(
    const PeSection& section,
    std::uint32_t rva) noexcept {
    if (rva < section.virtual_address) {
        return false;
    }
    const auto delta = rva - section.virtual_address;
    return delta < file_backed_virtual_extent(section);
}

[[nodiscard]] bool file_backed_rva_range_overlaps(
    const PeSection& section,
    std::uint64_t begin,
    std::uint64_t end) noexcept {
    const auto extent = file_backed_virtual_extent(section);
    if (extent == 0U) {
        return false;
    }
    const auto section_begin = static_cast<std::uint64_t>(section.virtual_address);
    const auto section_end = section_begin + static_cast<std::uint64_t>(extent);
    return begin < section_end && section_begin < end;
}

} // namespace

ExeByteWindowResult ExeByteWindowExtractor::extract(
    std::span<const std::byte> file_bytes,
    const PeImage& image,
    std::uint64_t va,
    std::size_t size) {
    if (size == 0U) {
        return fail(ExeByteWindowError::zero_size, "requested byte window is empty");
    }
    if (size > k_max_exe_byte_window_size) {
        return fail(
            ExeByteWindowError::size_limit_exceeded,
            "requested byte window exceeds the acquisition safety limit");
    }
    if (va < image.image_base) {
        return fail(
            ExeByteWindowError::va_below_image_base,
            "requested VA is below the PE image base");
    }

    const auto rva64 = va - image.image_base;
    if (rva64 > std::numeric_limits<std::uint32_t>::max()) {
        return fail(
            ExeByteWindowError::rva_overflow,
            "requested VA cannot be represented as a PE32/PE32+ RVA");
    }
    const auto rva = static_cast<std::uint32_t>(rva64);
    const auto requested_begin = static_cast<std::uint64_t>(rva);
    const auto requested_end = requested_begin + static_cast<std::uint64_t>(size);

    const PeSection* mapped_section = nullptr;
    std::uint32_t mapped_delta = 0U;
    for (const auto& section : image.sections) {
        if (!section_maps_file_backed_va(section, rva)) {
            continue;
        }
        if (mapped_section != nullptr) {
            return fail(
                ExeByteWindowError::ambiguous_raw_mapping,
                "requested RVA is backed by more than one PE section file-backed VA mapping");
        }
        mapped_section = &section;
        mapped_delta = rva - section.virtual_address;
    }

    if (rva < image.size_of_headers) {
        const auto remaining_header_bytes =
            static_cast<std::size_t>(image.size_of_headers - rva);
        if (size > remaining_header_bytes) {
            return fail(
                ExeByteWindowError::crosses_raw_mapping,
                "requested window crosses the PE header mapping boundary");
        }

        for (const auto& section : image.sections) {
            if (file_backed_rva_range_overlaps(
                    section, requested_begin, requested_end)) {
                return fail(
                    ExeByteWindowError::ambiguous_raw_mapping,
                    "requested header window overlaps section-backed VA data");
            }
        }

        if (!file_range_fits(file_bytes, rva, size)) {
            return fail(
                ExeByteWindowError::file_range_out_of_bounds,
                "requested PE header window exceeds the supplied file bytes");
        }

        const auto offset = static_cast<std::size_t>(rva);
        return ExeByteWindowResult{
            .window = ExeByteWindow{
                .va = va,
                .rva = rva,
                .file_offset = rva,
                .section_name = "<headers>",
                .bytes = std::vector<std::byte>(
                    file_bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                    file_bytes.begin() + static_cast<std::ptrdiff_t>(offset + size)),
            },
            .error = ExeByteWindowError::none,
            .message = {},
        };
    }

    if (mapped_section == nullptr) {
        return fail(
            ExeByteWindowError::unmapped_rva,
            "requested RVA is not backed by both PE virtual section extent and raw file data");
    }

    const auto mapped_extent = file_backed_virtual_extent(*mapped_section);
    const auto mapped_remaining =
        static_cast<std::size_t>(mapped_extent - mapped_delta);
    if (size > mapped_remaining) {
        return fail(
            ExeByteWindowError::crosses_raw_mapping,
            "requested window crosses the containing PE section file-backed VA boundary");
    }

    for (const auto& section : image.sections) {
        if (&section == mapped_section) {
            continue;
        }
        if (file_backed_rva_range_overlaps(
                section, requested_begin, requested_end)) {
            return fail(
                ExeByteWindowError::ambiguous_raw_mapping,
                "requested window overlaps another PE section file-backed VA mapping");
        }
    }

    if (mapped_section->raw_offset >
        std::numeric_limits<std::uint32_t>::max() - mapped_delta) {
        return fail(
            ExeByteWindowError::unmapped_rva,
            "requested RVA cannot be mapped to a file offset");
    }

    const auto file_offset = mapped_section->raw_offset + mapped_delta;
    if (!file_range_fits(file_bytes, file_offset, size)) {
        return fail(
            ExeByteWindowError::file_range_out_of_bounds,
            "requested section window exceeds the supplied file bytes");
    }

    const auto offset = static_cast<std::size_t>(file_offset);
    return ExeByteWindowResult{
        .window = ExeByteWindow{
            .va = va,
            .rva = rva,
            .file_offset = file_offset,
            .section_name = mapped_section->name,
            .bytes = std::vector<std::byte>(
                file_bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                file_bytes.begin() + static_cast<std::ptrdiff_t>(offset + size)),
        },
        .error = ExeByteWindowError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::exe
