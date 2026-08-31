#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"
#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;

template <typename Reconciler>
concept PublicResolverInjectionSurface = requires(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourcePayload* index,
    gdspaces::IndexProfileDisplayResolver resolver) {
    {
        Reconciler::reconcile(expansion, index, resolver)
    } -> std::same_as<gdspaces::ContainerNamingReconcileResult>;
};

template <typename Reconciler>
concept PublicProfiledReconcileSurface = requires(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourcePayload* index,
    gdspaces::IndexProfileDisplayResolver resolver) {
    {
        Reconciler::reconcile_profiled(expansion, index, resolver)
    } -> std::same_as<gdspaces::ContainerNamingReconcileResult>;
};

static_assert(
    !PublicResolverInjectionSurface<gdspaces::ContainerNamingReconciler>);
static_assert(
    !PublicProfiledReconcileSurface<gdspaces::ContainerNamingReconciler>);

[[nodiscard]] gdspaces::ContainerExpansion make_single_child_expansion(
    std::array<std::byte, 4> bytes,
    std::string logical_path) {
    namespace formats = dmc::rengine::formats;

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
                    .logical_path = std::move(logical_path),
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
            .bytes = {bytes.begin(), bytes.end()},
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
            .enclosing_container_name_evidence = {},
            .semantic_evidence = {},
        },
    });

    return expansion;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    using Contract = dmc3::ResourceTypeContract;

    // The canonical EXE contains two different byte classifiers. The registry
    // probe at 0x1402DB1F0 reads three bytes, while the family-mask classifier
    // at 0x1402FD650 reads four and requires ASCII space in byte 3.
    const std::array<std::byte, 4> modx{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{'X'}};
    assert(
        Contract::type_for_prefix(modx) == Contract::TypeCode::model);
    assert(
        Contract::family_mask_for_prefix(modx) == Contract::FamilyMask::unknown);

    const std::array<std::byte, 4> mod_space{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{' '}};
    assert(
        Contract::type_for_prefix(mod_space) == Contract::TypeCode::model);
    assert(
        Contract::family_mask_for_prefix(mod_space) == Contract::FamilyMask::model);

    const std::array<std::byte, 4> mcv_space{
        std::byte{'M'}, std::byte{'C'}, std::byte{'V'}, std::byte{' '}};
    assert(
        Contract::type_for_prefix(mcv_space) == Contract::TypeCode::unknown);
    assert(
        Contract::family_mask_for_prefix(mcv_space) ==
        Contract::FamilyMask::motion_curve);
    assert(
        Contract::canonical_extension(Contract::FamilyMask::motion_curve) == "mcv");

    // SCM is already recognized by the generic classifier. The DMC3 pipeline
    // must refine that compatible lower-precision claim to the recovered
    // three-byte runtime content-probe evidence.
    auto scm_expansion = make_single_child_expansion(
        {std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{' '}},
        "GData.afs/scr/st001.pac::PAC/slot-0000");

    const auto scm_result = dmc3::Dmc3NamingPipeline::apply(scm_expansion);
    assert(scm_result.ok());
    assert(scm_result.profile_semantics_applied);
    assert(scm_result.derived_display_names_applied);
    assert(scm_result.snapshot.has_value());
    assert(!scm_result.snapshot->external_index_evidence.has_value());
    assert(scm_result.snapshot->children.size() == 1U);

    const auto& scm_identity = scm_result.snapshot->children[0];
    assert(scm_identity.extracted_ordinal == 0U);
    assert(scm_identity.semantic_format == "scm");
    assert(scm_identity.canonical_extension == "scm");
    assert(scm_identity.semantic_format_evidence.has_value());
    assert(
        scm_identity.semantic_format_evidence->kind() ==
        gdspaces::ResourceSemanticEvidenceKind::profile_runtime_content_tag);
    assert(scm_identity.canonical_display_name == "st001_000.scm");
    assert(scm_expansion.children[0].payload.semantic_evidence.size() == 1U);

    const auto scm_classified = gdspaces::ResourceClassifier::classify(
        scm_expansion.children[0].payload,
        scm_identity.canonical_display_name);
    assert(scm_classified.format == "scm");
    assert(scm_classified.runtime_content_tag_confirmed);
    assert(!scm_classified.runtime_family_mask_confirmed);
    assert(!scm_classified.magic_confirmed);
    assert(!scm_classified.structural_confirmed);

    // MCV is the critical proof that the four-byte classifier is a separate
    // evidence path rather than a restatement of the three-byte registry probe.
    auto mcv_expansion = make_single_child_expansion(
        mcv_space,
        "GData.afs/scr/st001.pac::PAC/slot-0000");

    const auto mcv_result = dmc3::Dmc3NamingPipeline::apply(mcv_expansion);
    assert(mcv_result.ok());
    assert(mcv_result.profile_semantics_applied);
    assert(mcv_result.derived_display_names_applied);
    assert(mcv_result.snapshot.has_value());
    assert(mcv_result.snapshot->children.size() == 1U);

    const auto& mcv_identity = mcv_result.snapshot->children[0];
    assert(mcv_identity.semantic_format == "mcv");
    assert(mcv_identity.canonical_extension == "mcv");
    assert(mcv_identity.semantic_format_evidence.has_value());
    assert(
        mcv_identity.semantic_format_evidence->kind() ==
        gdspaces::ResourceSemanticEvidenceKind::profile_runtime_family_mask_tag);
    assert(mcv_identity.canonical_display_name == "st001_000.mcv");

    const auto mcv_classified = gdspaces::ResourceClassifier::classify(
        mcv_expansion.children[0].payload,
        mcv_identity.canonical_display_name);
    assert(mcv_classified.format == "mcv");
    assert(mcv_classified.runtime_family_mask_confirmed);
    assert(!mcv_classified.runtime_content_tag_confirmed);
    assert(!mcv_classified.magic_confirmed);
    assert(!mcv_classified.structural_confirmed);

    return 0;
}
