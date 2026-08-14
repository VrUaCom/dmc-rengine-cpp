#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_ascii(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::string_view value) {
    for (std::size_t index = 0U; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] std::vector<std::byte> make_level_c_pac() {
    std::vector<std::byte> bytes(48U, std::byte{0});
    write_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 4U, 2U);
    write_u32(bytes, 8U, 16U);
    write_u32(bytes, 12U, 32U);
    write_ascii(bytes, 16U, "DDS ");
    write_ascii(bytes, 32U, std::string_view{"SCM\0", 4U});
    return bytes;
}

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

struct ReadableEntry final {
    dmc::rengine::gdspaces::ResourceRef resource;
    std::vector<std::byte> bytes;
    bool include_provenance{true};
};

class ReadableStaticSource final : public dmc::rengine::gdspaces::ISource {
public:
    ReadableStaticSource(
        std::string source_id,
        std::vector<ReadableEntry> entries)
        : source_id_(std::move(source_id)),
          entries_(std::move(entries)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "stage-runtime-readable-test";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        std::vector<dmc::rengine::gdspaces::ResourceRef> result;
        result.reserve(entries_.size());
        for (const auto& entry : entries_) {
            result.push_back(entry.resource);
        }
        return result;
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId& resource) const override {
        for (const auto& entry : entries_) {
            if (entry.resource.id.canonical() != resource.canonical()) {
                continue;
            }

            auto payload = dmc::rengine::gdspaces::ResourcePayload{
                .resource = entry.resource,
                .bytes = entry.bytes,
                .diagnostics = {},
                .byte_provenance = std::nullopt,
            };
            if (entry.include_provenance) {
                payload.byte_provenance = dmc::rengine::gdspaces::ByteProvenance{
                    .kind = dmc::rengine::gdspaces::ByteOriginKind::direct_source_span,
                    .authority_id = source_id_,
                    .offset = entry.resource.id.offset,
                    .stored_size = static_cast<std::uint64_t>(entry.bytes.size()),
                    .materialized_size = static_cast<std::uint64_t>(entry.bytes.size()),
                    .transform = dmc::rengine::gdspaces::ByteTransform::none,
                    .crc32 = std::nullopt,
                };
            }
            return payload;
        }
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<ReadableEntry> entries_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef make_resource(
    std::string source_id,
    std::string logical_path,
    std::string chain,
    std::uint64_t size = 32U) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = std::move(chain),
            .offset = 0U,
            .size = size,
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

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan
physical_only_bootstrap() {
    constexpr std::array<std::uint32_t, 0> no_archives{};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(no_archives);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeSourceBindings
physical_only_bindings() {
    return dmc::rengine::profiles::dmc3::RuntimeSourceBindings{
        .physical_source_id = "physical-load",
        .archives = {},
    };
}

[[nodiscard]] std::vector<ReadableEntry> level_c_entries(
    std::optional<std::size_t> missing_provenance_index = std::nullopt,
    std::optional<std::size_t> malformed_pac_index = std::nullopt) {
    const std::array<std::string, 4> paths{
        "GDataX360.afs/shared_intro.pac",
        "GDataX360.afs/st777cfg_alias.pac",
        "GDataX360.afs/common_effects.pac",
        "GDataX360.afs/snd_shared.pac",
    };

    std::vector<ReadableEntry> entries;
    entries.reserve(paths.size());
    for (std::size_t index = 0U; index < paths.size(); ++index) {
        auto bytes = make_level_c_pac();
        if (malformed_pac_index == index) {
            write_u32(bytes, 12U, 8U);
        }
        entries.push_back(ReadableEntry{
            .resource = make_resource(
                "physical-load", paths[index], "direct/" + std::to_string(index),
                static_cast<std::uint64_t>(bytes.size())),
            .bytes = std::move(bytes),
            .include_provenance = missing_provenance_index != index,
        });
    }
    return entries;
}

[[nodiscard]] dmc::rengine::gdspaces::SourceRegistry level_c_sources(
    std::optional<std::size_t> missing_provenance_index = std::nullopt,
    std::optional<std::size_t> malformed_pac_index = std::nullopt) {
    dmc::rengine::gdspaces::SourceRegistry registry;
    assert(registry.mount(std::make_unique<ReadableStaticSource>(
        "physical-load",
        level_c_entries(missing_provenance_index, malformed_pac_index))));
    return registry;
}

[[nodiscard]] bool has_diagnostic(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    std::string_view code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::StageBundleAssembler;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::StageRuntimeLoader;
    using dmc::rengine::profiles::dmc3::StageRuntimeResolver;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

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
        assert(report.plan.resource_set_key() == entry.catalog_entry_id);
        assert(!report.plan.semantic_stage_known());
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

        const auto bundle = StageBundleAssembler::assemble(
            StageIdentity{
                .profile = "dmc3-hd",
                .stage_id = report.catalog_entry_id,
                .display_name = "Catalog row 17",
                .exe_evidence_id = report.plan.evidence_id,
                .resource_set_id = report.catalog_entry_id,
                .semantic_stage_id = {},
            },
            candidates);
        assert(bundle.valid());
        assert(bundle.size() == 4U);
        assert(bundle.identity().resource_set_key() == entry.catalog_entry_id);
        assert(!bundle.identity().semantic_stage_known());
    }

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

    // Level-C success: one arbitrary non-pattern StageCatalogEntry resolves,
    // materializes, validates byte provenance, expands four PAC roots and
    // produces a loaded parent+child StageBundle with unresolved semantics.
    {
        const auto load_bootstrap = physical_only_bootstrap();
        const auto load_bindings = physical_only_bindings();
        auto registry = level_c_sources();
        const auto parsers = make_container_parser_registry();
        assert(load_bootstrap.valid());
        assert(load_bindings.valid_for(load_bootstrap));

        const auto report = StageRuntimeLoader::load_entry(
            entry,
            load_bootstrap,
            load_bindings,
            registry,
            parsers);
        assert(report.complete());
        assert(report.resolution.complete());
        assert(report.bundle.has_value());
        assert(report.bundle->valid());
        assert(report.bundle->identity().resource_set_key() == entry.catalog_entry_id);
        assert(!report.bundle->identity().semantic_stage_known());
        assert(report.bundle->size() == 12U);
        assert(report.bundle->members(StageResourceCategory::scripts).size() == 1U);
        assert(report.bundle->members(StageResourceCategory::unknown).size() == 1U);
        assert(report.bundle->members(StageResourceCategory::effects).size() == 1U);
        assert(report.bundle->members(StageResourceCategory::sounds).size() == 1U);
        assert(report.bundle->members(StageResourceCategory::textures).size() == 4U);
        assert(report.bundle->members(StageResourceCategory::models).size() == 4U);

        for (const auto& loaded : report.resources) {
            assert(loaded.complete());
            assert(loaded.payload.has_value());
            assert(loaded.payload->byte_provenance.has_value());
            assert(loaded.payload->byte_provenance->valid());
            assert(loaded.expansion.has_value());
            assert(loaded.expansion->complete());
            assert(loaded.expansion->expansions.size() == 1U);
            assert(loaded.expansion->expansions[0].children.size() == 2U);
        }
    }

    // Missing provenance is a hard Level-C failure even when lookup and bytes
    // succeed. The partial materialized state is retained for diagnostics.
    {
        const auto load_bootstrap = physical_only_bootstrap();
        const auto load_bindings = physical_only_bindings();
        auto registry = level_c_sources(2U, std::nullopt);
        const auto parsers = make_container_parser_registry();
        const auto report = StageRuntimeLoader::load_entry(
            entry,
            load_bootstrap,
            load_bindings,
            registry,
            parsers);
        assert(!report.complete());
        assert(report.resolution.complete());
        assert(report.resources[2].payload.has_value());
        assert(!report.resources[2].payload_valid());
        assert(has_diagnostic(
            report.resources[2].diagnostics,
            "dmc3.stage-load.provenance-missing"));
        assert(has_diagnostic(report.diagnostics, "dmc3.stage-load.incomplete"));
    }

    // Malformed container bytes keep exact read/provenance but fail recursive
    // expansion, so a readable payload cannot be mistaken for a loaded stage.
    {
        const auto load_bootstrap = physical_only_bootstrap();
        const auto load_bindings = physical_only_bindings();
        auto registry = level_c_sources(std::nullopt, 1U);
        const auto parsers = make_container_parser_registry();
        const auto report = StageRuntimeLoader::load_entry(
            entry,
            load_bootstrap,
            load_bindings,
            registry,
            parsers);
        assert(!report.complete());
        assert(report.resources[1].payload_valid());
        assert(report.resources[1].expansion.has_value());
        assert(!report.resources[1].expansion->complete());
        assert(has_diagnostic(
            report.resources[1].diagnostics,
            "dmc3.stage-load.container-expansion-incomplete"));
    }

    return 0;
}
