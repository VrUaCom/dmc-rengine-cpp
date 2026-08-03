#include "custom_build_fixture.hpp"

#include "dmc_rengine/integration/project_workspace.hpp"

#include <algorithm>
#include <cassert>
#include <string>

int main() {
    using namespace dmc::rengine;
    using namespace tests::custom_build_fixture;
    using integration::ProjectEdgeKind;
    using integration::ProjectNodeKind;
    using integration::ProjectWorkspace;

    ProjectWorkspace workspace;
    prepare_workspace(workspace);

    const auto first = build_record();
    assert(first.valid());
    assert(workspace.register_custom_build_record(first));
    assert(!workspace.register_custom_build_record(first));
    assert(workspace.custom_build_count() == 1U);

    const auto* by_id = workspace.find_custom_build(
        "dmc3-custom-build-0001");
    const auto* by_hash = workspace.find_custom_build_by_sha256(
        first.identity.executable_sha256);
    assert(by_id != nullptr);
    assert(by_hash == by_id);
    assert(workspace.find_custom_build_by_sha256(hash('9')) == nullptr);

    const auto* mapping = workspace.find_custom_build_mapping(
        first.identity.custom_build_id, 0x1210U);
    assert(mapping != nullptr);
    assert(mapping->source_path ==
        "recovered/gameplay/item/inventory_policy.cpp");
    assert(mapping->source_line_begin == 10U);
    assert(mapping->source_line_end == 42U);
    assert(workspace.find_custom_build_mapping(
        first.identity.custom_build_id, 0x1300U) == nullptr);

    assert(workspace.graph().nodes(ProjectNodeKind::custom_build).size() == 1U);
    assert(workspace.graph().nodes(
        ProjectNodeKind::source_binary_mapping).size() == 1U);
    assert(workspace.graph().nodes(
        ProjectNodeKind::build_test_result).size() == 2U);
    assert(workspace.artifacts().by_sha256(
        first.identity.executable_sha256).size() == 1U);

    const auto build_node = "custom-build:" +
        first.identity.custom_build_id;
    bool built_from_project = false;
    bool based_on_baseline = false;
    bool references_original = false;
    bool produces_custom_exe = false;
    bool exe_editor_recognizes = false;
    bool includes_modification = false;
    bool mapping_targets_output = false;
    bool tested_by = false;
    for (const auto& edge : workspace.graph().all_edges()) {
        if (edge.from == build_node &&
            edge.to ==
                "source-integration-project:integration:custom-build-fixture" &&
            edge.kind == ProjectEdgeKind::built_from) {
            built_from_project = true;
        }
        if (edge.from == build_node &&
            edge.kind == ProjectEdgeKind::based_on) {
            based_on_baseline = true;
        }
        if (edge.from == build_node &&
            edge.to == "artifact:artifact-custom-build-original-exe" &&
            edge.kind == ProjectEdgeKind::references_artifact) {
            references_original = true;
        }
        if (edge.from == build_node &&
            edge.to ==
                "artifact:custom-executable:dmc3-custom-build-0001" &&
            edge.kind == ProjectEdgeKind::produces) {
            produces_custom_exe = true;
        }
        if (edge.from == "tool:exe-editor" &&
            edge.to == build_node &&
            edge.kind == ProjectEdgeKind::recognizes) {
            exe_editor_recognizes = true;
        }
        if (edge.from == build_node &&
            edge.to ==
                "source-modification:inventory-source-core@1.0.0" &&
            edge.kind == ProjectEdgeKind::includes) {
            includes_modification = true;
        }
        if (edge.from.find("source-binary-mapping:") == 0U &&
            edge.to ==
                "artifact:custom-executable:dmc3-custom-build-0001" &&
            edge.kind == ProjectEdgeKind::targets) {
            mapping_targets_output = true;
        }
        if (edge.from == build_node &&
            edge.kind == ProjectEdgeKind::tested_by) {
            tested_by = true;
        }
    }
    assert(built_from_project);
    assert(based_on_baseline);
    assert(references_original);
    assert(produces_custom_exe);
    assert(exe_editor_recognizes);
    assert(includes_modification);
    assert(mapping_targets_output);
    assert(tested_by);

    const auto second = build_record(
        "dmc3-custom-build-0002",
        '4',
        first.identity.custom_build_id);
    assert(second.valid());
    assert(workspace.register_custom_build_record(second));
    assert(workspace.custom_build_count() == 2U);
    bool lineage = false;
    for (const auto& edge : workspace.graph().all_edges()) {
        if (edge.from == "custom-build:dmc3-custom-build-0002" &&
            edge.to == "custom-build:dmc3-custom-build-0001" &&
            edge.kind == ProjectEdgeKind::lineage_from) {
            lineage = true;
        }
    }
    assert(lineage);

    auto duplicate_sha = build_record(
        "dmc3-custom-build-duplicate-sha",
        'c');
    assert(duplicate_sha.valid());
    assert(!workspace.register_custom_build_record(duplicate_sha));

    auto unknown_evidence = build_record(
        "dmc3-custom-build-unknown-evidence",
        '5');
    unknown_evidence.source_binary_mappings[0].evidence_record_ids = {
        "ev-not-loaded",
    };
    assert(unknown_evidence.valid());
    assert(!workspace.register_custom_build_record(unknown_evidence));

    auto missing_package_test = build_record(
        "dmc3-custom-build-missing-test",
        '6');
    missing_package_test.test_results.erase(
        missing_package_test.test_results.begin());
    assert(missing_package_test.valid());
    assert(!workspace.register_custom_build_record(missing_package_test));

    auto wrong_source_unit = build_record(
        "dmc3-custom-build-wrong-unit",
        '7');
    wrong_source_unit.source_binary_mappings[0].source_unit_id =
        "missing-unit";
    assert(wrong_source_unit.valid());
    assert(!workspace.register_custom_build_record(wrong_source_unit));

    auto missing_prior = build_record(
        "dmc3-custom-build-missing-prior",
        '8',
        "dmc3-custom-build-does-not-exist");
    assert(missing_prior.valid());
    assert(!workspace.register_custom_build_record(missing_prior));

    return 0;
}
