#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/gdspaces/resource_name_evidence.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload index_payload(
    std::string_view text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "index-source",
                .logical_path = "root.index",
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "root.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion expansion_fixture() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "container-source",
            .logical_path = "root.pac",
            .container_chain = {},
            .offset = 0U,
            .size = 0x40U,
        },
        .display_name = "root.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };

    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
    };

    const gdspaces::ResourceId child_id{
        .source_id = parent.id.source_id,
        .logical_path = "root.pac::PAC/slot-0000",
        .container_chain = "PAC[0]",
        .offset = 0x20U,
        .size = 4U,
    };
    expansion.children.push_back(gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = 0U,
            .offset = 0x20U,
            .size = 4U,
            .logical_name = "slot_0000.bin",
            .populated = true,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = child_id,
                .display_name = "slot_0000.bin",
                .format = "hits",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = {
                std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}},
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
        },
    });
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

    auto expansion = expansion_fixture();
    assert(expansion.usable());
    const auto child_id = expansion.children[0].payload.resource.id;
    const auto child_bytes = expansion.children[0].payload.bytes;

    const auto first_index = index_payload("external_name.ukn\n");
    apply_index(expansion, first_index);
    const auto& after_first = expansion.children[0].payload;
    assert(after_first.resource.id == child_id);
    assert(after_first.bytes == child_bytes);
    assert(after_first.resource.display_name == "external_name.hits");
    assert(after_first.name_evidence.size() == 1U);

    const auto& first_external = after_first.name_evidence.front();
    assert(first_external.valid());
    assert(
        first_external.kind() ==
        gdspaces::ResourceNameEvidenceKind::external_index);
    assert(
        first_external.mapping_mode() ==
        gdspaces::ResourceNameMappingMode::physical_position);
    assert(first_external.authority_resource() == first_index.resource.id);
    assert(first_external.authority_sha256().size() == 64U);
    assert(first_external.raw_label() == "external_name.ukn");
    assert(first_external.normalized_name() == "external_name.ukn");
    assert(first_external.physical_slot_index() == 0U);
    assert(first_external.source_line().has_value());
    assert(*first_external.source_line() == 1U);
    assert(!first_external.source_offset().has_value());
    const auto first_hash = std::string{first_external.authority_sha256()};

    // Replacing the active external manifest must replace, not accumulate,
    // external-index evidence. Identity and bytes remain physical authority.
    const auto renamed_index = index_payload("renamed_only folder\n");
    apply_index(expansion, renamed_index);
    const auto& after_rename = expansion.children[0].payload;
    assert(after_rename.resource.id == child_id);
    assert(after_rename.bytes == child_bytes);
    assert(after_rename.resource.display_name == "renamed_only.hits");
    assert(after_rename.name_evidence.size() == 1U);

    const auto& renamed_external = after_rename.name_evidence.front();
    assert(renamed_external.valid());
    assert(
        renamed_external.kind() ==
        gdspaces::ResourceNameEvidenceKind::external_index);
    assert(renamed_external.authority_resource() == renamed_index.resource.id);
    assert(renamed_external.raw_label() == "renamed_only folder");
    assert(renamed_external.normalized_name() == "renamed_only");
    assert(renamed_external.authority_sha256() != first_hash);
    assert(renamed_external.physical_slot_index() == 0U);
    assert(renamed_external.source_line().has_value());
    assert(*renamed_external.source_line() == 1U);

    // ResourceNameEvidence cannot be aggregate-constructed by arbitrary callers;
    // only the sealed index overlay builder (and a future dedicated embedded-name
    // evidence builder) can create it. This compile-time boundary is intentional.

    return 0;
}
