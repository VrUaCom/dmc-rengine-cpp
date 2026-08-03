#include "dmc_rengine/source/modification.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] std::string hash(char value) {
    return std::string(64U, value);
}

[[nodiscard]] dmc::rengine::source::SourceModificationPackage package() {
    using namespace dmc::rengine::source;
    return SourceModificationPackage{
        .id = "inventory-capacity-source-mod",
        .version = "1.0.0",
        .title = "Inventory Capacity Source Modification",
        .description = "Replaces the recovered inventory capacity policy with a bounded configurable implementation.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = SourceBaselineIdentity{
            .game_profile = "dmc3-hd",
            .original_exe_sha256 =
                "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
            .source_schema_version = "recovered-cpp-v1",
            .source_revision = "source-tree-0001",
            .compiler_profile = "msvc-x64-dmc3-hd",
            .prior_custom_build_id = {},
        },
        .semantic_intent = "Make inventory capacity explicit, testable, and source-level rather than a collection of unrelated immediate patches.",
        .change_units = {
            SourceChangeUnit{
                .id = "inventory-policy-unit",
                .source_path = "recovered/gameplay/item/inventory_policy.cpp",
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash('b'),
                .semantic_intent = "Introduce one maintained inventory capacity policy.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = "symbol:inventory-capacity-check",
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "InventoryCapacityCheck",
                        .rva = 0x1000U,
                        .evidence_record_ids = {
                            "ev-dmc3-inventory-limit-patch-family",
                        },
                    },
                },
            },
        },
        .dependencies = {},
        .declared_conflicts = {},
        .configuration_keys = {"inventory.max_count"},
        .build_requirements = {"recovered-cpp-v1", "msvc-x64-dmc3-hd"},
        .tests = {
            ModificationTestRequirement{
                .id = "inventory-capacity-unit",
                .layer = TestLayer::unit,
                .profile = "inventory-policy",
                .expected_result = "All supported capacity values preserve bounds and ownership invariants.",
                .mandatory = true,
            },
            ModificationTestRequirement{
                .id = "inventory-capacity-runtime",
                .layer = TestLayer::runtime_smoke,
                .profile = "dmc3-hd-copy-build",
                .expected_result = "Inventory collection and save/load remain stable in a copied test build.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {
            "ev-dmc3-inventory-limit-patch-family",
        },
        .save_compatibility = "No save schema change; existing values remain bounded by the selected configuration.",
        .migration_notes = "Migrate the existing six-site binary evidence into one recovered source policy after all locations are confirmed.",
        .rollback_instructions = "Remove the source modification and rebuild from source-tree-0001.",
        .known_risks = {
            "All six historical comparison sites must be reconciled with the recovered source control flow.",
        },
        .provenance = ModificationProvenance{
            .authors = {"VrUaCom"},
            .contribution_ledger_refs = {"conversation:dmc-rengine:item-editor"},
            .reviewers = {"DMC Rengine maintainers"},
            .ai_assistance_disclosure = "AI-assisted architecture and test generation; human review required.",
            .licence = "Project contribution licence pending repository policy.",
            .redistribution_permission = "Source contribution only; executable distribution requires a separate release decision.",
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::source::ModificationDependency;
    using dmc::rengine::source::version_constraint_matches;

    const auto valid = package();
    assert(valid.valid());
    assert(valid.baseline.valid());
    assert(valid.change_units[0].valid());
    assert(valid.change_units[0].affected_symbols[0].valid());

    auto bad_hash = valid;
    bad_hash.baseline.original_exe_sha256 = "not-a-sha";
    assert(!bad_hash.valid());

    auto unchanged_unit = valid;
    unchanged_unit.change_units[0].result_sha256 =
        unchanged_unit.change_units[0].baseline_sha256;
    assert(!unchanged_unit.valid());

    auto no_tests = valid;
    no_tests.tests.clear();
    assert(!no_tests.valid());

    auto no_rollback = valid;
    no_rollback.rollback_instructions.clear();
    assert(!no_rollback.valid());

    auto duplicate_author = valid;
    duplicate_author.provenance.authors.push_back("VrUaCom");
    assert(!duplicate_author.valid());

    auto self_dependency = valid;
    self_dependency.dependencies.push_back(ModificationDependency{
        .modification_id = self_dependency.id,
        .version_constraint = "*",
        .mandatory = true,
        .rationale = "Invalid self dependency.",
    });
    assert(!self_dependency.valid());

    assert(version_constraint_matches("*", "1.0.0"));
    assert(version_constraint_matches("1.0.0", "1.0.0"));
    assert(!version_constraint_matches("1.0.1", "1.0.0"));
    assert(!version_constraint_matches(">=1.0.0", "1.2.0"));
    return 0;
}
