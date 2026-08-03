#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::source {

enum class SourceSymbolKind {
    function,
    class_type,
    global,
    source_file,
    initialization_unit,
    resource_binding,
    save_schema,
};

[[nodiscard]] constexpr std::string_view to_string(
    SourceSymbolKind kind) noexcept {
    switch (kind) {
    case SourceSymbolKind::function: return "function";
    case SourceSymbolKind::class_type: return "class";
    case SourceSymbolKind::global: return "global";
    case SourceSymbolKind::source_file: return "source-file";
    case SourceSymbolKind::initialization_unit: return "initialization-unit";
    case SourceSymbolKind::resource_binding: return "resource-binding";
    case SourceSymbolKind::save_schema: return "save-schema";
    }
    return "function";
}

enum class ModificationImplementationKind {
    recovered_source,
    generated_source,
    binary_migration,
    resource_schema,
};

[[nodiscard]] constexpr std::string_view to_string(
    ModificationImplementationKind kind) noexcept {
    switch (kind) {
    case ModificationImplementationKind::recovered_source:
        return "recovered-source";
    case ModificationImplementationKind::generated_source:
        return "generated-source";
    case ModificationImplementationKind::binary_migration:
        return "binary-migration";
    case ModificationImplementationKind::resource_schema:
        return "resource-schema";
    }
    return "recovered-source";
}

enum class ConflictClass {
    textual_source,
    same_function_semantic,
    class_layout,
    global_state,
    abi,
    initialization_order,
    threading_synchronization,
    memory_ownership,
    resource_format,
    save_format,
    configuration_key,
    compiler_linker,
    performance_tradeoff,
    incompatible_design_intent,
};

[[nodiscard]] constexpr std::string_view to_string(
    ConflictClass conflict) noexcept {
    switch (conflict) {
    case ConflictClass::textual_source: return "textual-source";
    case ConflictClass::same_function_semantic:
        return "same-function-semantic";
    case ConflictClass::class_layout: return "class-layout";
    case ConflictClass::global_state: return "global-state";
    case ConflictClass::abi: return "abi";
    case ConflictClass::initialization_order: return "initialization-order";
    case ConflictClass::threading_synchronization:
        return "threading-synchronization";
    case ConflictClass::memory_ownership: return "memory-ownership";
    case ConflictClass::resource_format: return "resource-format";
    case ConflictClass::save_format: return "save-format";
    case ConflictClass::configuration_key: return "configuration-key";
    case ConflictClass::compiler_linker: return "compiler-linker";
    case ConflictClass::performance_tradeoff: return "performance-tradeoff";
    case ConflictClass::incompatible_design_intent:
        return "incompatible-design-intent";
    }
    return "textual-source";
}

enum class TestLayer {
    unit,
    integration,
    compile_link,
    pe_structure,
    abi_symbol,
    save_load,
    stage_resource_loading,
    performance,
    long_running_stability,
    compatibility,
    install_update_rollback,
    runtime_smoke,
};

[[nodiscard]] constexpr std::string_view to_string(TestLayer layer) noexcept {
    switch (layer) {
    case TestLayer::unit: return "unit";
    case TestLayer::integration: return "integration";
    case TestLayer::compile_link: return "compile-link";
    case TestLayer::pe_structure: return "pe-structure";
    case TestLayer::abi_symbol: return "abi-symbol";
    case TestLayer::save_load: return "save-load";
    case TestLayer::stage_resource_loading: return "stage-resource-loading";
    case TestLayer::performance: return "performance";
    case TestLayer::long_running_stability:
        return "long-running-stability";
    case TestLayer::compatibility: return "compatibility";
    case TestLayer::install_update_rollback:
        return "install-update-rollback";
    case TestLayer::runtime_smoke: return "runtime-smoke";
    }
    return "unit";
}

struct SourceBaselineIdentity final {
    std::string game_profile;
    std::string original_exe_sha256;
    std::string source_schema_version;
    std::string source_revision;
    std::string compiler_profile;
    std::string prior_custom_build_id;

    [[nodiscard]] bool valid() const noexcept;
    friend bool operator==(
        const SourceBaselineIdentity&,
        const SourceBaselineIdentity&) = default;
};

struct SourceSymbolRef final {
    std::string id;
    SourceSymbolKind kind{SourceSymbolKind::function};
    std::string recovered_name;
    std::optional<std::uint64_t> rva;
    std::vector<std::string> evidence_record_ids;

    [[nodiscard]] bool valid() const noexcept;
};

struct SourceChangeUnit final {
    std::string id;
    std::string source_path;
    std::string baseline_sha256;
    std::string result_sha256;
    std::string semantic_intent;
    std::vector<SourceSymbolRef> affected_symbols;

    [[nodiscard]] bool valid() const noexcept;
};

struct ModificationDependency final {
    std::string modification_id;
    std::string version_constraint;
    bool mandatory{true};
    std::string rationale;

    [[nodiscard]] bool valid() const noexcept;
};

struct ModificationConflictDeclaration final {
    std::string modification_id;
    ConflictClass conflict_class{ConflictClass::textual_source};
    std::string rationale;
    bool blocking{true};

    [[nodiscard]] bool valid() const noexcept;
};

struct ModificationTestRequirement final {
    std::string id;
    TestLayer layer{TestLayer::unit};
    std::string profile;
    std::string expected_result;
    bool mandatory{true};

    [[nodiscard]] bool valid() const noexcept;
};

struct ModificationProvenance final {
    std::vector<std::string> authors;
    std::vector<std::string> contribution_ledger_refs;
    std::vector<std::string> reviewers;
    std::string ai_assistance_disclosure;
    std::string licence;
    std::string redistribution_permission;

    [[nodiscard]] bool valid() const noexcept;
};

struct SourceModificationPackage final {
    std::string id;
    std::string version;
    std::string title;
    std::string description;
    ModificationImplementationKind implementation_kind{
        ModificationImplementationKind::recovered_source};
    SourceBaselineIdentity baseline;
    std::string semantic_intent;
    std::vector<SourceChangeUnit> change_units;
    std::vector<ModificationDependency> dependencies;
    std::vector<ModificationConflictDeclaration> declared_conflicts;
    std::vector<std::string> configuration_keys;
    std::vector<std::string> build_requirements;
    std::vector<ModificationTestRequirement> tests;
    std::vector<std::string> evidence_record_ids;
    std::string save_compatibility;
    std::string migration_notes;
    std::string rollback_instructions;
    std::vector<std::string> known_risks;
    ModificationProvenance provenance;

    [[nodiscard]] bool valid() const noexcept;
};

[[nodiscard]] bool version_constraint_matches(
    std::string_view constraint,
    std::string_view version) noexcept;

} // namespace dmc::rengine::source
