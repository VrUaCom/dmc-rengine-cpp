#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class StaticSource final : public dmc::rengine::gdspaces::ISource {
public:
    StaticSource(
        std::string source_id,
        std::vector<dmc::rengine::gdspaces::ResourceRef> resources)
        : source_id_(std::move(source_id)),
          resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "stage-runtime-test";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        return resources_;
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId&) const override {
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef make_resource(
    std::string source_id,
    std::string logical_path,
    std::string chain) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = std::move(chain),
            .offset = 0U,
            .size = 32U,
        },
        .display_name = "stage-resource.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };
}

void mount(
    dmc::rengine::gdspaces::SourceRegistry& registry,
    std::string source_id,
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources = {}) {
    assert(registry.mount(std::make_unique<StaticSource>(
        std::move(source_id), std::move(resources))));
}

[[nodiscard]] dmc::rengine::profiles::dmc3::StageResourceTableRowObservation row() {
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::StageResourceTableCellObservation;
    using dmc::rengine::profiles::dmc3::StageResourceTableRowObservation;

    StageResourceTableRowObservation result{.row_index = 17U, .cells = {}};
    const std::array<std::string, 4> paths{
        "scr/shared_intro.pac",
        "room/st777cfg_alias.pac",
        "room/common_effects.pac",
        "se/snd_shared.pac",
    };
    const std::array<StageResourceRole, 4> roles{
        StageResourceRole::script,
        StageResourceRole::room_config,
        StageResourceRole::room_effects,
        StageResourceRole::room_sound,
    };

    for (std::uint32_t column = 0U; column < 4U; ++column) {
        result.cells[column] = StageResourceTableCellObservation{
            .row_index = result.row_index,
            .column_index = column,
            .role = roles[column],
            .cell_file_offset = 0x1000U + column * 0x10U,
            .cell_rva = 0x2000U + column * 0x10U,
            .cell_va = 0x140002000ULL + column * 0x10U,
            .path_pointer_va = 0x140003000ULL + column * 0x20U,
            .path_file_offset = 0x3000U + column * 0x20U,
            .logical_path = paths[column],
        };
    }
    return result;
}

