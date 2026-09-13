#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/exe/executable_report.hpp"

#include <cassert>
#include <cstdint>
#include <string>

namespace {

using dmc::rengine::core::json::Parser;
using dmc::rengine::exe::ExecutableArtifactIdentity;
using dmc::rengine::exe::ExecutableReportOptions;
using dmc::rengine::exe::PeDirectories;
using dmc::rengine::exe::PeExportedSymbol;
using dmc::rengine::exe::PeExportTable;
using dmc::rengine::exe::PeFunctionRange;
using dmc::rengine::exe::PeFunctionTable;
using dmc::rengine::exe::PeImage;
using dmc::rengine::exe::PeImportedFunction;
using dmc::rengine::exe::PeImportedModule;
using dmc::rengine::exe::PeKind;
using dmc::rengine::exe::PeMachine;
using dmc::rengine::exe::PeSection;
using dmc::rengine::exe::RttiClass;
using dmc::rengine::exe::RttiScanResult;
using dmc::rengine::exe::RttiVtable;
using dmc::rengine::exe::to_json;

struct Fixture final {
    ExecutableArtifactIdentity artifact;
    PeImage image;
    PeDirectories directories;
    RttiScanResult rtti;

    Fixture() {
        artifact.sha256 = "0123456789abcdef";
        artifact.size = 4096U;

        image.kind = PeKind::pe32_plus;
        image.machine = PeMachine::amd64;
        image.image_base = 0x140000000ULL;
        image.entry_point_rva = 0x1000U;
        image.size_of_image = 0x3000U;
        image.section_count = 1U;
        image.sections.push_back(PeSection{".text", 0x200U, 0x1000U, 0x200U, 0x400U, 0x60000020U});
        image.data_directories.resize(16U);
        image.data_directories[1] = dmc::rengine::exe::PeDataDirectory{0x2000U, 40U};

        PeImportedModule module;
        module.name = "ALPHA.dll";
        module.functions.push_back(PeImportedFunction{"alpha_one", 0x2060U, 7U, 0U, false});
        directories.imports.push_back(std::move(module));

        PeExportTable table;
        table.module_name = "synth.dll";
        table.ordinal_base = 1U;
        table.address_count = 1U;
        table.symbols.push_back(PeExportedSymbol{"synth_start", {}, 1U, 0x1040U});
        directories.exports = std::move(table);

        PeFunctionTable functions;
        functions.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0x2300U});
        functions.functions.push_back(PeFunctionRange{0x1040U, 0x1100U, 0x2310U});
        functions.smallest_size = 0x40U;
        functions.largest_size = 0xC0U;
        functions.covered_bytes = 0x100U;
        directories.functions = std::move(functions);

        RttiClass entry;
        entry.decorated_name = ".?AVDerived@@";
        entry.display_name = "Derived";
        entry.type_descriptor_rva = 0x2040U;
        entry.vtables.push_back(RttiVtable{0x2100U, 0x2148U, 2U, 0U, 0U});
        rtti.classes.push_back(std::move(entry));
        rtti.type_descriptors = 2U;
        rtti.locators = 1U;
        rtti.vtables_located = 1U;
    }
};

void the_report_is_valid_json() {
    const Fixture fixture;
    const auto report =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);

    assert(!report.empty());
    assert(report.front() == '{');

    const auto parsed = Parser::parse(report);
    assert(parsed.ok());
    assert(parsed.value->as_object() != nullptr);
}

void a_literal_member_stays_a_string() {
    const Fixture fixture;
    const auto report =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);

    // A string literal converts to bool before string_view, so without a
    // dedicated overload the schema member silently serialized as `true`.
    const auto parsed = Parser::parse(report);
    assert(parsed.ok());

    const auto* root = parsed.value->as_object();
    const auto schema = root->find("schema");
    assert(schema != root->end());
    assert(schema->second.as_string() != nullptr);
    assert(*schema->second.as_string() == "dmc-rengine.executable-report.v1");
}

void the_report_is_deterministic() {
    const Fixture fixture;
    const auto first =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);
    const auto second =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);
    assert(first == second);
}

void function_ranges_are_opt_in() {
    const Fixture fixture;

    const auto summary =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);
    assert(summary.find("\"size_histogram\"") != std::string::npos);
    assert(summary.find("\"ranges\"") == std::string::npos);

    ExecutableReportOptions options;
    options.include_function_ranges = true;
    const auto detailed =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti, options);
    assert(detailed.find("\"ranges\"") != std::string::npos);
    assert(Parser::parse(detailed).ok());
}

void optional_sections_can_be_suppressed() {
    const Fixture fixture;

    ExecutableReportOptions options;
    options.include_import_functions = false;
    options.include_rtti_classes = false;
    const auto trimmed =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti, options);

    assert(trimmed.find("alpha_one") == std::string::npos);
    assert(trimmed.find("class_graph") == std::string::npos);

    // The counts survive even when the detail is dropped.
    assert(trimmed.find("\"function_count\": 1") != std::string::npos);
    assert(trimmed.find("\"distinct_types\": 1") != std::string::npos);
    assert(Parser::parse(trimmed).ok());
}

void escaping_survives_a_windows_path() {
    Fixture fixture;
    dmc::rengine::exe::PeCodeViewIdentity identity;
    identity.age = 1U;
    identity.pdb_path = "C:\\dev\\\"quoted\"\\test.pdb";
    fixture.directories.codeview = identity;
    fixture.directories.debug_entries.push_back(
        dmc::rengine::exe::PeDebugEntry{2U, 46U, 0x2220U, 0x820U, 0U});

    const auto report =
        to_json(fixture.artifact, fixture.image, fixture.directories, fixture.rtti);
    const auto parsed = Parser::parse(report);
    assert(parsed.ok());

    const auto* root = parsed.value->as_object();
    const auto debug = root->find("debug");
    assert(debug != root->end());
    const auto* debug_object = debug->second.as_object();
    const auto codeview = debug_object->find("codeview");
    assert(codeview != debug_object->end());
    const auto* codeview_object = codeview->second.as_object();
    const auto path = codeview_object->find("pdb_path");
    assert(path != codeview_object->end());
    assert(*path->second.as_string() == "C:\\dev\\\"quoted\"\\test.pdb");
}

} // namespace

int main() {
    the_report_is_valid_json();
    a_literal_member_stays_a_string();
    the_report_is_deterministic();
    function_ranges_are_opt_in();
    optional_sections_can_be_suppressed();
    escaping_survives_a_windows_path();
    return 0;
}
