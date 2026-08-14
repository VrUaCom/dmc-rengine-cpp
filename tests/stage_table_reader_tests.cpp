#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

void write_ascii_z(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    const std::string& value) {
    std::memcpy(bytes.data() + offset, value.data(), value.size());
    bytes[offset + value.size()] = std::byte{0};
}

struct Fixture final {
    std::vector<std::byte> bytes;
    dmc::rengine::exe::PeImage image;
    dmc::rengine::profiles::dmc3::StageResourceTableDescriptor descriptor;
};

[[nodiscard]] Fixture make_fixture() {
    using dmc::rengine::exe::PeKind;
    using dmc::rengine::exe::PeMachine;
    using dmc::rengine::exe::PeSection;
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::StageResourceTableDescriptor;

    constexpr std::uint64_t image_base = 0x140000000ULL;
    constexpr std::uint32_t table_rva = 0x2000U;
    constexpr std::uint32_t strings_rva = 0x5000U;
    constexpr std::uint32_t section_raw = 0x400U;
    constexpr std::uint32_t section_rva = 0x1000U;
    constexpr std::size_t table_file_offset =
        section_raw + (table_rva - section_rva);
    constexpr std::size_t strings_file_offset =
        section_raw + (strings_rva - section_rva);

    // Allocate the final synthetic artifact size before populating either the
    // canonical 110x4 table span or the non-overlapping string pool.
    std::vector<std::byte> bytes(0x10000U, std::byte{0});
    const std::array<std::string, 8> paths{
        "scr/st000.pac",
        "room/st000cfg.pac",
        "room/st000_effect.pac",
        "se/snd_r000.pac",
        "scr/shared_intro.pac",
        "room/st777cfg_alias.pac",
        "room/common_effects.pac",
        "se/snd_shared.pac",
    };

    std::array<std::uint32_t, 8> path_rvas{};
    std::size_t string_cursor = strings_file_offset;
    std::uint32_t string_rva_cursor = strings_rva;
    for (std::size_t index = 0U; index < paths.size(); ++index) {
        path_rvas[index] = string_rva_cursor;
        write_ascii_z(bytes, string_cursor, paths[index]);
        string_cursor += paths[index].size() + 1U;
        string_rva_cursor += static_cast<std::uint32_t>(paths[index].size() + 1U);
    }

    dmc::rengine::exe::PeImage image{
        .kind = PeKind::pe32_plus,
        .machine = PeMachine::amd64,
        .section_count = 1U,
        .image_base = image_base,
        .entry_point_rva = 0x1000U,
        .size_of_image = 0x10000U,
        .size_of_headers = section_raw,
        .subsystem = 2U,
        .sections = {
            PeSection{
                .name = ".data",
                .virtual_size = 0xF000U,
                .virtual_address = section_rva,
                .raw_size = 0xF000U,
                .raw_offset = section_raw,
                .characteristics = 0U,
            },
        },
    };

    StageResourceTableDescriptor descriptor{
        .id = "synthetic-stage-table",
        .evidence_packet_id = "synthetic-evidence",
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .artifact_size = bytes.size(),
        .file_offset = table_file_offset,
        .rva = table_rva,
        .va = image_base + table_rva,
        .row_count = 110U,
        .cell_stride = 0x10U,
        .path_pointer_offset = 0U,
        .columns = {
            StageResourceRole::script,
            StageResourceRole::room_config,
            StageResourceRole::room_effects,
            StageResourceRole::room_sound,
        },
    };

    // Repeat two deliberately different logical rows across all 110 observed
    // table rows. The second row is intentionally non-pattern/shared-looking so
    // the reader regression cannot depend on an stNNN naming convention.
    for (std::uint32_t row = 0U; row < descriptor.row_count; ++row) {
        for (std::uint32_t column = 0U; column < 4U; ++column) {
            const auto source_index = static_cast<std::size_t>((row % 2U) * 4U + column);
            const auto cell = table_file_offset +
                (static_cast<std::size_t>(row) * 4U + column) * 0x10U;
            write_u64(bytes, cell, image_base + path_rvas[source_index]);
            // Preserve the unresolved +0x08 field as zero in the synthetic
            // fixture; the production reader deliberately does not interpret it.
            write_u64(bytes, cell + 8U, 0U);
        }
    }

    return Fixture{
        .bytes = std::move(bytes),
        .image = std::move(image),
        .descriptor = std::move(descriptor),
    };
}

} // namespace

int main() {
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::StageResourceTableReader;

    auto fixture = make_fixture();
    assert(fixture.descriptor.valid());

    const auto result = StageResourceTableReader::read(
        std::span<const std::byte>{fixture.bytes},
        fixture.image,
        fixture.descriptor,
        260U);
    assert(result.complete(fixture.descriptor));
    assert(result.rows.size() == 110U);
    assert(result.rows[0].complete());
    assert(result.rows[0].cells[0].role == StageResourceRole::script);
    assert(result.rows[0].cells[1].role == StageResourceRole::room_config);
    assert(result.rows[0].cells[2].role == StageResourceRole::room_effects);
    assert(result.rows[0].cells[3].role == StageResourceRole::room_sound);
    assert(result.rows[0].cells[0].logical_path == "scr/st000.pac");
    assert(result.rows[1].cells[0].logical_path == "scr/shared_intro.pac");
    assert(result.rows[1].cells[1].logical_path == "room/st777cfg_alias.pac");
    assert(result.rows[1].cells[2].logical_path == "room/common_effects.pac");
    assert(result.rows[1].cells[3].logical_path == "se/snd_shared.pac");
    assert(result.rows[0].logical_paths()[3] == "se/snd_r000.pac");

    auto wrong_size = fixture;
    wrong_size.descriptor.artifact_size += 1U;
    const auto size_failure = StageResourceTableReader::read(
        std::span<const std::byte>{wrong_size.bytes},
        wrong_size.image,
        wrong_size.descriptor,
        260U);
    assert(!size_failure.complete(wrong_size.descriptor));
    assert(!size_failure.diagnostics.empty());

    const auto path_failure = StageResourceTableReader::read(
        std::span<const std::byte>{fixture.bytes},
        fixture.image,
        fixture.descriptor,
        4U);
    assert(!path_failure.complete(fixture.descriptor));
    assert(!path_failure.diagnostics.empty());

    return 0;
}
