#include "dmc_rengine/exe/pe_reader.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] bool has_range(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::size_t size) noexcept {
    return offset <= bytes.size() && size <= bytes.size() - offset;
}

[[nodiscard]] std::optional<std::uint16_t> read_u16(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 2U)) {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(
             std::to_integer<std::uint8_t>(bytes[offset + 1U]))
         << 8U));
}

[[nodiscard]] std::optional<std::uint32_t> read_u32(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 4U)) {
        return std::nullopt;
    }

    std::uint32_t value = 0;
    for (std::size_t index = 0; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(
                     std::to_integer<std::uint8_t>(bytes[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

[[nodiscard]] std::optional<std::uint64_t> read_u64(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    if (!has_range(bytes, offset, 8U)) {
        return std::nullopt;
    }

    std::uint64_t value = 0;
    for (std::size_t index = 0; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
                     std::to_integer<std::uint8_t>(bytes[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return value;
}

[[nodiscard]] std::string read_name(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    std::string name;
    for (std::size_t index = 0; index < 8U; ++index) {
        const auto character = std::to_integer<unsigned char>(bytes[offset + index]);
        if (character == 0U) {
            break;
        }
        name.push_back(static_cast<char>(character));
    }
    return name;
}

[[nodiscard]] PeMachine machine_from_value(std::uint16_t value) noexcept {
    switch (value) {
    case static_cast<std::uint16_t>(PeMachine::i386): return PeMachine::i386;
    case static_cast<std::uint16_t>(PeMachine::amd64): return PeMachine::amd64;
    case static_cast<std::uint16_t>(PeMachine::arm64): return PeMachine::arm64;
    default: return PeMachine::unknown;
    }
}

} // namespace

PeReadResult PeReader::read(std::span<const std::byte> bytes) {
    PeReadResult result;

    if (!has_range(bytes, 0U, 0x40U)) {
        result.errors.emplace_back("File is too small for a DOS header.");
        return result;
    }

    const auto dos_magic = read_u16(bytes, 0U);
    if (!dos_magic.has_value() || *dos_magic != 0x5A4DU) {
        result.errors.emplace_back("Missing MZ signature.");
        return result;
    }

    const auto pe_offset_value = read_u32(bytes, 0x3CU);
    if (!pe_offset_value.has_value()) {
        result.errors.emplace_back("Missing e_lfanew field.");
        return result;
    }

    const auto pe_offset = static_cast<std::size_t>(*pe_offset_value);
    if (!has_range(bytes, pe_offset, 24U)) {
        result.errors.emplace_back("PE header points outside the file.");
        return result;
    }

    const auto signature = read_u32(bytes, pe_offset);
    if (!signature.has_value() || *signature != 0x00004550U) {
        result.errors.emplace_back("Missing PE signature.");
        return result;
    }

    const auto machine_value = read_u16(bytes, pe_offset + 4U);
    const auto section_count_value = read_u16(bytes, pe_offset + 6U);
    const auto optional_size_value = read_u16(bytes, pe_offset + 20U);
    if (!machine_value.has_value() || !section_count_value.has_value() ||
        !optional_size_value.has_value()) {
        result.errors.emplace_back("Truncated COFF header.");
        return result;
    }

    if (*section_count_value > 4096U) {
        result.errors.emplace_back("Section count exceeds the safety limit.");
        return result;
    }

    const auto optional_offset = pe_offset + 24U;
    const auto optional_size = static_cast<std::size_t>(*optional_size_value);
    if (!has_range(bytes, optional_offset, optional_size) || optional_size < 70U) {
        result.errors.emplace_back("Truncated or unsupported optional header.");
        return result;
    }

    const auto optional_magic = read_u16(bytes, optional_offset);
    if (!optional_magic.has_value()) {
        result.errors.emplace_back("Missing optional-header magic.");
        return result;
    }

    PeImage image;
    if (*optional_magic == 0x10BU) {
        image.kind = PeKind::pe32;
        const auto image_base = read_u32(bytes, optional_offset + 28U);
        if (!image_base.has_value()) {
            result.errors.emplace_back("Truncated PE32 image base.");
            return result;
        }
        image.image_base = *image_base;
    } else if (*optional_magic == 0x20BU) {
        image.kind = PeKind::pe32_plus;
        const auto image_base = read_u64(bytes, optional_offset + 24U);
        if (!image_base.has_value()) {
            result.errors.emplace_back("Truncated PE32+ image base.");
            return result;
        }
        image.image_base = *image_base;
    } else {
        result.errors.emplace_back("Unsupported optional-header magic.");
        return result;
    }

    const auto entry_point = read_u32(bytes, optional_offset + 16U);
    const auto size_of_image = read_u32(bytes, optional_offset + 56U);
    const auto size_of_headers = read_u32(bytes, optional_offset + 60U);
    const auto subsystem = read_u16(bytes, optional_offset + 68U);
    if (!entry_point.has_value() || !size_of_image.has_value() ||
        !size_of_headers.has_value() || !subsystem.has_value()) {
        result.errors.emplace_back("Required optional-header fields are truncated.");
        return result;
    }

    image.machine = machine_from_value(*machine_value);
    image.section_count = *section_count_value;
    image.entry_point_rva = *entry_point;
    image.size_of_image = *size_of_image;
    image.size_of_headers = *size_of_headers;
    image.subsystem = *subsystem;

    const auto section_table_offset = optional_offset + optional_size;
    constexpr std::size_t section_header_size = 40U;
    const auto section_count = static_cast<std::size_t>(image.section_count);
    if (section_count > (bytes.size() - section_table_offset) / section_header_size) {
        result.errors.emplace_back("Truncated section table.");
        return result;
    }

    image.sections.reserve(section_count);
    for (std::size_t index = 0; index < section_count; ++index) {
        const auto offset = section_table_offset + index * section_header_size;
        const auto virtual_size = read_u32(bytes, offset + 8U);
        const auto virtual_address = read_u32(bytes, offset + 12U);
        const auto raw_size = read_u32(bytes, offset + 16U);
        const auto raw_offset = read_u32(bytes, offset + 20U);
        const auto characteristics = read_u32(bytes, offset + 36U);
        if (!virtual_size.has_value() || !virtual_address.has_value() ||
            !raw_size.has_value() || !raw_offset.has_value() ||
            !characteristics.has_value()) {
            result.errors.emplace_back("Truncated section header.");
            return result;
        }

        PeSection section{
            .name = read_name(bytes, offset),
            .virtual_size = *virtual_size,
            .virtual_address = *virtual_address,
            .raw_size = *raw_size,
            .raw_offset = *raw_offset,
            .characteristics = *characteristics,
        };

        const auto raw_end = static_cast<std::uint64_t>(section.raw_offset) +
            static_cast<std::uint64_t>(section.raw_size);
        if (raw_end > static_cast<std::uint64_t>(bytes.size())) {
            result.errors.emplace_back(
                "Section '" + section.name + "' exceeds the file boundary.");
        }

        const auto virtual_end = static_cast<std::uint64_t>(section.virtual_address) +
            static_cast<std::uint64_t>(std::max(section.virtual_size, section.raw_size));
        if (virtual_end > static_cast<std::uint64_t>(image.size_of_image)) {
            result.warnings.emplace_back(
                "Section '" + section.name + "' exceeds SizeOfImage.");
        }

        image.sections.push_back(std::move(section));
    }

    for (std::size_t left = 0; left < image.sections.size(); ++left) {
        const auto& first = image.sections[left];
        const auto first_begin = static_cast<std::uint64_t>(first.raw_offset);
        const auto first_end = first_begin + first.raw_size;
        if (first.raw_size == 0U) {
            continue;
        }

        for (std::size_t right = left + 1U; right < image.sections.size(); ++right) {
            const auto& second = image.sections[right];
            if (second.raw_size == 0U) {
                continue;
            }

            const auto second_begin = static_cast<std::uint64_t>(second.raw_offset);
            const auto second_end = second_begin + second.raw_size;
            if (first_begin < second_end && second_begin < first_end) {
                result.warnings.emplace_back(
                    "Sections '" + first.name + "' and '" + second.name +
                    "' overlap in the file.");
            }
        }
    }

    if (image.machine == PeMachine::unknown) {
        result.warnings.emplace_back("PE machine type is not recognized.");
    }

    if (!image.rva_to_file_offset(image.entry_point_rva).has_value()) {
        result.warnings.emplace_back("Entry point RVA does not map to file data.");
    }

    if (result.errors.empty()) {
        result.image = std::move(image);
    }
    return result;
}

} // namespace dmc::rengine::exe
