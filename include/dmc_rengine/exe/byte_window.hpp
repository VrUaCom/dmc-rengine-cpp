#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

inline constexpr std::size_t k_max_exe_byte_window_size = 0x10000U;

enum class ExeByteWindowError : std::uint8_t {
    none,
    zero_size,
    size_limit_exceeded,
    va_below_image_base,
    rva_overflow,
    unmapped_rva,
    ambiguous_raw_mapping,
    crosses_raw_mapping,
    file_range_out_of_bounds,
};

[[nodiscard]] constexpr const char* to_string(ExeByteWindowError error) noexcept {
    switch (error) {
    case ExeByteWindowError::none: return "none";
    case ExeByteWindowError::zero_size: return "zero-size";
    case ExeByteWindowError::size_limit_exceeded: return "size-limit-exceeded";
    case ExeByteWindowError::va_below_image_base: return "va-below-image-base";
    case ExeByteWindowError::rva_overflow: return "rva-overflow";
    case ExeByteWindowError::unmapped_rva: return "unmapped-rva";
    case ExeByteWindowError::ambiguous_raw_mapping: return "ambiguous-raw-mapping";
    case ExeByteWindowError::crosses_raw_mapping: return "crosses-raw-mapping";
    case ExeByteWindowError::file_range_out_of_bounds: return "file-range-out-of-bounds";
    }
    return "unknown";
}

struct ExeByteWindow final {
    std::uint64_t va{};
    std::uint32_t rva{};
    std::uint32_t file_offset{};
    std::string section_name;
    std::vector<std::byte> bytes;
};

struct ExeByteWindowResult final {
    std::optional<ExeByteWindow> window;
    ExeByteWindowError error{ExeByteWindowError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return window.has_value() && error == ExeByteWindowError::none;
    }
};

class ExeByteWindowExtractor final {
public:
    [[nodiscard]] static ExeByteWindowResult extract(
        std::span<const std::byte> file_bytes,
        const PeImage& image,
        std::uint64_t va,
        std::size_t size);
};

} // namespace dmc::rengine::exe