[[nodiscard]] dmc::rengine::profiles::dmc3::StageCatalogEntry catalog_entry() {
    const auto observation = row();
    return dmc::rengine::profiles::dmc3::StageCatalogEntry{
        .catalog_entry_id = "dmc3-stage-resource-table/row/17",
        .row_index = observation.row_index,
        .evidence_id = "ev-dmc3-stage-resource-table",
        .observation = observation,
        .semantic_stage_id = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan bootstrap() {
    constexpr std::array<std::uint32_t, 2> present{0U, 1U};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeSourceBindings bindings() {
    return dmc::rengine::profiles::dmc3::RuntimeSourceBindings{
        .physical_source_id = "physical",
        .archives = {
            {0U, "archive-0"},
            {1U, "archive-1"},
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::SourceRegistry complete_sources() {
    dmc::rengine::gdspaces::SourceRegistry registry;
    mount(registry, "physical", {
        make_resource(
            "physical", "GDataX360.afs/common_effects.pac", "direct"),
    });
    mount(registry, "archive-0", {
        make_resource(
            "archive-0", "GDataX360.afs/st777cfg_alias.pac", "nbz[7]"),
    });
    mount(registry, "archive-1", {
        make_resource(
            "archive-1", "GDataX360.afs/shared_intro.pac", "nbz[3]"),
        make_resource(
            "archive-1", "snd_shared.pac", "nbz[9]"),
    });
    return registry;
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::StageBundleAssembler;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::StageRuntimeResolver;

    const auto entry = catalog_entry();
    assert(entry.complete());
    assert(!entry.semantic_stage_id.has_value());
    const auto runtime_bootstrap = bootstrap();
    const auto runtime_bindings = bindings();
    assert(runtime_bootstrap.valid());
    assert(runtime_bindings.valid_for(runtime_bootstrap));

    {
        auto registry = complete_sources();
        const auto report = StageRuntimeResolver::resolve_entry(
            entry,
            runtime_bootstrap,
            runtime_bindings,
            registry);

        assert(report.complete());
        assert(report.catalog_entry_id == entry.catalog_entry_id);
        assert(report.table_row_index == 17U);
        assert(report.plan.valid());
        assert(report.plan.stage_id == entry.catalog_entry_id);
        assert(report.plan.evidence_id == "ev-dmc3-stage-resource-table");
        assert(report.plan.resources[0].logical_path == "scr/shared_intro.pac");
        assert(report.plan.resources[1].logical_path == "room/st777cfg_alias.pac");
        assert(report.plan.resources[2].logical_path == "room/common_effects.pac");
        assert(report.plan.resources[3].logical_path == "se/snd_shared.pac");

        assert(report.resources[0].runtime.ok());
        assert(report.resources[0].runtime.resolved->id.source_id == "archive-1");
        assert(report.resources[1].runtime.resolved->id.source_id == "archive-0");
        assert(report.resources[2].runtime.resolved->id.source_id == "physical");
        assert(report.resources[3].runtime.resolved->id.source_id == "archive-1");
        assert(report.resources[3].runtime.probes.back().lookup_attempt_index == 5U);

        const auto candidates = report.resolved_candidates();
        assert(candidates.size() == 4U);
        assert(candidates[0].category == StageResourceCategory::scripts);
        assert(candidates[1].category == StageResourceCategory::unknown);
        assert(candidates[2].category == StageResourceCategory::effects);
        assert(candidates[3].category == StageResourceCategory::sounds);

        // StageIdentity still exposes a field named stage_id. Until that generic
        // schema is split, catalog row identity is carried here explicitly as a
        // technical identity, not as an inferred semantic gameplay stage id.
        const auto bundle = StageBundleAssembler::assemble(
            StageIdentity{
                .profile = "dmc3-hd",
                .stage_id = report.catalog_entry_id,
                .display_name = "Catalog row 17",
                .exe_evidence_id = report.plan.evidence_id,
            },
            candidates);
        assert(bundle.valid());
        assert(bundle.size() == 4U);
    }

    // One missing role keeps all per-role traces but blocks bundle candidates.
    {
        dmc::rengine::gdspaces::SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-0", {
            make_resource(
                "archive-0", "GDataX360.afs/st777cfg_alias.pac", "nbz[7]"),
        });
        mount(registry, "archive-1", {
            make_resource(
                "archive-1", "GDataX360.afs/shared_intro.pac", "nbz[3]"),
            make_resource(
                "archive-1", "snd_shared.pac", "nbz[9]"),
        });

        const auto report = StageRuntimeResolver::resolve_entry(
            entry,
            runtime_bootstrap,
            runtime_bindings,
            registry);
        assert(!report.complete());
        assert(report.resources[2].runtime.status == RuntimeResolutionStatus::not_found);
        assert(report.resources[2].runtime.probes.size() == 18U);
        assert(report.resolved_candidates().empty());
        assert(!report.diagnostics.empty());
    }

    // Ambiguity is preserved at the stage-role level and never converted into
    // an arbitrary StageBundle member.
    {
        dmc::rengine::gdspaces::SourceRegistry ambiguous_registry;
        mount(ambiguous_registry, "physical", {
            make_resource(
                "physical", "GDataX360.afs/common_effects.pac", "direct"),
        });
        mount(ambiguous_registry, "archive-0", {
            make_resource(
                "archive-0", "GDataX360.afs/st777cfg_alias.pac", "nbz[7]"),
        });
        mount(ambiguous_registry, "archive-1", {
            make_resource(
                "archive-1", "GDataX360.afs/SHARED_INTRO.PAC", "nbz[3]"),
            make_resource(
                "archive-1", "gdatax360.afs\\shared_intro.pac", "nbz[4]"),
            make_resource(
                "archive-1", "snd_shared.pac", "nbz[9]"),
        });

        const auto report = StageRuntimeResolver::resolve_entry(
            entry,
            runtime_bootstrap,
            runtime_bindings,
            ambiguous_registry);
        assert(!report.complete());
        assert(report.resources[0].runtime.status == RuntimeResolutionStatus::ambiguous);
        assert(report.resources[0].runtime.ambiguous_matches.size() == 2U);
        assert(report.resolved_candidates().empty());
    }

    // An incomplete catalog entry is rejected before any VFS request is made.
    {
        auto incomplete_entry = entry;
        incomplete_entry.observation.cells[2].logical_path.clear();
        auto registry = complete_sources();
        const auto report = StageRuntimeResolver::resolve_entry(
            incomplete_entry,
            runtime_bootstrap,
            runtime_bindings,
            registry);
        assert(!report.complete());
        assert(!report.diagnostics.empty());
        for (const auto& resource : report.resources) {
            assert(resource.runtime.probes.empty());
        }
    }

    // Raw-row compatibility requires an explicit catalog row identity and does
    // not infer any semantic stage id from filenames.
    {
        auto registry = complete_sources();
        const auto report = StageRuntimeResolver::resolve_row(
            "dmc3-stage-resource-table/row/17",
            "ev-dmc3-stage-resource-table",
            entry.observation,
            runtime_bootstrap,
            runtime_bindings,
            registry);
        assert(report.complete());
        assert(report.catalog_entry_id == "dmc3-stage-resource-table/row/17");
    }

    return 0;
}
