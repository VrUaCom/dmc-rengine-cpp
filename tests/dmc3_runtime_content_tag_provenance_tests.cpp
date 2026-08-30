#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace formats = dmc::rengine::formats;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "runtime-tag-provenance-test",
            .logical_path = "GData.afs/scr/st001.pac",
            .container_chain = "NBZ[7]",
            .offset = 0x1000U,
            .size = 0x200U,
        },
        .display_name = "st001.pac",
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
        .external_index_evidence = std::nullopt,
    };

    expansion.children.push_back(gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = 0U,
            .offset = 0x40U,
            .size = 4U,
            .logical_name = "slot_0000.bin",
            .populated = true,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = gdspaces::ResourceId{
                    .source_id = parent.id.source_id,
                    .logical_path = "GData.afs/scr/st001.pac::PAC/slot-0000",
                    .container_chain = "NBZ[7]/PAC[0]",
                    .offset = 0x1040U,
                    .size = 4U,
                },
                .display_name = "slot_0000.bin",
                .format = "unknown",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = {
                std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{' '},
            },
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
            .enclosing_container_name_evidence = {},
            .semantic_evidence = {},
        },
    });

    // The generic classifier already calls SCM magic-confirmed. The DMC3
    // naming pipeline must refine that generic observation to the stronger,
    // instruction-backed fact that the original runtime compares exactly the
    // same three-byte SCM tag. Keeping magic_confirmed_format here would lose
    // the recovered provenance even though the resulting format string agrees.
    const auto result = dmc3::Dmc3NamingPipeline::apply(expansion);
    assert(result.ok());
    assert(result.profile_semantics_applied);
    assert(result.derived_display_names_applied);
    assert(result.snapshot.has_value());
    assert(!result.snapshot->external_index_evidence.has_value());
    assert(result.snapshot->children.size() == 1U);

    const auto& identity = result.snapshot->children[0];
    assert(identity.extracted_ordinal == 0U);
    assert(identity.semantic_format == "scm");
    assert(identity.canonical_extension == "scm");
    assert(identity.semantic_format_evidence.has_value());
    assert(
        identity.semantic_format_evidence->kind() ==
        gdspaces::ResourceSemanticEvidenceKind::profile_runtime_content_tag);
    assert(identity.canonical_display_name == "st001_000.scm");

    // The refinement must replace, not accumulate beside, the generic magic
    // record; one physical byte image should expose one active semantic reason.
    assert(expansion.children[0].payload.semantic_evidence.size() == 1U);
    assert(
        expansion.children[0].payload.semantic_evidence[0].kind() ==
        gdspaces::ResourceSemanticEvidenceKind::profile_runtime_content_tag);
    assert(expansion.children[0].payload.bytes[0] == std::byte{'S'});
    assert(expansion.children[0].payload.resource.id == identity.resource_id);

    return 0;
}
