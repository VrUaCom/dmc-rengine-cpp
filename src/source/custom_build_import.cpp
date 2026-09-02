#include "dmc_rengine/source/custom_build_import.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <utility>

namespace dmc::rengine::source {
namespace {

using core::json::Value;

class Reader final {
public:
    explicit Reader(std::vector<CustomBuildImportDiagnostic>& diagnostics)
        : diagnostics_(diagnostics) {}

    void reject(std::string path, std::string message) {
        diagnostics_.push_back(CustomBuildImportDiagnostic{
            .path = std::move(path),
            .message = std::move(message),
        });
    }

    [[nodiscard]] const Value* member(
        const Value::Object& object,
        std::string_view name) const {
        const auto found = object.find(name);
        return found == object.end() ? nullptr : &found->second;
    }

    /** A missing member is reported rather than defaulted. */
    [[nodiscard]] const Value::Object* object_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path) {
        const auto* value = member(parent, name);
        if (value == nullptr) {
            reject(std::string{path}, "Required object is missing.");
            return nullptr;
        }
        const auto* object = value->as_object();
        if (object == nullptr) {
            reject(std::string{path}, "Expected an object.");
        }
        return object;
    }

    [[nodiscard]] std::string string_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path,
        bool required = true) {
        const auto* value = member(parent, name);
        if (value == nullptr) {
            if (required) {
                reject(std::string{path}, "Required string is missing.");
            }
            return {};
        }
        const auto* text = value->as_string();
        if (text == nullptr) {
            reject(std::string{path}, "Expected a string.");
            return {};
        }
        return *text;
    }

    [[nodiscard]] std::uint64_t unsigned_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path,
        bool required = true) {
        const auto* value = member(parent, name);
        if (value == nullptr) {
            if (required) {
                reject(std::string{path}, "Required number is missing.");
            }
            return 0U;
        }
        if (const auto* unsigned_value = value->as_u64()) {
            return *unsigned_value;
        }
        // A negative or fractional value is refused rather than clamped: an
        // address or a size silently turned into zero would place a mapping
        // somewhere the build never put it.
        if (const auto* signed_value = value->as_i64()) {
            if (*signed_value >= 0) {
                return static_cast<std::uint64_t>(*signed_value);
            }
        }
        reject(std::string{path}, "Expected a non-negative integer.");
        return 0U;
    }

    [[nodiscard]] bool bool_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path) {
        const auto* value = member(parent, name);
        if (value == nullptr) {
            reject(std::string{path}, "Required boolean is missing.");
            return false;
        }
        const auto* flag = value->as_bool();
        if (flag == nullptr) {
            reject(std::string{path}, "Expected a boolean.");
            return false;
        }
        return *flag;
    }

    [[nodiscard]] std::vector<std::string> string_array_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path,
        std::size_t limit) {
        std::vector<std::string> values;
        const auto* value = member(parent, name);
        if (value == nullptr) {
            reject(std::string{path}, "Required array is missing.");
            return values;
        }
        const auto* array = value->as_array();
        if (array == nullptr) {
            reject(std::string{path}, "Expected an array.");
            return values;
        }
        if (array->size() > limit) {
            reject(std::string{path}, "Array exceeds the configured limit.");
            return values;
        }
        values.reserve(array->size());
        for (std::size_t index = 0U; index < array->size(); ++index) {
            const auto* text = (*array)[index].as_string();
            if (text == nullptr) {
                reject(
                    std::string{path} + "[" + std::to_string(index) + "]",
                    "Expected a string.");
                return {};
            }
            values.push_back(*text);
        }
        return values;
    }

    [[nodiscard]] const Value::Array* array_at(
        const Value::Object& parent,
        std::string_view name,
        std::string_view path,
        std::size_t limit) {
        const auto* value = member(parent, name);
        if (value == nullptr) {
            reject(std::string{path}, "Required array is missing.");
            return nullptr;
        }
        const auto* array = value->as_array();
        if (array == nullptr) {
            reject(std::string{path}, "Expected an array.");
            return nullptr;
        }
        if (array->size() > limit) {
            reject(std::string{path}, "Array exceeds the configured limit.");
            return nullptr;
        }
        return array;
    }

private:
    std::vector<CustomBuildImportDiagnostic>& diagnostics_;
};

