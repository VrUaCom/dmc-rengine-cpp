#include "dmc_rengine/exe/byte_window.hpp"

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

    if (rva < image.size_of_headers) {
        const auto remaining_header_bytes =
            static_cast<std::size_t>(image.size_of_headers - rva);
        if (size > remaining_header_bytes) {
            return fail(
                ExeByteWindowError::crosses_raw_mapping,
                "requested window crosses the PE header mapping boundary");
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

    for (const auto& section : image.sections) {
        if (rva < section.virtual_address) {
            continue;
        }

        const auto delta = rva - section.virtual_address;
        if (delta >= section.raw_size) {
            continue;
        }

        const auto raw_remaining = static_cast<std::size_t>(section.raw_size - delta);
        if (size > raw_remaining) {
            return fail(
                ExeByteWindowError::crosses_raw_mapping,
                "requested window crosses the containing PE section raw-data boundary");
        }
        if (section.raw_offset >
            std::numeric_limits<std::uint32_t>::max() - delta) {
            return fail(
                ExeByteWindowError::unmapped_rva,
                "requested RVA cannot be mapped to a file offset");
        }

        const auto file_offset = section.raw_offset + delta;
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
                .section_name = section.name,
                .bytes = std::vector<std::byte>(
                    file_bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                    file_bytes.begin() + static_cast<std::ptrdiff_t>(offset + size)),
            },
            .error = ExeByteWindowError::none,
            .message = {},
        };
    }

    return fail(
        ExeByteWindowError::unmapped_rva,
        "requested RVA is not backed by PE header or section raw data");
}

} // namespace dmc::rengine::exe
