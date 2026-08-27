#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char character : text) {
        bytes.push_back(static_cast<std::byte>(
            static_cast<unsigned char>(character)));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> nested_pnst() {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    bytes[0x10U] = std::byte{'D'};
    bytes[0x11U] = std::byte{'D'};
    bytes[0x12U] = std::byte{'S'};
    bytes[0x13U] = std::byte{' '};
    return bytes;
}

[[nodiscard]] std::vector<std::byte> outer_pac() {
    const auto nested = nested_pnst();
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x20U);
    put_u32(bytes, 16U, 0x60U);
    std::copy(
        nested.begin(), nested.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(0x20U));
    bytes[0x60U] = std::byte{'D'};
    bytes[0x61U] = std::byte{'D'};
    bytes[0x62U] = std::byte{'S'};
    bytes[0x63U] = std::byte{' '};
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload parent_payload() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = outer_pac();
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-nbz-source",
                .logical_path = "GData.afs/outer.pac",
                .container_chain = "nbz[42]",
                .offset = 0x1000U,
                .size = size,
            },
            .display_name = "outer.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = gdspaces::ByteProvenance{
            .kind = gdspaces::ByteOriginKind::transformed_source_span,
            .authority_id = "retail-nbz-source",
            .offset = 0x777U,
            .stored_size = 0x50U,
            .materialized_size = size,
            .transform = gdspaces::ByteTransform::zip_deflate,
            .crc32 = 0x12345678U,
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload index_payload(
    std::string_view text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-index-source",
                .logical_path = "GData.afs/outer.index",
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "outer.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::EditOperation edit_byte(
    std::uint64_t offset,
    std::byte expected,
    std::byte replacement) {
    return dmc::rengine::gdspaces::EditOperation{
        .id = "index-name-reopen-edit",
        .base_revision = 0U,
        .offset = offset,
        .expected = {expected},
        .replacement = {replacement},
        .description = "Index name authority must not redirect physical nested edit.",
    };
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredChildImage author_child(
    const dmc::rengine::gdspaces::ResourcePayload& child,
    std::byte expected,
    std::byte replacement) {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    gdspaces::WorkingCopy working{child};
    const auto edited = working.apply(edit_byte(
        0x20U, expected, replacement));
    assert(edited.applied);
    const auto rebuilt = dmc3::RelativeSlotLayoutWriter::rebuild(
        child, working);
    assert(rebuilt.ok());
    return dmc3::AuthoredChildImage{
        .resource = child.resource.id,
        .source_sha256 = rebuilt.receipt->source_sha256,
        .output_sha256 = rebuilt.receipt->output_sha256,
        .revision = rebuilt.receipt->working_revision,
        .writer_mode = "layout-preserving-packed",
        .bytes = rebuilt.bytes,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion parse_expand(
    const dmc::rengine::gdspaces::ResourcePayload& parent) {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parsed.ok());
    auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    assert(expansion.usable());
    assert(expansion.children.size() == 3U);
    return expansion;
}

void apply_index(
    dmc::rengine::gdspaces::ContainerExpansion& expansion,
    const dmc::rengine::gdspaces::ResourcePayload& index) {
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto manifest = gdspaces::IndexManifestParser::parse(index);
    assert(manifest.ok());
    const auto binding = gdspaces::IndexSlotNameBinder::bind(
        expansion, *manifest.manifest);
    assert(binding.ok());
    const auto overlay = gdspaces::IndexNameOverlayBuilder::build(
        expansion, *binding.binding);
    assert(overlay.ok());
    const auto applied = gdspaces::IndexNameOverlayBuilder::apply(
        expansion, *overlay.overlay);
    assert(applied.ok());
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    auto parent = parent_payload();
    auto expansion = parse_expand(parent);
    const auto physical_id_before = expansion.children[0].payload.resource.id;
    const auto alias_id_before = expansion.children[1].payload.resource.id;
    assert(physical_id_before != alias_id_before);
    assert(
        physical_id_before.logical_path ==
        "GData.afs/outer.pac::PAC/slot-0000");
    assert(
        alias_id_before.logical_path ==
        "GData.afs/outer.pac::PAC/slot-0001");

    const auto original_index = index_payload(
        "nested_primary.pac\n"
        "nested_alias.pac\n"
        "tail.dds\n");
    apply_index(expansion, original_index);
    assert(
        expansion.children[0].payload.resource.display_name ==
        "nested_primary.pnst");
    assert(
        expansion.children[1].payload.resource.display_name ==
        "nested_alias.pnst");
    assert(expansion.children[0].payload.resource.id == physical_id_before);
    assert(expansion.children[1].payload.resource.id == alias_id_before);

    // First physical edit and parent reintegration.
    const auto authored = author_child(
        expansion.children[0].payload,
        std::byte{0},
        std::byte{0x5A});
    const std::vector<dmc3::AuthoredChildImage> authored_set{authored};
    const auto reintegrated = dmc3::NestedRelativeSlotReintegrator::reintegrate(
        parent, expansion, authored_set);
    assert(reintegrated.ok());
    assert(reintegrated.bytes[0x40U] == std::byte{0x5A});

    // Reopen the rebuilt parent. Physical slot identity must be the same.
    auto reopened_parent = parent;
    reopened_parent.bytes = reintegrated.bytes;
    auto reopened = parse_expand(reopened_parent);
    assert(reopened.children[0].payload.resource.id == physical_id_before);
    assert(reopened.children[1].payload.resource.id == alias_id_before);
    assert(reopened.children[0].payload.bytes[0x20U] == std::byte{0x5A});

    // The same sealed index observation can be re-applied after reopen without
    // altering physical identity or bytes.
    const auto reopened_bytes_before = reopened.children[0].payload.bytes;
    apply_index(reopened, original_index);
    assert(reopened.children[0].payload.resource.id == physical_id_before);
    assert(reopened.children[0].payload.bytes == reopened_bytes_before);
    assert(
        reopened.children[0].payload.resource.display_name ==
        "nested_primary.pnst");

    // Change only the .index labels. The second edit must still target the same
    // physical child after the rename and produce the expected byte transition.
    const auto renamed_index = index_payload(
        "renamed_only.pac\n"
        "renamed_alias.pac\n"
        "renamed_tail.dds\n");
    const auto id_before_rename = reopened.children[0].payload.resource.id;
    const auto bytes_before_rename = reopened.children[0].payload.bytes;
    apply_index(reopened, renamed_index);
    assert(
        reopened.children[0].payload.resource.display_name ==
        "renamed_only.pnst");
    assert(reopened.children[0].payload.resource.id == id_before_rename);
    assert(reopened.children[0].payload.bytes == bytes_before_rename);

    const auto second_authored = author_child(
        reopened.children[0].payload,
        std::byte{0x5A},
        std::byte{0x66});
    const std::vector<dmc3::AuthoredChildImage> second_set{second_authored};
    const auto second_reintegrated =
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            reopened_parent, reopened, second_set);
    assert(second_reintegrated.ok());
    assert(second_reintegrated.bytes[0x40U] == std::byte{0x66});

    auto second_parent = reopened_parent;
    second_parent.bytes = second_reintegrated.bytes;
    const auto second_reopen = parse_expand(second_parent);
    assert(second_reopen.children[0].payload.resource.id == physical_id_before);
    assert(second_reopen.children[0].payload.bytes[0x20U] == std::byte{0x66});

    return 0;
}