/**
 * Maps a canonical enum spelling back to its value.
 *
 * An unknown spelling is refused rather than defaulted. Defaulting a status or
 * an attestation state would silently downgrade a build's claims to whatever
 * the enum happens to list first.
 */
template <typename Enum, std::size_t Count>
[[nodiscard]] bool parse_enum(
    const std::array<Enum, Count>& values,
    std::string_view text,
    Enum& out) {
    for (const auto candidate : values) {
        if (to_string(candidate) == text) {
            out = candidate;
            return true;
        }
    }
    return false;
}

constexpr std::array kStatuses{
    CustomBuildStatus::compiled,
    CustomBuildStatus::structurally_validated,
    CustomBuildStatus::runtime_tested,
    CustomBuildStatus::release_candidate,
    CustomBuildStatus::approved,
    CustomBuildStatus::released,
    CustomBuildStatus::revoked,
    CustomBuildStatus::superseded,
};

constexpr std::array kForms{
    DistributionForm::private_test_build,
    DistributionForm::full_custom_executable,
    DistributionForm::binary_delta,
    DistributionForm::source_modification_package,
    DistributionForm::local_reconstruction_installer,
};

constexpr std::array kAttestations{
    AttestationState::none,
    AttestationState::unsigned_internal,
    AttestationState::hash_attested,
    AttestationState::signed_build,
    AttestationState::revoked,
};

constexpr std::array kConfidences{
    SourceBinaryMappingConfidence::confirmed,
    SourceBinaryMappingConfidence::high,
    SourceBinaryMappingConfidence::candidate,
    SourceBinaryMappingConfidence::research_required,
};

constexpr std::array kLayers{
    TestLayer::unit,
    TestLayer::integration,
    TestLayer::compile_link,
    TestLayer::pe_structure,
    TestLayer::abi_symbol,
    TestLayer::save_load,
    TestLayer::stage_resource_loading,
    TestLayer::performance,
    TestLayer::long_running_stability,
    TestLayer::compatibility,
    TestLayer::install_update_rollback,
    TestLayer::runtime_smoke,
};

void read_identity(
    Reader& reader,
    const Value::Object& root,
    CustomBuildIdentity& identity) {
    const auto* object = reader.object_at(root, "identity", "identity");
    if (object == nullptr) {
        return;
    }
    identity.custom_build_id =
        reader.string_at(*object, "custom_build_id", "identity.custom_build_id");
    identity.edition_id =
        reader.string_at(*object, "edition_id", "identity.edition_id", false);
    identity.semantic_version = reader.string_at(
        *object, "semantic_version", "identity.semantic_version");
    identity.integration_project_id = reader.string_at(
        *object, "integration_project_id", "identity.integration_project_id");
    identity.game_profile =
        reader.string_at(*object, "game_profile", "identity.game_profile");
    identity.original_exe_sha256 = reader.string_at(
        *object, "original_exe_sha256", "identity.original_exe_sha256");
    identity.source_schema_version = reader.string_at(
        *object, "source_schema_version", "identity.source_schema_version");
    identity.source_revision =
        reader.string_at(*object, "source_revision", "identity.source_revision");
    identity.source_tree_sha256 = reader.string_at(
        *object, "source_tree_sha256", "identity.source_tree_sha256");
    identity.executable_sha256 = reader.string_at(
        *object, "executable_sha256", "identity.executable_sha256");
    identity.prior_custom_build_id = reader.string_at(
        *object, "prior_custom_build_id", "identity.prior_custom_build_id", false);
    identity.release_channel =
        reader.string_at(*object, "release_channel", "identity.release_channel");
    identity.maintainer_identity = reader.string_at(
        *object, "maintainer_identity", "identity.maintainer_identity");
}

