#include "dmc_rengine/profiles/dmc3/stageops_ingress.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"
#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/semantic_graph.hpp"

#include "hits_test_fixture.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace dmc::rengine;

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size,
    bool container = false) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = "stageops-ingress-test",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = offset,
            .size = size,
        },
        .display_name = "stageops ingress resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = container,
    };
}

[[nodiscard]] gdspaces::ResourcePayload payload(
    gdspaces::ResourceRef reference,
    std::vector<std::byte> bytes) {
    return gdspaces::ResourcePayload{
        .resource = std::move(reference),
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = gdspaces::ByteProvenance{
            .kind = gdspaces::ByteOriginKind::direct_source_span,
            .authority_id = "stageops-ingress-source",
            .offset = 0U,
            .stored_size = 0U,
            .materialized_size = 0U,
            .transform = gdspaces::ByteTransform::none,
            .crc32 = std::nullopt,
        },
    };
}

void fix_provenance(gdspaces::ResourcePayload& value) {
    assert(value.byte_provenance.has_value());
    value.byte_provenance->offset = value.resource.id.offset;
    value.byte_provenance->stored_size =
        static_cast<std::uint64_t>(value.bytes.size());
    value.byte_provenance->materialized_size =
        static_cast<std::uint64_t>(value.bytes.size());
}

