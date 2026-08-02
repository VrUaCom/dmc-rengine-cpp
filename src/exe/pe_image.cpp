#include "dmc_rengine/exe/pe_image.hpp"

#include <algorithm>
#include <limits>

namespace dmc::rengine::exe {

bool PeSection::contains_rva(std::uint32_t rva) const noexcept {
    if (rva < virtual_address) {
        return false;
    }

    const auto extent = std::max(virtual_size, raw_size);
    return static_cast<std::uint64_t>(rva - virtual_address) <
        static_cast<std::uint64_t>(extent);
}

bool PeSection::contains_file_offset(std::uint32_t offset) const noexcept {
    if (offset < raw_offset) {
        return false;
    }

    return static_cast<std::uint64_t>(offset - raw_offset) <
        static_cast<std::uint64_t>(raw_size);
}

std::optional<std::uint64_t> PeImage::rva_to_va(
    std::uint32_t rva) const noexcept {
    if (image_base > std::numeric_limits<std::uint64_t>::max() - rva) {
        return std::nullopt;
    }
    return image_base + rva;
}

std::optional<std::uint32_t> PeImage::rva_to_file_offset(
    std::uint32_t rva) const noexcept {
    if (rva < size_of_headers) {
        return rva;
    }

    for (const auto& section : sections) {
        if (rva < section.virtual_address) {
            continue;
        }

        const auto delta = rva - section.virtual_address;
        if (delta >= section.raw_size ||
            section.raw_offset > std::numeric_limits<std::uint32_t>::max() - delta) {
            continue;
        }

        return section.raw_offset + delta;
    }

    return std::nullopt;
}

std::optional<std::uint32_t> PeImage::file_offset_to_rva(
    std::uint32_t offset) const noexcept {
    if (offset < size_of_headers) {
        return offset;
    }

    for (const auto& section : sections) {
        if (!section.contains_file_offset(offset)) {
            continue;
        }

        const auto delta = offset - section.raw_offset;
        if (section.virtual_address >
            std::numeric_limits<std::uint32_t>::max() - delta) {
            return std::nullopt;
        }

        return section.virtual_address + delta;
    }

    return std::nullopt;
}

} // namespace dmc::rengine::exe
