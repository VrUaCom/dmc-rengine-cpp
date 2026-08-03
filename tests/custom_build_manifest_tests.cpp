#include "custom_build_fixture.hpp"

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/source/custom_build_manifest.hpp"

#include <cassert>
#include <string_view>

namespace {

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using namespace tests::custom_build_fixture;
    using core::json::Parser;
    using integration::ProjectWorkspace;
    using source::custom_build_manifest_json;

    ProjectWorkspace workspace;
    prepare_workspace(workspace);
    const auto build = build_record();
    assert(workspace.register_custom_build_record(build));

    const auto json = custom_build_manifest_json(
        workspace,
        build.identity.custom_build_id);
    assert(!json.empty());
    assert(json == custom_build_manifest_json(
        workspace,
        build.identity.custom_build_id));

    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* terminology_value = member(*root, "terminology");
    assert(terminology_value != nullptr);
    const auto* terminology = terminology_value->as_object();
    assert(terminology != nullptr);
    const auto* source_artifact = member(
        *terminology, "source_artifact");
    const auto* binary_artifact = member(
        *terminology, "binary_artifact");
    const auto* plugin_model = member(
        *terminology, "runtime_plugin_model");
    assert(source_artifact != nullptr &&
           source_artifact->as_string() != nullptr);
    assert(*source_artifact->as_string() == "Composite Source Build");
    assert(binary_artifact != nullptr &&
           binary_artifact->as_string() != nullptr);
    assert(*binary_artifact->as_string() ==
        "Custom Recompiled Game EXE");
    assert(plugin_model != nullptr && plugin_model->as_bool() != nullptr);
    assert(!*plugin_model->as_bool());

    const auto* identity_value = member(*root, "identity");
    assert(identity_value != nullptr);
    const auto* identity = identity_value->as_object();
    assert(identity != nullptr);
    const auto* original_hash = member(*identity, "original_exe_sha256");
    const auto* output_hash = member(*identity, "executable_sha256");
    const auto* integration_id = member(
        *identity, "integration_project_id");
    assert(original_hash != nullptr &&
           original_hash->as_string() != nullptr);
    assert(*original_hash->as_string() == baseline().original_exe_sha256);
    assert(output_hash != nullptr && output_hash->as_string() != nullptr);
    assert(*output_hash->as_string() == build.identity.executable_sha256);
    assert(integration_id != nullptr &&
           integration_id->as_string() != nullptr);
    assert(*integration_id->as_string() ==
        "integration:custom-build-fixture");

    const auto* build_value = member(*root, "build");
    assert(build_value != nullptr);
    const auto* build_object = build_value->as_object();
    assert(build_object != nullptr);
    const auto* graph_registered = member(
        *build_object, "graph_registered");
    const auto* artifact_registered = member(
        *build_object, "artifact_registered");
    const auto* reopen = member(
        *build_object, "sha256_reopen_lookup");
    assert(graph_registered != nullptr &&
           graph_registered->as_bool() != nullptr &&
           *graph_registered->as_bool());
    assert(artifact_registered != nullptr &&
           artifact_registered->as_bool() != nullptr &&
           *artifact_registered->as_bool());
    assert(reopen != nullptr && reopen->as_bool() != nullptr &&
           *reopen->as_bool());

    const auto* toolchain_value = member(*root, "toolchain");
    assert(toolchain_value != nullptr);
    const auto* toolchain = toolchain_value->as_object();
    assert(toolchain != nullptr);
    const auto* compiler = member(*toolchain, "compiler_name");
    const auto* linker = member(*toolchain, "linker_name");
    const auto* compile_flags = member(*toolchain, "compile_flags");
    assert(compiler != nullptr && compiler->as_string() != nullptr);
    assert(*compiler->as_string() == "MSVC");
    assert(linker != nullptr && linker->as_string() != nullptr);
    assert(*linker->as_string() == "link.exe");
    assert(compile_flags != nullptr &&
           compile_flags->as_array() != nullptr &&
           compile_flags->as_array()->size() == 2U);

    const auto* pe_value = member(*root, "pe_validation");
    assert(pe_value != nullptr);
    const auto* pe = pe_value->as_object();
    assert(pe != nullptr);
    const auto* image_base = member(*pe, "image_base");
    const auto* machine = member(*pe, "machine");
    const auto* mappings_complete = member(
        *pe, "source_mapping_complete");
    assert(image_base != nullptr && image_base->as_u64() != nullptr);
    assert(*image_base->as_u64() == 0x140000000ULL);
    assert(machine != nullptr && machine->as_u64() != nullptr);
    assert(*machine->as_u64() == 0x8664U);
    assert(mappings_complete != nullptr &&
           mappings_complete->as_bool() != nullptr &&
           *mappings_complete->as_bool());

    const auto* modifications_value = member(
        *root, "included_modifications");
    assert(modifications_value != nullptr &&
           modifications_value->as_array() != nullptr);
    assert(modifications_value->as_array()->size() == 1U);

    const auto* mappings_value = member(
        *root, "source_binary_mappings");
    assert(mappings_value != nullptr &&
           mappings_value->as_array() != nullptr);
    assert(mappings_value->as_array()->size() == 1U);
    const auto* mapping = (*mappings_value->as_array())[0].as_object();
    assert(mapping != nullptr);
    const auto* source_path = member(*mapping, "source_path");
    const auto* output_rva = member(*mapping, "output_rva");
    const auto* output_va = member(*mapping, "output_va");
    const auto* line_begin = member(*mapping, "source_line_begin");
    assert(source_path != nullptr && source_path->as_string() != nullptr);
    assert(*source_path->as_string() ==
        "recovered/gameplay/item/inventory_policy.cpp");
    assert(output_rva != nullptr && output_rva->as_u64() != nullptr);
    assert(*output_rva->as_u64() == 0x1200U);
    assert(output_va != nullptr && output_va->as_u64() != nullptr);
    assert(*output_va->as_u64() == 0x140001200ULL);
    assert(line_begin != nullptr && line_begin->as_u64() != nullptr);
    assert(*line_begin->as_u64() == 10U);

    const auto* tests_value = member(*root, "test_results");
    assert(tests_value != nullptr && tests_value->as_array() != nullptr);
    assert(tests_value->as_array()->size() == 2U);

    const auto* distribution_value = member(*root, "distribution");
    assert(distribution_value != nullptr);
    const auto* distribution = distribution_value->as_object();
    assert(distribution != nullptr);
    const auto* form = member(*distribution, "form");
    const auto* attestation = member(
        *distribution, "attestation_state");
    const auto* rollback = member(*distribution, "rollback_build_id");
    assert(form != nullptr && form->as_string() != nullptr);
    assert(*form->as_string() == "private-test-build");
    assert(attestation != nullptr && attestation->as_string() != nullptr);
    assert(*attestation->as_string() == "hash-attested");
    assert(rollback != nullptr && rollback->as_string() != nullptr);
    assert(*rollback->as_string() == "baseline:source-tree-0001");

    assert(custom_build_manifest_json(workspace, "missing-build").empty());
    return 0;
}