void read_toolchain(
    Reader& reader,
    const Value::Object& root,
    ToolchainIdentity& toolchain,
    std::size_t list_limit) {
    const auto* object = reader.object_at(root, "toolchain", "toolchain");
    if (object == nullptr) {
        return;
    }
    toolchain.compiler_name =
        reader.string_at(*object, "compiler_name", "toolchain.compiler_name");
    toolchain.compiler_version = reader.string_at(
        *object, "compiler_version", "toolchain.compiler_version");
    toolchain.linker_name =
        reader.string_at(*object, "linker_name", "toolchain.linker_name");
    toolchain.linker_version =
        reader.string_at(*object, "linker_version", "toolchain.linker_version");
    toolchain.target_triple =
        reader.string_at(*object, "target_triple", "toolchain.target_triple");
    toolchain.cmake_version =
        reader.string_at(*object, "cmake_version", "toolchain.cmake_version");
    toolchain.compile_flags = reader.string_array_at(
        *object, "compile_flags", "toolchain.compile_flags", list_limit);
    toolchain.link_flags = reader.string_array_at(
        *object, "link_flags", "toolchain.link_flags", list_limit);
    toolchain.dependency_lock_sha256 = reader.string_at(
        *object, "dependency_lock_sha256", "toolchain.dependency_lock_sha256");
}

void read_pe_validation(
    Reader& reader,
    const Value::Object& root,
    PeBuildValidation& validation) {
    const auto* object =
        reader.object_at(root, "pe_validation", "pe_validation");
    if (object == nullptr) {
        return;
    }
    const auto bounded_u16 = [&reader](
                                 std::uint64_t value,
                                 std::string_view path) -> std::uint16_t {
        if (value > std::numeric_limits<std::uint16_t>::max()) {
            reader.reject(std::string{path}, "Value exceeds a 16-bit field.");
            return 0U;
        }
        return static_cast<std::uint16_t>(value);
    };
    const auto bounded_u32 = [&reader](
                                 std::uint64_t value,
                                 std::string_view path) -> std::uint32_t {
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            reader.reject(std::string{path}, "Value exceeds a 32-bit field.");
            return 0U;
        }
        return static_cast<std::uint32_t>(value);
    };

    validation.pe32_plus =
        reader.bool_at(*object, "pe32_plus", "pe_validation.pe32_plus");
    validation.machine = bounded_u16(
        reader.unsigned_at(*object, "machine", "pe_validation.machine"),
        "pe_validation.machine");
    validation.image_base =
        reader.unsigned_at(*object, "image_base", "pe_validation.image_base");
    validation.entry_point_rva = bounded_u32(
        reader.unsigned_at(
            *object, "entry_point_rva", "pe_validation.entry_point_rva"),
        "pe_validation.entry_point_rva");
    validation.subsystem = bounded_u16(
        reader.unsigned_at(*object, "subsystem", "pe_validation.subsystem"),
        "pe_validation.subsystem");
    validation.section_count = bounded_u16(
        reader.unsigned_at(
            *object, "section_count", "pe_validation.section_count"),
        "pe_validation.section_count");
    validation.import_table_valid = reader.bool_at(
        *object, "import_table_valid", "pe_validation.import_table_valid");
    validation.exception_directory_valid = reader.bool_at(
        *object,
        "exception_directory_valid",
        "pe_validation.exception_directory_valid");
    validation.load_config_valid = reader.bool_at(
        *object, "load_config_valid", "pe_validation.load_config_valid");
    validation.source_mapping_complete = reader.bool_at(
        *object,
        "source_mapping_complete",
        "pe_validation.source_mapping_complete");
    validation.report_sha256 =
        reader.string_at(*object, "report_sha256", "pe_validation.report_sha256");
}

void read_modifications(
    Reader& reader,
    const Value::Object& root,
    std::vector<IncludedModification>& modifications,
    std::size_t limit) {
    const auto* array = reader.array_at(
        root, "included_modifications", "included_modifications", limit);
    if (array == nullptr) {
        return;
    }
    modifications.reserve(array->size());
    for (std::size_t index = 0U; index < array->size(); ++index) {
        const auto path = "included_modifications[" + std::to_string(index) + "]";
        const auto* object = (*array)[index].as_object();
        if (object == nullptr) {
            reader.reject(path, "Expected an object.");
            return;
        }
        modifications.push_back(IncludedModification{
            .modification_id = reader.string_at(
                *object, "modification_id", path + ".modification_id"),
            .version = reader.string_at(*object, "version", path + ".version"),
            .package_manifest_sha256 = reader.string_at(
                *object,
                "package_manifest_sha256",
                path + ".package_manifest_sha256"),
        });
    }
}

