#include "dmc_rengine/stageops/dmc3_stage_assembler.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace {

using namespace dmc::rengine;

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    bool container = false) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = "stageops-assembly-test",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = offset,
            .size = 1U,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = container,
    };
}

[[nodiscard]] gdspaces::ResourcePayload payload(
    gdspaces::ResourceRef reference) {
    return gdspaces::ResourcePayload{
        .resource = std::move(reference),
        .bytes = {std::byte{0x01}},
        .diagnostics = {},
        .byte_provenance = gdspaces::ByteProvenance{
            .kind = gdspaces::ByteOriginKind::direct_source_span,
            .authority_id = "fixture-source",
            .offset = 0U,
            .stored_size = 1U,
            .materialized_size = 1U,
            .transform = gdspaces::ByteTransform::none,
            .crc32 = std::nullopt,
        },
    };
}

[[nodiscard]] profiles::dmc3::StageRuntimeLoadReport make_report() {
    using profiles::dmc3::RuntimeResolutionStatus;
    using profiles::dmc3::StageResourceReference;
    using profiles::dmc3::StageResourceRole;

    const std::array<gdspaces::ResourceRef, 4> roots{
        resource("scr/st308.pac", "pac", 0x1000U, true),
        resource("room/st308cfg.pac", "pac", 0x2000U),
        resource("room/st308_effect.pac", "pac", 0x3000U),
        resource("se/snd_r308.pac", "pac", 0x4000U),
    };

    profiles::dmc3::StageRuntimeLoadReport report;
    report.resolution.catalog_entry_id = "dmc3-stage-row-110";
    report.resolution.table_row_index = 110U;
    report.resolution.source_table_id = "dmc3-stage-wave2-bank-b";
    report.resolution.source_row_index = 0U;
    report.resolution.numeric_stage_id = 308U;
    report.resolution.semantic_stage_id = std::nullopt;
    report.resolution.plan.stage_id = report.resolution.catalog_entry_id;
    report.resolution.plan.resource_set_id = report.resolution.catalog_entry_id;
    report.resolution.plan.evidence_id = "dmc3-stage-wave2";
    report.resolution.plan.numeric_stage_id = 308U;

    constexpr std::array<StageResourceRole, 4> roles{
        StageResourceRole::script,
        StageResourceRole::room_config,
        StageResourceRole::room_effects,
        StageResourceRole::room_sound,
    };

    for (std::size_t index = 0U; index < roots.size(); ++index) {
        const StageResourceReference reference{
            .role = roles[index],
            .logical_path = roots[index].id.logical_path,
            .kind16 = 1U,
        };
        report.resolution.plan.resources[index] = reference;
        report.resolution.resources[index] =
            profiles::dmc3::StageRuntimeResourceResolution{
                .reference = reference,
                .runtime = profiles::dmc3::RuntimeResolutionReport{
                    .request = reference.logical_path,
                    .status = RuntimeResolutionStatus::resolved,
                    .resolved = roots[index],
                    .ambiguous_matches = {},
                    .probes = {},
                    .detail = "fixture",
                },
                .list_fallback = std::nullopt,
            };
        report.resources[index].resolution = report.resolution.resources[index];
        report.resources[index].payload = payload(roots[index]);
    }

    // First root is a container with one materialized child. Stage Ops must keep
    // the child-parent-slot relationship without promoting gameplay semantics.
    const auto child_ref = resource(
        "scr/st308.pac::PAC/slot-0003/collision.hits",
        "hits",
        0x1010U);
    gdspaces::ContainerTreeExpansion tree;
    tree.fully_expanded = true;
    tree.expanded_container_count = 1U;
    tree.parsed_container_bytes = 1U;
    tree.expansions.push_back(gdspaces::ContainerExpansion{
        .parent = roots[0],
        .parser_format = "PAC",
        .children = {
            gdspaces::ContainerChild{
                .entry = formats::ContainerEntry{
                    .slot_index = 3U,
                    .offset = 0U,
                    .size = 1U,
                    .logical_name = "collision.hits",
                    .populated = true,
                    .synthetic_name = false,
                },
                .payload = payload(child_ref),
            },
        },
        .diagnostics = {},
    });
    report.resources[0].expansion = std::move(tree);

    report.bundle.emplace(gdspaces::StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = report.resolution.catalog_entry_id,
        .display_name = "Stage resource set dmc3-stage-row-110",
        .exe_evidence_id = "dmc3-stage-wave2",
        .resource_set_id = report.resolution.catalog_entry_id,
        .semantic_stage_id = {},
        .numeric_stage_id = 308U,
    });

    return report;
}

} // namespace

