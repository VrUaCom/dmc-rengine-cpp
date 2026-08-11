#include "dmc_rengine/gdspaces/byte_provenance.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::formats::ContainerParseResult parsed_child(
    std::uint64_t parent_size,
    std::uint64_t child_offset,
    std::uint64_t child_size) {
    using dmc::rengine::formats::ContainerDocument;
    using dmc::rengine::formats::ContainerEntry;
    using dmc::rengine::formats::ContainerParseResult;

    return ContainerParseResult{
        .recognized = true,
        .document = ContainerDocument{
            .format = "pac",
            .schema_version = 1U,
            .declared_slot_count = 1U,
            .container_size = parent_size,
            .entries = {
                ContainerEntry{
                    .slot_index = 0U,
                    .offset = child_offset,
                    .size = child_size,
                    .logical_name = "child.bin",
                    .populated = true,
                    .synthetic_name = true,
                },
            },
        },
        .diagnostics = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload parent_payload(
    std::uint64_t resource_offset,
    dmc::rengine::gdspaces::ByteProvenance provenance) {
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;

    std::vector<std::byte> bytes(160U, std::byte{0});
    return ResourcePayload{
        .resource = ResourceRef{
            .id = ResourceId{
                .source_id = "provenance-test",
                .logical_path = "DMC3/root.pac",
                .container_chain = {},
                .offset = resource_offset,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "root.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::move(provenance),
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ByteOriginKind;
    using dmc::rengine::gdspaces::ByteProvenance;
    using dmc::rengine::gdspaces::ByteTransform;
    using dmc::rengine::gdspaces::ContainerExpander;

    const ByteProvenance direct{
        .kind = ByteOriginKind::direct_source_span,
        .authority_id = "source-file",
        .offset = 1000U,
        .stored_size = 160U,
        .materialized_size = 160U,
        .transform = ByteTransform::none,
        .crc32 = std::nullopt,
    };
    assert(direct.valid());
    assert(direct.direct_byte_mapping());

    const ByteProvenance zip_stored{
        .kind = ByteOriginKind::direct_source_span,
        .authority_id = "DMC3-0.nbz",
        .offset = 50000U,
        .stored_size = 160U,
        .materialized_size = 160U,
        .transform = ByteTransform::zip_stored,
        .crc32 = 0x12345678U,
    };
    assert(zip_stored.valid());
    assert(zip_stored.direct_byte_mapping());

    const auto parsed = parsed_child(160U, 96U, 16U);
    const auto direct_expansion = ContainerExpander::expand(
        parent_payload(1000U, direct), parsed);
    assert(direct_expansion.usable());
    assert(direct_expansion.children.size() == 1U);
    const auto& direct_child = direct_expansion.children.front().payload;
    assert(direct_child.resource.id.offset == 1096U);
    assert(direct_child.byte_provenance.has_value());
    assert(direct_child.byte_provenance->valid());
    assert(direct_child.byte_provenance->kind ==
        ByteOriginKind::direct_source_span);
    assert(direct_child.byte_provenance->authority_id == "source-file");
    assert(direct_child.byte_provenance->offset == 1096U);
    assert(direct_child.byte_provenance->stored_size == 16U);
    assert(direct_child.byte_provenance->materialized_size == 16U);

    const auto zip_stored_expansion = ContainerExpander::expand(
        parent_payload(0U, zip_stored), parsed);
    assert(zip_stored_expansion.usable());
    assert(zip_stored_expansion.children.size() == 1U);
    const auto& zip_stored_child = zip_stored_expansion.children.front().payload;
    assert(zip_stored_child.resource.id.offset == 96U);
    assert(zip_stored_child.byte_provenance.has_value());
    assert(zip_stored_child.byte_provenance->valid());
    assert(zip_stored_child.byte_provenance->kind ==
        ByteOriginKind::direct_source_span);
    assert(zip_stored_child.byte_provenance->authority_id == "DMC3-0.nbz");
    assert(zip_stored_child.byte_provenance->offset == 50096U);

    const ByteProvenance deflated{
        .kind = ByteOriginKind::transformed_source_span,
        .authority_id = "DMC3-0.nbz",
        .offset = 50000U,
        .stored_size = 40U,
        .materialized_size = 160U,
        .transform = ByteTransform::zip_deflate,
        .crc32 = 0x12345678U,
    };
    assert(deflated.valid());
    assert(!deflated.direct_byte_mapping());

    const auto transformed_parent = parent_payload(0U, deflated);
    const auto transformed_parent_id = transformed_parent.resource.id.canonical();
    const auto transformed_expansion = ContainerExpander::expand(
        transformed_parent, parsed);
    assert(transformed_expansion.usable());
    assert(transformed_expansion.children.size() == 1U);
    const auto& transformed_child = transformed_expansion.children.front().payload;

    // Resource identity is relative to the materialized parent bytes. Storage
    // provenance must not fabricate 50000 + 96 as the child's archive offset.
    assert(transformed_child.resource.id.offset == 96U);
    assert(transformed_child.byte_provenance.has_value());
    assert(transformed_child.byte_provenance->valid());
    assert(transformed_child.byte_provenance->kind ==
        ByteOriginKind::materialized_parent_span);
    assert(transformed_child.byte_provenance->authority_id == transformed_parent_id);
    assert(transformed_child.byte_provenance->offset == 96U);
    assert(transformed_child.byte_provenance->offset != 50096U);
    assert(transformed_child.byte_provenance->stored_size == 16U);
    assert(transformed_child.byte_provenance->materialized_size == 16U);
    assert(transformed_child.byte_provenance->transform == ByteTransform::none);

    const ByteProvenance invalid_direct{
        .kind = ByteOriginKind::direct_source_span,
        .authority_id = "source-file",
        .offset = 0U,
        .stored_size = 40U,
        .materialized_size = 160U,
        .transform = ByteTransform::none,
        .crc32 = std::nullopt,
    };
    assert(!invalid_direct.valid());

    const ByteProvenance invalid_transform{
        .kind = ByteOriginKind::transformed_source_span,
        .authority_id = "DMC3-0.nbz",
        .offset = 50000U,
        .stored_size = 40U,
        .materialized_size = 160U,
        .transform = ByteTransform::none,
        .crc32 = std::nullopt,
    };
    assert(!invalid_transform.valid());

    return 0;
}