void read_mappings(
    Reader& reader,
    const Value::Object& root,
    std::vector<SourceBinaryMapping>& mappings,
    std::size_t limit,
    std::size_t list_limit) {
    const auto* array = reader.array_at(
        root, "source_binary_mappings", "source_binary_mappings", limit);
    if (array == nullptr) {
        return;
    }
    mappings.reserve(array->size());
    for (std::size_t index = 0U; index < array->size(); ++index) {
        const auto path = "source_binary_mappings[" + std::to_string(index) + "]";
        const auto* object = (*array)[index].as_object();
        if (object == nullptr) {
            reader.reject(path, "Expected an object.");
            return;
        }

        SourceBinaryMapping mapping;
        mapping.id = reader.string_at(*object, "id", path + ".id");
        mapping.modification_id = reader.string_at(
            *object, "modification_id", path + ".modification_id");
        mapping.modification_version = reader.string_at(
            *object, "modification_version", path + ".modification_version");
        mapping.source_unit_id =
            reader.string_at(*object, "source_unit_id", path + ".source_unit_id");
        mapping.source_path =
            reader.string_at(*object, "source_path", path + ".source_path");
        mapping.source_line_begin = static_cast<std::uint32_t>(
            reader.unsigned_at(
                *object, "source_line_begin", path + ".source_line_begin"));
        mapping.source_line_end = static_cast<std::uint32_t>(
            reader.unsigned_at(
                *object, "source_line_end", path + ".source_line_end"));
        mapping.recovered_symbol_id = reader.string_at(
            *object, "recovered_symbol_id", path + ".recovered_symbol_id");
        mapping.recovered_symbol_name = reader.string_at(
            *object, "recovered_symbol_name", path + ".recovered_symbol_name");

        // An absent original RVA means the mapping does not claim one. Reading
        // a null as zero would turn "no claim" into a claim about address 0.
        const auto* original = object->find("original_rva") == object->end()
            ? nullptr
            : &object->find("original_rva")->second;
        if (original != nullptr && !original->is_null()) {
            mapping.original_rva =
                reader.unsigned_at(*object, "original_rva", path + ".original_rva");
        }

        mapping.output_file_offset = reader.unsigned_at(
            *object, "output_file_offset", path + ".output_file_offset");
        mapping.output_rva =
            reader.unsigned_at(*object, "output_rva", path + ".output_rva");
        mapping.output_va =
            reader.unsigned_at(*object, "output_va", path + ".output_va");
        mapping.byte_size =
            reader.unsigned_at(*object, "byte_size", path + ".byte_size");

        const auto confidence =
            reader.string_at(*object, "confidence", path + ".confidence");
        if (!parse_enum(kConfidences, confidence, mapping.confidence)) {
            reader.reject(
                path + ".confidence",
                "Unknown mapping confidence '" + confidence + "'.");
        }
        mapping.evidence_record_ids = reader.string_array_at(
            *object,
            "evidence_record_ids",
            path + ".evidence_record_ids",
            list_limit);
        mappings.push_back(std::move(mapping));
    }
}

void read_test_results(
    Reader& reader,
    const Value::Object& root,
    std::vector<BuildTestResult>& tests,
    std::size_t limit,
    std::size_t list_limit) {
    const auto* array =
        reader.array_at(root, "test_results", "test_results", limit);
    if (array == nullptr) {
        return;
    }
    tests.reserve(array->size());
    for (std::size_t index = 0U; index < array->size(); ++index) {
        const auto path = "test_results[" + std::to_string(index) + "]";
        const auto* object = (*array)[index].as_object();
        if (object == nullptr) {
            reader.reject(path, "Expected an object.");
            return;
        }

        BuildTestResult test;
        test.id = reader.string_at(*object, "id", path + ".id");
        const auto layer = reader.string_at(*object, "layer", path + ".layer");
        if (!parse_enum(kLayers, layer, test.layer)) {
            reader.reject(path + ".layer", "Unknown test layer '" + layer + "'.");
        }
        test.profile = reader.string_at(*object, "profile", path + ".profile");
        test.mandatory = reader.bool_at(*object, "mandatory", path + ".mandatory");
        test.passed = reader.bool_at(*object, "passed", path + ".passed");
        test.summary = reader.string_at(*object, "summary", path + ".summary");
        test.report_sha256 =
            reader.string_at(*object, "report_sha256", path + ".report_sha256");
        test.evidence_record_ids = reader.string_array_at(
            *object,
            "evidence_record_ids",
            path + ".evidence_record_ids",
            list_limit);
        tests.push_back(std::move(test));
    }
}

