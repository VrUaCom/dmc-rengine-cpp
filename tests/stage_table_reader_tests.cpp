#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

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

[[nodiscard]] dmc::rengine::exe::PeImage make_image() {
    using dmc::rengine::exe::PeImage;
    using dmc::rengine::exe::PeKind;
    using dmc::rengine::exe::PeMachine;
    using dmc::rengine::exe::PeSection;

    return PeImage{
        .kind = PeKind::pe32_plus,
        .machine = PeMachine::amd64,
        .section_count = 2U,
        .image_base = 0x140000000ULL,
        .entry_point_rva = 0x0034615CU,
        .size_of_image = 0x00DAD000U,
        .size_of_headers = 0x400U,
        .subsystem = 2U,
        .sections = {
            PeSection{
                .name = ".rdata",
                .virtual_size = 0x00203368U,
                .virtual_address = 0x0034F000U,
                .raw_size = 0x00203400U,
                .raw_offset = 0x0034E200U,
                .characteristics = 0x40000040U,
            },
            PeSection{
                .name = ".data",
                .virtual_size = 0x0081F1B8U,
                .virtual_address = 0x00553000U,
                .raw_size = 0x00086000U,
                .raw_offset = 0x00551600U,
                .characteristics = 0xC0000040U,
            },
        },
    };
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;

    const auto& descriptor = phase12_stage_resource_table();
    std::vector<std::byte> bytes(
        static_cast<std::size_t>(descriptor.artifact_size),
        std::byte{0});

    const auto image = make_image();
    std::size_t string_offset = 0x00502000U;

    for (std::uint32_t row = 0U; row < descriptor.row_count; ++row) {
        const std::array<std::string, 4> paths{
            "scr\\row_" + std::to_string(row) + ".pac",
            "room\\row_" + std::to_string(row) + "_cfg.pac",
            "room\\row_" + std::to_string(row) + "_fx.pac",
            "se\\row_" + std::to_string(row) + ".pac",
        };

        for (std::size_t column = 0U; column < paths.size(); ++column) {
            const auto path_rva = image.file_offset_to_rva(
                static_cast<std::uint32_t>(string_offset));
            assert(path_rva.has_value());
            const auto path_va = image.rva_to_va(*path_rva);
            assert(path_va.has_value());

            const auto entry_index =
                static_cast<std::uint64_t>(row) * paths.size() + column;
            const auto cell_offset = descriptor.file_offset +
                entry_index * descriptor.cell_stride;
            write_u64(
                bytes,
                static_cast<std::size_t>(cell_offset + descriptor.path_pointer_offset),
                *path_va);
            write_ascii_z(bytes, string_offset, paths[column]);
            string_offset += paths[column].size() + 1U;
        }
    }

    return bytes;
}

[[nodiscard]] bool has_code(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    const std::string& code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::StageResourceTableReader;
    using dmc::rengine::profiles::dmc3::phase12_stage_resource_table;

    const auto& descriptor = phase12_stage_resource_table();
    assert(descriptor.valid());
    assert(descriptor.artifact_size == 6356432ULL);
    assert(descriptor.file_offset == 0x005C30A8ULL);
    assert(descriptor.rva == 0x005C4AA8U);
    assert(descriptor.va == 0x1405C4AA8ULL);
    assert(descriptor.cell_stride == 0x10U);
    assert(descriptor.path_pointer_offset == 0U);
    assert(descriptor.table_size_bytes() == 0x1B80ULL);

    const auto image = make_image();
    const auto bytes = make_fixture();
    const auto result = StageResourceTableReader::read(
        std::span<const std::byte>{bytes}, image, descriptor);
    assert(result.complete(descriptor));
    assert(result.rows.size() == 110U);

    const auto& first = result.rows.front();
    assert(first.row_index == 0U);
    assert(first.complete());
    assert(first.cells[0].role == StageResourceRole::script);
    assert(first.cells[1].role == StageResourceRole::room_config);
    assert(first.cells[2].role == StageResourceRole::room_effects);
    assert(first.cells[3].role == StageResourceRole::room_sound);
    assert(first.cells[0].cell_file_offset == descriptor.file_offset);
    assert(first.cells[0].cell_rva == descriptor.rva);
    assert(first.cells[0].cell_va == descriptor.va);
    assert(first.cells[0].logical_path == "scr\\row_0.pac");

    const auto first_paths = first.logical_paths();
    assert(first_paths[1] == "room\\row_0_cfg.pac");
    assert(first_paths[2] == "room\\row_0_fx.pac");
    assert(first_paths[3] == "se\\row_0.pac");

    const auto& last = result.rows.back();
    assert(last.row_index == 109U);
    assert(last.cells[0].logical_path == "scr\\row_109.pac");
    assert(last.cells[3].logical_path == "se\\row_109.pac");

    auto wrong_size = bytes;
    wrong_size.pop_back();
    const auto size_mismatch = StageResourceTableReader::read(
        std::span<const std::byte>{wrong_size}, image, descriptor);
    assert(!size_mismatch.complete(descriptor));
    assert(has_code(
        size_mismatch.diagnostics,
        "dmc3.stage-table.artifact-size-mismatch"));

    auto bad_pointer = bytes;
    write_u64(
        bad_pointer,
        static_cast<std::size_t>(descriptor.file_offset),
        image.image_base - 1U);
    const auto invalid_pointer = StageResourceTableReader::read(
        std::span<const std::byte>{bad_pointer}, image, descriptor);
    assert(!invalid_pointer.complete(descriptor));
    assert(has_code(
        invalid_pointer.diagnostics,
        "dmc3.stage-table.invalid-path-pointer"));

    const auto tiny_path_limit = StageResourceTableReader::read(
        std::span<const std::byte>{bytes}, image, descriptor, 4U);
    assert(!tiny_path_limit.complete(descriptor));
    assert(has_code(
        tiny_path_limit.diagnostics,
        "dmc3.stage-table.invalid-path-string"));

    return 0;
}
