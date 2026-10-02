#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/hits/editor.hpp"
#include "dmc_rengine/hits/pac_editor.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(i * 8U)) & 0xFFU);
    }
}

void write_i32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_vec3(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float x,
    float y,
    float z) {
    write_f32(bytes, offset, x);
    write_f32(bytes, offset + 4U, y);
    write_f32(bytes, offset + 8U, z);
}

[[nodiscard]] std::vector<std::byte> make_hits(
    std::uint32_t flags) {
    constexpr std::size_t pointer_table = 0x44U;
    constexpr std::size_t list_offset = 0x48U;
    constexpr std::size_t triangle_offset = 0x50U;
    constexpr std::size_t end_offset = triangle_offset + 0x38U;
    constexpr std::size_t file_size = 0x90U;

    std::vector<std::byte> bytes(file_size, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -10.0F, -10.0F, -10.0F);
    write_vec3(bytes, 0x14U, 10.0F, 10.0F, 10.0F);
    write_vec3(bytes, 0x20U, 20.0F, 20.0F, 20.0F);
    write_u32(bytes, 0x2CU, 1U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 1U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(
        bytes,
        0x40U,
        static_cast<std::uint32_t>(triangle_offset - 8U));
    write_i32(
        bytes,
        pointer_table,
        static_cast<std::int32_t>(list_offset - 8U));
    write_i32(bytes, list_offset, 0);
    write_i32(bytes, list_offset + 4U, -1);

    write_u32(bytes, triangle_offset, flags);
    write_vec3(bytes, triangle_offset + 0x04U, 0.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x10U, 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x1CU, 0.0F, 0.0F, 1.0F);
    write_vec3(bytes, triangle_offset + 0x28U, 0.0F, -1.0F, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_pac() {
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto hits0 = make_hits(0x18060001U);
    const auto hits1 = make_hits(0x00000001U);

    std::vector<std::byte> bytes(0x160U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    write_u32(bytes, 4U, 7U);

    // Physical slots 3 and 6 are independent HITS resources.
    write_u32(bytes, 8U + 3U * 4U, 0x40U);
    write_u32(bytes, 8U + 6U * 4U, 0xD0U);

    std::copy(hits0.begin(), hits0.end(), bytes.begin() + 0x40U);
    std::copy(hits1.begin(), hits1.end(), bytes.begin() + 0xD0U);

    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "hits-editor-test",
                .logical_path = "GData.afs/st000.pac",
                .container_chain = "nbz[0]",
                .offset = 0U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "st000.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace editor = dmc::rengine::hits::editor;
    namespace pac_editor = dmc::rengine::hits::pac_editor;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto parent = make_pac();
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{
            parent.bytes.data(),
            parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parsed.ok());
    const auto expansion =
        gdspaces::ContainerExpander::expand(parent, parsed);
    assert(expansion.usable());

    const auto slot3 = std::find_if(
        expansion.children.begin(),
        expansion.children.end(),
        [](const gdspaces::ContainerChild& child) {
            return child.entry.slot_index == 3U;
        });
    const auto slot6 = std::find_if(
        expansion.children.begin(),
        expansion.children.end(),
        [](const gdspaces::ContainerChild& child) {
            return child.entry.slot_index == 6U;
        });
    assert(slot3 != expansion.children.end());
    assert(slot6 != expansion.children.end());
    const auto original_slot6 = slot6->payload.bytes;

    auto session_opened = editor::Session::open(slot3->payload.bytes);
    assert(session_opened.has_value());
    auto session = std::move(*session_opened);
    assert(session.set_collision_preset(
        1U,
        editor::CollisionPreset::green_raw_0000000a));
    const auto rebuilt_hits = session.rebuild();
    assert(rebuilt_hits.ok());

    const auto replaced = pac_editor::PacHitsWriter::replace_slot(
        parent,
        3U,
        rebuilt_hits.bytes,
        session.revision());
    assert(replaced.ok());
    assert(replaced.slot_index == 3U);

    auto reopened_parent = parent;
    reopened_parent.bytes = replaced.bytes;
    reopened_parent.resource.id.size =
        static_cast<std::uint64_t>(reopened_parent.bytes.size());

    const auto reparsed = registry.parse(
        std::span<const std::byte>{
            reopened_parent.bytes.data(),
            reopened_parent.bytes.size()},
        reopened_parent.resource.id.logical_path);
    assert(reparsed.ok());
    const auto reopened =
        gdspaces::ContainerExpander::expand(reopened_parent, reparsed);
    assert(reopened.usable());

    const auto new_slot3 = std::find_if(
        reopened.children.begin(),
        reopened.children.end(),
        [](const gdspaces::ContainerChild& child) {
            return child.entry.slot_index == 3U;
        });
    const auto new_slot6 = std::find_if(
        reopened.children.begin(),
        reopened.children.end(),
        [](const gdspaces::ContainerChild& child) {
            return child.entry.slot_index == 6U;
        });
    assert(new_slot3 != reopened.children.end());
    assert(new_slot6 != reopened.children.end());
    assert(new_slot3->payload.bytes == rebuilt_hits.bytes);
    assert(new_slot6->payload.bytes == original_slot6);

    const auto new_hits_scan =
        dmc::rengine::formats::hits::RecordScanner::scan(
            new_slot3->payload.bytes);
    assert(new_hits_scan.ok());
    assert(new_hits_scan.triangles.size() == 1U);
    assert(new_hits_scan.triangles[0].flags == 0x0000000AU);

    const auto missing = pac_editor::PacHitsWriter::replace_slot(
        parent,
        4U,
        rebuilt_hits.bytes);
    assert(
        missing.status ==
        pac_editor::ReplaceStatus::target_slot_not_found);

    std::vector<std::byte> bad_hits{
        std::byte{'N'},
        std::byte{'O'},
        std::byte{'P'},
        std::byte{'E'},
    };
    const auto invalid = pac_editor::PacHitsWriter::replace_slot(
        parent,
        3U,
        bad_hits);
    assert(
        invalid.status ==
        pac_editor::ReplaceStatus::invalid_replacement_hits);

    return 0;
}