[[nodiscard]] profiles::dmc3::StageRuntimeLoadReport make_report() {
    using profiles::dmc3::RuntimeResolutionStatus;
    using profiles::dmc3::StageResourceReference;
    using profiles::dmc3::StageResourceRole;

    const std::array<gdspaces::ResourceRef, 4> roots{
        resource("scr/st308.pac", "pac", 0x1000U, 1U, true),
        resource("room/st308cfg.pac", "pac", 0x2000U, 1U),
        resource("room/st308_effect.pac", "pac", 0x3000U, 1U),
        resource("se/snd_r308.pac", "pac", 0x4000U, 1U),
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
        auto root_payload = payload(roots[index], {std::byte{0x01}});
        fix_provenance(root_payload);
        report.resources[index].payload = std::move(root_payload);
    }

    const auto hits_bytes = tests::hits_fixture::make_minimal_hits();
    const auto child_ref = resource(
        "scr/st308.pac::PAC/slot-0003/collision.hits",
        "hits",
        0x1010U,
        static_cast<std::uint64_t>(hits_bytes.size()));
    auto child_payload = payload(child_ref, hits_bytes);
    fix_provenance(child_payload);

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
                    .size = static_cast<std::uint64_t>(
                        child_payload.bytes.size()),
                    .logical_name = "collision.hits",
                    .populated = true,
                    .synthetic_name = false,
                },
                .payload = std::move(child_payload),
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
    assert(report.materialization_complete());

    integration::ProjectWorkspace project;
    const auto ingress = profiles::dmc3::StageOpsIngress::attach(
        std::move(report), project);
    assert(ingress.valid());
    assert(ingress.assembly.valid());
    assert(ingress.assembly.status() ==
        stageops::StageAssemblyStatus::product_materialized);
    assert(ingress.sessions_created == 5U);
    assert(ingress.sessions_reused == 0U);
    assert(project.session_count() == 5U);
    assert(ingress.analyses.size() == 1U);
    assert(ingress.analyses[0].ok());
    assert(ingress.analyses[0].byte_source ==
        integration::ParsedByteSource::immutable_source);
    assert(ingress.analyses[0].byte_revision == 0U);

    const auto hits_members = ingress.assembly.by_category(
        gdspaces::StageResourceCategory::collision);
    assert(hits_members.size() == 1U);
    const auto hits_id = hits_members[0]->resource_id;
    const auto* hits_session = project.find_session(hits_id);
    assert(hits_session != nullptr);
    assert(hits_session->parsed_resource() != nullptr);
    assert(hits_session->parsed_resource()->recognized());
    assert(hits_session->parsed_resource()->byte_source ==
        integration::ParsedByteSource::immutable_source);
    assert(hits_session->parsed_resource()->byte_revision == 0U);
    assert(hits_session->binary_document() != nullptr);

    stageops::StageOperationsSession operations(ingress.assembly, project);
    assert(operations.valid());
    assert(operations.stage_revision() == 0U);

    auto domains = stageops::StageDomainAssembler::assemble(
        ingress.assembly, project, operations.stage_revision());
    assert(domains.valid());
    assert(domains.by_kind(stageops::StageDomainKind::hits_collision).size() == 1U);
    assert(domains.stale_typed_result_count == 0U);

    auto semantic = stageops::semantic::StageSemanticGraphBuilder::build(
        ingress.assembly, domains, operations.stage_revision());
    assert(semantic.valid());
    assert(semantic.by_kind(
        stageops::semantic::NodeKind::domain_object).size() == 1U);
    const auto domain_id = domains.objects.front().id;
    const auto* domain_node = semantic.find_node(domain_id);
    assert(domain_node != nullptr);
    assert(domain_node->attributes.at("current_for_active_bytes") == "true");

    // Edit a semantic HITS record field rather than structural header offsets.
    // triangle.flags @ 0x50 may change while file/layout validity remains intact.
    assert(operations.enable_editing(hits_id));
    hits_session = project.find_session(hits_id);
    assert(hits_session != nullptr);
    const auto& source_bytes = hits_session->source_payload().bytes;
    const auto edit = operations.apply_edit(
        hits_id,
        gdspaces::EditOperation{
            .id = "vertical-stageops-edit",
            .base_revision = 0U,
            .offset = 0x50U,
            .expected = {source_bytes[0x50U]},
            .replacement = {std::byte{0x03}},
            .description = "Change HITS triangle flags without invalidating its structural layout.",
        });
    assert(edit.applied);
    assert(operations.stage_revision() == 1U);
    assert(operations.derived_state_stale());

    domains = stageops::StageDomainAssembler::assemble(
        ingress.assembly, project, operations.stage_revision());
    assert(domains.stale_typed_result_count == 1U);
    assert(!domains.current_for_active_bytes());
    assert(!domains.by_kind(stageops::StageDomainKind::hits_collision)[0]
        ->current_for_active_bytes);

    semantic = stageops::semantic::StageSemanticGraphBuilder::build(
        ingress.assembly, domains, operations.stage_revision());
    assert(semantic.valid());
    domain_node = semantic.find_node(domains.objects.front().id);
    assert(domain_node != nullptr);
    assert(domain_node->attributes.at("current_for_active_bytes") == "false");

    const auto refreshed_analysis = operations.refresh_resource_analysis();
    assert(refreshed_analysis.stage_revision == 1U);
    assert(refreshed_analysis.attempted == 1U);
    assert(refreshed_analysis.succeeded == 1U);
    assert(refreshed_analysis.skipped_without_parser == 4U);
    assert(refreshed_analysis.missing_project_session_count == 0U);
    assert(refreshed_analysis.complete_for_attempted());
    assert(refreshed_analysis.reports.size() == 1U);
    assert(refreshed_analysis.reports[0].byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(refreshed_analysis.reports[0].byte_revision == 1U);

    hits_session = project.find_session(hits_id);
    assert(hits_session != nullptr);
    assert(hits_session->parsed_resource() != nullptr);
    assert(hits_session->parsed_resource()->byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(hits_session->parsed_resource()->byte_revision == 1U);
    const auto* reparsed_hits = hits_session->parsed_resource()
        ->get_if<formats::hits::ScanResult>();
    assert(reparsed_hits != nullptr);
    assert(reparsed_hits->triangles.size() == 1U);
    assert(reparsed_hits->triangles[0].flags == 0x00000003U);

    domains = stageops::StageDomainAssembler::assemble(
        ingress.assembly, project, operations.stage_revision());
    assert(domains.stale_typed_result_count == 0U);
    assert(domains.current_for_active_bytes());
    assert(domains.by_kind(stageops::StageDomainKind::hits_collision)[0]
        ->current_for_active_bytes);

    semantic = stageops::semantic::StageSemanticGraphBuilder::build(
        ingress.assembly, domains, operations.stage_revision());
    assert(semantic.valid());
    domain_node = semantic.find_node(domains.objects.front().id);
    assert(domain_node != nullptr);
    assert(domain_node->attributes.at("byte_source") == "working-copy");
    assert(domain_node->attributes.at("byte_revision") == "1");
    assert(domain_node->attributes.at("current_for_active_bytes") == "true");

    assert(operations.commit_derived_refresh(1U));
    assert(!operations.derived_state_stale());

    const auto second = profiles::dmc3::StageOpsIngress::attach(
        make_report(), project);
    assert(second.valid());
    assert(second.sessions_created == 0U);
    assert(second.sessions_reused == 5U);
    assert(second.analyses.empty());
    assert(project.session_count() == 5U);
    hits_session = project.find_session(hits_id);
    assert(hits_session != nullptr);
    assert(hits_session->parsed_resource() != nullptr);
    assert(hits_session->parsed_resource()->byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(hits_session->parsed_resource()->byte_revision == 1U);

    integration::ProjectWorkspace conflicting_project;
    auto conflict_report = make_report();
    const auto conflict_root = *conflict_report.resources[1].payload;
    auto wrong_payload = conflict_root;
    wrong_payload.bytes[0] = std::byte{0x7F};
    assert(conflicting_project.create_session(
        std::move(wrong_payload),
        integration::WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        }));

    const auto conflict = profiles::dmc3::StageOpsIngress::attach(
        std::move(conflict_report), conflicting_project);
    assert(!conflict.valid());
    assert(std::any_of(
        conflict.diagnostics.begin(), conflict.diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.code ==
                "stageops.ingress.existing-session-payload-mismatch";
        }));

    return 0;
}
