#include "custom_build_fixture.hpp"

#include "dmc_rengine/source/custom_build_import.hpp"
#include "dmc_rengine/source/custom_build_manifest.hpp"

#include <cassert>
#include <string>

namespace {

using dmc::rengine::source::CustomBuildImportResult;

[[nodiscard]] bool rejected_at(
    const CustomBuildImportResult& result,
    std::string_view path) {
    for (const auto& diagnostic : result.diagnostics) {
        if (diagnostic.path == path) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using namespace tests::custom_build_fixture;
    using integration::ProjectWorkspace;
    using source::custom_build_from_json;
    using source::custom_build_manifest_json;

    ProjectWorkspace workspace;
    prepare_workspace(workspace);
    const auto original = build_record();
    assert(workspace.register_custom_build_record(original));

    const auto manifest = custom_build_manifest_json(
        workspace, original.identity.custom_build_id);
    assert(!manifest.empty());

    // A manifest emitted by the canonical writer must import back. Without
    // that the export is a dead end: another tool could read a build but
    // never hand one back.
    const auto imported = custom_build_from_json(manifest);
    assert(imported.ok());
    const auto& record = *imported.record;

    assert(record.identity.custom_build_id == original.identity.custom_build_id);
    assert(record.identity.executable_sha256 ==
           original.identity.executable_sha256);
    assert(record.identity.original_exe_sha256 ==
           original.identity.original_exe_sha256);
    assert(record.identity.integration_project_id ==
           original.identity.integration_project_id);
    assert(record.status == original.status);
    assert(record.executable_size == original.executable_size);
    assert(record.toolchain.compiler_name == original.toolchain.compiler_name);
    assert(record.toolchain.compile_flags == original.toolchain.compile_flags);
    assert(record.pe_validation.image_base == original.pe_validation.image_base);
    assert(record.pe_validation.entry_point_rva ==
           original.pe_validation.entry_point_rva);
    assert(record.included_modifications.size() ==
           original.included_modifications.size());
    assert(record.source_binary_mappings.size() ==
           original.source_binary_mappings.size());
    assert(record.test_results.size() == original.test_results.size());
    assert(record.distribution_form == original.distribution_form);
    assert(record.attestation_state == original.attestation_state);
    assert(record.distribution_permission == original.distribution_permission);

    // The address triple is what the Source Map answers from, so it has to
    // survive exactly rather than approximately.
    for (const auto& mapping : record.source_binary_mappings) {
        const auto* match = original.mapping_for_rva(mapping.output_rva);
        assert(match != nullptr);
        assert(match->output_va == mapping.output_va);
        assert(match->output_file_offset == mapping.output_file_offset);
        assert(match->byte_size == mapping.byte_size);
        assert(match->recovered_symbol_name == mapping.recovered_symbol_name);
        assert(match->source_path == mapping.source_path);
        assert(match->confidence == mapping.confidence);
    }

    // An imported record must be registrable in a fresh workspace: importing
    // something the registry would refuse is a false success.
    ProjectWorkspace reopened;
    prepare_workspace(reopened);
    assert(reopened.register_custom_build_record(record));
    assert(reopened.find_custom_build_by_sha256(
               record.identity.executable_sha256) != nullptr);

    // Malformed input is refused with the path that failed, not defaulted.
    const auto not_json = custom_build_from_json("{");
    assert(!not_json.ok());
    assert(!not_json.record.has_value());

    const auto not_object = custom_build_from_json("[]");
    assert(!not_object.ok());
    assert(rejected_at(not_object, "$"));

    const auto empty_object = custom_build_from_json("{}");
    assert(!empty_object.ok());
    assert(rejected_at(empty_object, "identity"));

    // An unknown enum spelling is refused rather than silently defaulted to
    // the first value, which would downgrade a build's own claims.
    auto bad_status = manifest;
    const auto status_at = bad_status.find("\"status\": \"");
    assert(status_at != std::string::npos);
    bad_status.replace(
        status_at, std::string{"\"status\": \""}.size(),
        "\"status\": \"not-a-status\" , \"ignored\": \"");
    const auto unknown_status = custom_build_from_json(bad_status);
    assert(!unknown_status.ok());

    // A structurally complete record that the canonical rules reject must not
    // be handed back as if it imported.
    auto no_mappings = manifest;
    const auto mappings_at = no_mappings.find("\"source_binary_mappings\": [");
    assert(mappings_at != std::string::npos);
    const auto mappings_end = no_mappings.find("],", mappings_at);
    assert(mappings_end != std::string::npos);
    no_mappings.replace(
        mappings_at,
        mappings_end - mappings_at,
        "\"source_binary_mappings\": [");
    const auto invalid = custom_build_from_json(no_mappings);
    assert(!invalid.ok());
    assert(!invalid.record.has_value());

    return 0;
}