int main() {
    using namespace dmc::rengine;

    auto report = make_report();
    assert(report.resolution.complete());
    assert(report.materialization_complete());
    assert(!report.game_ready_equivalent());

    const auto assembled = stageops::DMC3StageAssembler::assemble(report);
    assert(assembled.valid());
    assert(assembled.status() == stageops::StageAssemblyStatus::product_materialized);
    assert(assembled.product_ready());
    assert(!assembled.original_game_ready());
    assert(!assembled.game_ready_equivalent);

    assert(assembled.identity.catalog_entry_id == "dmc3-stage-row-110");
    assert(assembled.identity.global_catalog_row == 110U);
    assert(assembled.identity.source_table_id == "dmc3-stage-wave2-bank-b");
    assert(assembled.identity.source_row_index == 0U);
    assert(assembled.identity.numeric_stage_id == 308U);
    assert(!assembled.identity.semantic_stage_id.has_value());

    assert(assembled.descriptor_root_count() == 4U);
    assert(assembled.nested_resource_count() == 1U);
    assert(assembled.resources.size() == 5U);

    const auto child_id =
        report.resources[0].expansion->expansions[0].children[0]
            .payload.resource.id.canonical();
    const auto parent_id = report.resources[0].payload->resource.id.canonical();
    const auto* child = assembled.find_resource(child_id);
    assert(child != nullptr);
    assert(child->materialized);
    assert(child->nested_container_child);
    assert(!child->descriptor_root);
    assert(!child->game_ready);

    const auto child_memberships = assembled.memberships_for(child_id);
    assert(child_memberships.size() == 1U);
    assert(child_memberships[0]->kind ==
        stageops::StageAssemblyMembershipKind::nested_container_child);
    assert(child_memberships[0]->parent_resource_id == parent_id);
    assert(child_memberships[0]->container_slot == 3U);
    assert(child_memberships[0]->category ==
        gdspaces::StageResourceCategory::collision);

    assert(assembled.by_category(gdspaces::StageResourceCategory::scripts).size() == 1U);
    assert(assembled.by_category(gdspaces::StageResourceCategory::effects).size() == 1U);
    assert(assembled.by_category(gdspaces::StageResourceCategory::sounds).size() == 1U);
    assert(assembled.by_category(gdspaces::StageResourceCategory::collision).size() == 1U);

    // Partial materialization remains a valid operational Stage Ops workspace,
    // but it cannot cross either product-complete or original-game-ready gates.
    auto partial_report = make_report();
    partial_report.resources[2].payload.reset();
    const auto partial = stageops::DMC3StageAssembler::assemble(partial_report);
    assert(partial.valid());
    assert(partial.status() == stageops::StageAssemblyStatus::partial);
    assert(!partial.product_ready());
    assert(!partial.original_game_ready());
    assert(partial.descriptor_root_count() == 4U);

    const auto missing_root_id =
        *partial_report.resources[2].resolution.runtime.resolved;
    const auto* missing_root = partial.find_resource(
        missing_root_id.id.canonical());
    assert(missing_root != nullptr);
    assert(!missing_root->materialized);
    assert(missing_root->descriptor_root);

    return 0;
}