void read_distribution(
    Reader& reader,
    const Value::Object& root,
    CustomBuildRecord& record) {
    const auto* object = reader.object_at(root, "distribution", "distribution");
    if (object == nullptr) {
        return;
    }
    const auto form = reader.string_at(*object, "form", "distribution.form");
    if (!parse_enum(kForms, form, record.distribution_form)) {
        reader.reject(
            "distribution.form", "Unknown distribution form '" + form + "'.");
    }
    record.distribution_permission =
        reader.string_at(*object, "permission", "distribution.permission");
    const auto attestation = reader.string_at(
        *object, "attestation_state", "distribution.attestation_state");
    if (!parse_enum(kAttestations, attestation, record.attestation_state)) {
        reader.reject(
            "distribution.attestation_state",
            "Unknown attestation state '" + attestation + "'.");
    }
    record.attestation_reference = reader.string_at(
        *object, "attestation_reference", "distribution.attestation_reference",
        false);
    record.rollback_build_id = reader.string_at(
        *object, "rollback_build_id", "distribution.rollback_build_id", false);
}

} // namespace

CustomBuildImportResult custom_build_from_json(
    std::string_view input,
    CustomBuildImportLimits limits) {
    CustomBuildImportResult result;
    Reader reader{result.diagnostics};

    const auto parsed = core::json::Parser::parse(input, limits.json);
    if (!parsed.ok()) {
        for (const auto& error : parsed.errors) {
            reader.reject(
                "$", "JSON parse error at offset " +
                    std::to_string(error.offset) + ": " + error.message);
        }
        if (result.diagnostics.empty()) {
            reader.reject("$", "The document could not be parsed.");
        }
        return result;
    }

    const auto* root = parsed.value->as_object();
    if (root == nullptr) {
        reader.reject("$", "The document root must be an object.");
        return result;
    }

    CustomBuildRecord record;
    read_identity(reader, *root, record.identity);
    read_toolchain(reader, *root, record.toolchain, limits.max_string_list_items);
    read_pe_validation(reader, *root, record.pe_validation);

    // The manifest reports the build's own fields under "build"; the
    // workspace-derived siblings there describe the workspace that emitted it
    // and are deliberately not read back.
    if (const auto* build = reader.object_at(*root, "build", "build")) {
        const auto status = reader.string_at(*build, "status", "build.status");
        if (!parse_enum(kStatuses, status, record.status)) {
            reader.reject("build.status", "Unknown build status '" + status + "'.");
        }
        record.executable_size =
            reader.unsigned_at(*build, "executable_size", "build.executable_size");
    }

    read_modifications(
        reader, *root, record.included_modifications, limits.max_modifications);
    record.unified_implementation_decision_ids = reader.string_array_at(
        *root,
        "unified_implementation_decision_ids",
        "unified_implementation_decision_ids",
        limits.max_string_list_items);
    read_mappings(
        reader,
        *root,
        record.source_binary_mappings,
        limits.max_mappings,
        limits.max_string_list_items);
    read_test_results(
        reader,
        *root,
        record.test_results,
        limits.max_test_results,
        limits.max_string_list_items);
    record.resource_package_versions = reader.string_array_at(
        *root,
        "resource_package_versions",
        "resource_package_versions",
        limits.max_string_list_items);
    record.known_issues = reader.string_array_at(
        *root, "known_issues", "known_issues", limits.max_string_list_items);
    record.credits = reader.string_array_at(
        *root, "credits", "credits", limits.max_string_list_items);
    read_distribution(reader, *root, record);

    if (!result.diagnostics.empty()) {
        return result;
    }

    // The record is offered to the canonical validity rules rather than
    // trusted because it parsed. An importer that accepted a structurally
    // well-formed but invalid build would push the refusal down to the
    // registry, where the reason for it is no longer visible.
    if (!record.valid()) {
        reader.reject(
            "$",
            "The imported record is structurally complete but not a valid "
            "Custom Build Record under the canonical rules.");
        return result;
    }

    result.record = std::move(record);
    return result;
}

} // namespace dmc::rengine::source
