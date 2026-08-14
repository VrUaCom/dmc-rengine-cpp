#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"
#include "dmc_rengine/profiles/dmc3/vanilla_static_tables.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u64(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_ascii_z(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    const std::string& value) {
    for (std::size_t index = 0U; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
    bytes[offset + value.size()] = std::byte{0};
}

[[nodiscard]] std::string numbered_name(
    const char* prefix,
    std::uint32_t index,
    const char* extension) {
    char buffer[64]{};
    const auto count = std::snprintf(
        buffer,
        sizeof(buffer),
        "%s%04u%s",
        prefix,
        static_cast<unsigned>(index),
        extension);
    assert(count > 0);
    return std::string{buffer, static_cast<std::size_t>(count)};
}

} // namespace

int main() {
    namespace vanilla = dmc::rengine::profiles::dmc3::vanilla;

    constexpr std::uint64_t image_base = 0x140000000ULL;
    constexpr std::uint32_t data_rva = 0x00553000U;
    constexpr std::uint32_t data_raw = 0x400U;
    constexpr std::uint32_t data_size = 0x50000U;

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(data_raw) + data_size,
        std::byte{0});

    dmc::rengine::exe::PeImage image{
        .kind = dmc::rengine::exe::PeKind::pe32_plus,
        .machine = dmc::rengine::exe::PeMachine::amd64,
        .section_count = 1U,
        .image_base = image_base,
        .entry_point_rva = 0x1000U,
        .size_of_image = data_rva + data_size,
        .size_of_headers = data_raw,
        .subsystem = 2U,
        .sections = {
            dmc::rengine::exe::PeSection{
                .name = ".data",
                .virtual_size = data_size,
                .virtual_address = data_rva,
                .raw_size = data_size,
                .raw_offset = data_raw,
                .characteristics = 0U,
            },
        },
    };

    const auto file_offset_for_va = [=](std::uint64_t va) {
        assert(va >= image_base + data_rva);
        return static_cast<std::size_t>(data_raw) +
            static_cast<std::size_t>(va - image_base - data_rva);
    };
    const auto va_for_file_offset = [=](std::size_t offset) {
        assert(offset >= data_raw);
        return image_base + data_rva +
            static_cast<std::uint64_t>(offset - data_raw);
    };

    const auto master_table_offset = file_offset_for_va(
        vanilla::numeric_resource_resolver.master_name_catalog_va);
    const auto audio_table_offset = file_offset_for_va(vanilla::hd_audio.table_va);

    std::size_t string_cursor = 0x20000U;
    for (std::uint32_t index = 0U;
         index < vanilla::numeric_resource_resolver.master_name_count;
         ++index) {
        const auto name = numbered_name("resource_", index, ".pac");
        write_u64(
            bytes,
            master_table_offset + static_cast<std::size_t>(index) * 8U,
            va_for_file_offset(string_cursor));
        write_ascii_z(bytes, string_cursor, name);
        string_cursor += name.size() + 1U;
    }

    for (std::uint32_t index = 0U; index < vanilla::hd_audio.record_count; ++index) {
        const auto name = numbered_name("battle_", index, ".ogg");
        const auto record = audio_table_offset +
            static_cast<std::size_t>(index) * vanilla::hd_audio.record_stride;
        write_u64(bytes, record, va_for_file_offset(string_cursor));
        if (index == 0U) {
            write_u32(
                bytes,
                record + vanilla::hd_audio_descriptor_abi.loop_start_ms_offset,
                std::numeric_limits<std::uint32_t>::max());
            write_u32(
                bytes,
                record + vanilla::hd_audio_descriptor_abi.loop_end_ms_offset,
                std::numeric_limits<std::uint32_t>::max());
        } else {
            write_u32(
                bytes,
                record + vanilla::hd_audio_descriptor_abi.loop_start_ms_offset,
                1000U + index);
            write_u32(
                bytes,
                record + vanilla::hd_audio_descriptor_abi.loop_end_ms_offset,
                5000U + index);
        }
        write_ascii_z(bytes, string_cursor, name);
        string_cursor += name.size() + 1U;
    }

    const auto master = vanilla::VanillaStaticTableReader::read_master_resource_catalog(
        std::span<const std::byte>{bytes}, image);
    assert(master.complete());
    assert(master.names.size() == 4039U);
    assert(master.names.front().index == 0U);
    assert(master.names.front().logical_name == "resource_0000.pac");
    assert(master.names.back().logical_name == "resource_4038.pac");

    const auto audio = vanilla::VanillaStaticTableReader::read_hd_audio_descriptors(
        std::span<const std::byte>{bytes}, image);
    assert(audio.complete());
    assert(audio.descriptors.size() == 154U);
    assert(audio.descriptors.front().filename == "battle_0000.ogg");
    assert(!audio.descriptors.front().has_explicit_loop());
    assert(audio.descriptors[1U].has_explicit_loop());
    assert(audio.descriptors[1U].loop_start_ms == 1001U);
    assert(audio.descriptors[1U].loop_end_ms == 5001U);

    const auto bad_master = vanilla::VanillaStaticTableReader::read_master_resource_catalog(
        std::span<const std::byte>{bytes}, image, 4U);
    assert(!bad_master.complete());
    assert(!bad_master.diagnostics.empty());

    return 0;
}
