#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_writer.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/mod_authored_child_bridge.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void put_u8(std::vector<std::byte>& bytes,
            std::size_t offset,
            std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void put_u16(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint16_t value) {
    put_u8(bytes, offset + 0U,
           static_cast<std::uint8_t>(value & 0xFFU));
    put_u8(bytes, offset + 1U,
           static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void put_u32(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes,
             std::size_t offset,
             float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_ascii(std::vector<std::byte>& bytes,
               std::size_t offset,
               std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(text[index]));
    }
}

std::vector<std::byte> make_valid_mod() {
    std::vector<std::byte> bytes(0x260U, std::byte{0});
    put_ascii(bytes, 0x00U, "MOD ");
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U);
    put_u8(bytes, 0x11U, 1U);
    put_u8(bytes, 0x12U, 8U);
    put_u8(bytes, 0x13U, 0x5AU);
    put_u32(bytes, 0x14U, 0x12345678U);
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U);
    put_u8(bytes, 0x41U, 0x90U);
    put_u16(bytes, 0x42U, 1U);
    put_u64(bytes, 0x48U, 0x80U);
    put_u32(bytes, 0x50U, 0x00004000U);
    put_f32(bytes, 0x70U, 10.0F);
    put_f32(bytes, 0x74U, -20.0F);
    put_f32(bytes, 0x78U, 30.0F);
    put_f32(bytes, 0x7CU, 42.5F);

    put_u16(bytes, 0x80U, 1U);
    put_u16(bytes, 0x82U, 7U);
    put_u16(bytes, 0x84U, 1U);
    put_u16(bytes, 0x86U, 2U);
    put_u16(bytes, 0x88U, 3U);
    put_u16(bytes, 0x8AU, 4U);
    put_u64(bytes, 0x90U, 0xD0U);
    put_u64(bytes, 0x98U, 0xE0U);
    put_u64(bytes, 0xA0U, 0xF0U);
    put_u64(bytes, 0xA8U, 0x100U);
    put_u64(bytes, 0xB0U, 0x110U);
    put_u64(bytes, 0xB8U, 0U);
    put_u64(bytes, 0xC0U, 0xA0U);
    put_u32(bytes, 0xC8U, 0U);
    put_u32(bytes, 0xCCU, 0U);

    put_f32(bytes, 0xD0U, 1.0F);
    put_f32(bytes, 0xD4U, 2.0F);
    put_f32(bytes, 0xD8U, 3.0F);
    put_f32(bytes, 0xE0U, 0.0F);
    put_f32(bytes, 0xE4U, 1.0F);
    put_f32(bytes, 0xE8U, 0.0F);
    put_u16(bytes, 0xF0U, 4096U);
    put_u16(bytes, 0xF2U, 2048U);
    put_u16(bytes, 0x110U, 0x001FU);

    put_u32(bytes, 0x200U, 0x20U);
    put_u32(bytes, 0x204U, 0x24U);
    put_u32(bytes, 0x208U, 0x28U);
    put_u32(bytes, 0x20CU, 0x30U);
    put_u8(bytes, 0x220U, 0xFFU);
    put_u8(bytes, 0x224U, 0U);
    put_u8(bytes, 0x228U, 0U);
    return bytes;
}

dmc::rengine::gdspaces::ResourcePayload make_parent_pac() {
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto mod = make_valid_mod();
    constexpr std::size_t child_offset = 0x20U;
    std::vector<std::byte> bytes(child_offset + mod.size(), std::byte{0});
    put_ascii(bytes, 0U, "PAC\0");
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, static_cast<std::uint32_t>(child_offset));
    std::copy(mod.begin(), mod.end(), bytes.begin() + child_offset);

    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "synthetic-mod-parent",
                .logical_path = "GData.afs/em000.pac",
                .container_chain = "nbz[synthetic]",
                .offset = 0x1000U,
                .size = size,
            },
            .display_name = "em000.pac",
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

std::filesystem::path write_temp_nbz(
    std::span<const std::byte> bytes) {
    const auto path = std::filesystem::temp_directory_path() /
        "dmc-rengine-mod-authored-child-overlay.nbz";
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

const dmc::rengine::gdspaces::ResourceRef* find_path(
    const std::vector<dmc::rengine::gdspaces::ResourceRef>& refs,
    std::string_view path) {
    const auto found = std::find_if(
        refs.begin(), refs.end(),
        [path](const auto& ref) { return ref.id.logical_path == path; });
    return found == refs.end() ? nullptr : &*found;
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace mod = dmc::rengine::formats::mod;

    const auto parent = make_parent_pac();
    const auto registry = dmc3::make_container_parser_registry();
    const auto parent_parse = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parent_parse.ok());

    const auto expansion = gdspaces::ContainerExpander::expand(
        parent, parent_parse);
    assert(expansion.usable());
    assert(expansion.children.size() == 1U);
    const auto& child = expansion.children[0].payload;
    assert(child.resource.id.size == 0x260U);

    const auto child_span = std::span<const std::byte>{
        child.bytes.data(), child.bytes.size()};
    const auto parsed = mod::Parser::parse(child_span);
    assert(parsed.ok());
    assert(parsed.document.outer_models.size() == 1U);

    auto edited = parsed.document;
    edited.outer_models[0].bounding_radius = 50.0F;
    const auto written = mod::Writer::write(child_span, edited);
    assert(written.ok());
    assert(written.receipt.modified_byte_count > 0U);

    const auto bridged = dmc3::ModAuthoredChildBridge::build(child, written);
    assert(bridged.ok());
    assert(bridged.image->resource == child.resource.id);
    assert(bridged.image->source_sha256 == written.receipt.source_sha256);
    assert(bridged.image->output_sha256 == written.receipt.output_sha256);
    assert(bridged.image->writer_mode == "mod-preserve-layout-v1");
    assert(bridged.image->bytes == written.bytes);

    const std::vector<dmc3::AuthoredChildImage> authored{*bridged.image};
    const auto reintegrated = dmc3::NestedRelativeSlotReintegrator::reintegrate(
        parent, expansion, authored);
    assert(reintegrated.ok());
    assert(reintegrated.bytes.size() == parent.bytes.size());
    assert(reintegrated.receipt->spans.size() == 1U);
    assert(reintegrated.receipt->spans[0].offset == 0x20U);
    assert(reintegrated.receipt->spans[0].size == child.resource.id.size);

    auto reopened_parent = parent;
    reopened_parent.bytes = reintegrated.bytes;
    const auto reopened_parent_parse = registry.parse(
        std::span<const std::byte>{
            reopened_parent.bytes.data(), reopened_parent.bytes.size()},
        reopened_parent.resource.id.logical_path);
    assert(reopened_parent_parse.ok());
    const auto reopened_expansion = gdspaces::ContainerExpander::expand(
        reopened_parent, reopened_parent_parse);
    assert(reopened_expansion.usable());
    assert(reopened_expansion.children.size() == 1U);
    const auto& reopened_child = reopened_expansion.children[0].payload;
    assert(reopened_child.bytes == written.bytes);

    const auto reopened_mod = mod::Parser::parse(
        std::span<const std::byte>{
            reopened_child.bytes.data(), reopened_child.bytes.size()});
    assert(reopened_mod.ok());
    assert(reopened_mod.document.outer_models[0].bounding_radius == 50.0F);

    // Continue the same validated authored-resource chain through the existing
    // next-volume NBZ overlay writer. This is a composition gate: no new MOD,
    // PAC/PNST, or NBZ serialization authority is introduced here.
    const std::vector<std::uint32_t> present_volumes{0U, 1U};
    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_volumes);
    assert(bootstrap.valid());
    assert(bootstrap.first_missing_index == 2U);
    assert(bootstrap.present_after_first_gap.empty());

    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = parent.resource.id.logical_path,
            .bytes = reintegrated.bytes,
        },
    };
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, members);
    assert(overlay.ok());
    assert(overlay.receipt->volume_index == 2U);
    assert(overlay.receipt->filename == "DMC3-2.nbz");
    assert(overlay.receipt->members.size() == 1U);
    assert(overlay.receipt->members[0].logical_path ==
           parent.resource.id.logical_path);

    const auto overlay_path = write_temp_nbz(
        std::span<const std::byte>{overlay.bytes.data(), overlay.bytes.size()});
    gdspaces::NbzZipSource reopened_nbz(
        "synthetic-mod-authored-overlay", overlay_path);
    assert(reopened_nbz.valid());
    assert(reopened_nbz.index_receipt().has_value());
    assert(reopened_nbz.index_receipt()->central_offset_matches);
    assert(reopened_nbz.index_receipt()->entry_count_matches);
    assert(reopened_nbz.index_receipt()->walked_entry_count == 1U);

    const auto overlay_refs = reopened_nbz.enumerate();
    assert(overlay_refs.size() == 1U);
    const auto* root_ref = find_path(
        overlay_refs, parent.resource.id.logical_path);
    assert(root_ref != nullptr);
    assert(root_ref->id.source_id == "synthetic-mod-authored-overlay");

    const auto overlay_parent = reopened_nbz.read(root_ref->id);
    assert(overlay_parent.has_value());
    assert(overlay_parent->readable());
    assert(overlay_parent->bytes == reintegrated.bytes);

    const auto overlay_parent_parse = registry.parse(
        std::span<const std::byte>{
            overlay_parent->bytes.data(), overlay_parent->bytes.size()},
        overlay_parent->resource.id.logical_path);
    assert(overlay_parent_parse.ok());
    const auto overlay_expansion = gdspaces::ContainerExpander::expand(
        *overlay_parent, overlay_parent_parse);
    assert(overlay_expansion.usable());
    assert(overlay_expansion.children.size() == 1U);
    const auto& overlay_child = overlay_expansion.children[0].payload;
    assert(overlay_child.bytes == written.bytes);

    const auto overlay_mod = mod::Parser::parse(
        std::span<const std::byte>{
            overlay_child.bytes.data(), overlay_child.bytes.size()});
    assert(overlay_mod.ok());
    assert(overlay_mod.document.outer_models[0].bounding_radius == 50.0F);

    std::error_code remove_error;
    std::filesystem::remove(overlay_path, remove_error);

    auto tampered = written;
    tampered.bytes[0x7CU] ^= std::byte{0x01U};
    const auto rejected = dmc3::ModAuthoredChildBridge::build(child, tampered);
    assert(!rejected.ok());
    assert(rejected.status == dmc3::ModAuthoredChildStatus::output_hash_mismatch);

    auto wrong_source = child;
    wrong_source.resource.container = true;
    const auto invalid_source =
        dmc3::ModAuthoredChildBridge::build(wrong_source, written);
    assert(!invalid_source.ok());
    assert(invalid_source.status ==
           dmc3::ModAuthoredChildStatus::invalid_source);

    return 0;
}
