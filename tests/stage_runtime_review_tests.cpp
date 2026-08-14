#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class ReviewSource final : public dmc::rengine::gdspaces::ISource {
public:
    ReviewSource(
        std::string source_id,
        std::vector<dmc::rengine::gdspaces::ResourceRef> resources)
        : source_id_(std::move(source_id)),
          resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "stage-runtime-review";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        return resources_;
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId& id) const override {
        for (const auto& resource : resources_) {
            if (resource.id.canonical() != id.canonical()) {
                continue;
            }

            std::vector<std::byte> bytes(
                static_cast<std::size_t>(resource.id.size),
                std::byte{0x2A});
            return dmc::rengine::gdspaces::ResourcePayload{
                .resource = resource,
                .bytes = std::move(bytes),
                .diagnostics = {},
                .byte_provenance = dmc::rengine::gdspaces::ByteProvenance{
                    .kind = dmc::rengine::gdspaces::ByteOriginKind::direct_source_span,
                    .authority_id = source_id_,
                    .offset = 0U,
                    .stored_size = resource.id.size,
                    .materialized_size = resource.id.size,
                    .transform = dmc::rengine::gdspaces::ByteTransform::none,
                    .crc32 = std::nullopt,
                },
            };
        }
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string logical_path) {
    const auto display_name = logical_path.substr(logical_path.find_last_of('/') + 1U);
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "physical-review",
            .logical_path = std::move(logical_path),
            .container_chain = "direct",
            .offset = 0U,
            .size = 4U,
        },
        .display_name = display_name,
        .format = "bin",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] dmc::rengine::profiles::dmc3::StageCatalogEntry make_entry(
    std::uint32_t row_index,
    const std::array<std::string, 4>& paths,
    const std::array<std::uint16_t, 4>& kind16,
    std::string semantic_stage_id) {
    using dmc::rengine::profiles::dmc3::StageCatalogEntry;
    using dmc::rengine::profiles::dmc3::StageResourceRole;
    using dmc::rengine::profiles::dmc3::StageResourceTableCellObservation;
    using dmc::rengine::profiles::dmc3::StageResourceTableRowObservation;

    StageResourceTableRowObservation row{
        .row_index = row_index,
        .cells = {},
    };
    const std::array<StageResourceRole, 4> roles{
        StageResourceRole::script,
        StageResourceRole::room_config,
        StageResourceRole::room_effects,
        StageResourceRole::room_sound,
    };

    for (std::uint32_t column = 0U; column < 4U; ++column) {
        row.cells[column] = StageResourceTableCellObservation{
            .row_index = row.row_index,
            .column_index = column,
            .role = roles[column],
            .cell_file_offset = 0x5000U + column * 0x10U,
            .cell_rva = 0x6000U + column * 0x10U,
            .cell_va = 0x140006000ULL + column * 0x10U,
            .kind16 = kind16[column],
            .path_pointer_va = 0x140007000ULL + column * 0x20U,
            .path_file_offset = 0x7000U + column * 0x20U,
            .logical_path = paths[column],
        };
    }

    return StageCatalogEntry{
        .catalog_entry_id = "dmc3-stage-resource-table/row/" +
            std::to_string(row_index),
        .row_index = row.row_index,
        .evidence_id = "dmc3-stage-wave2",
        .observation = std::move(row),
        .semantic_stage_id = std::move(semantic_stage_id),
    };
}

[[nodiscard]] dmc::rengine::profiles::dmc3::StageCatalogEntry duplicate_entry() {
    return make_entry(
        23U,
        {
            "scr/shared.pac",
            "room/shared.pac",
            "room/effects.pac",
            "se/sound.pac",
        },
        {1U, 2U, 3U, 4U},
        "semantic-stage-review");
}

[[nodiscard]] dmc::rengine::profiles::dmc3::StageCatalogEntry fallback_entry() {
    return make_entry(
        24U,
        {
            "scr/missing.pac",
            "room/config.pac",
            "room/effects.pac",
            "se/sound.pac",
        },
        {0U, 1U, 1U, 1U},
        "semantic-stage-list-fallback");
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan bootstrap() {
    constexpr std::array<std::uint32_t, 0> no_archives{};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(no_archives);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeSourceBindings bindings() {
    return dmc::rengine::profiles::dmc3::RuntimeSourceBindings{
        .physical_source_id = "physical-review",
        .archives = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::SourceRegistry mount_sources(
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources) {
    dmc::rengine::gdspaces::SourceRegistry registry;
    assert(registry.mount(std::make_unique<ReviewSource>(
        "physical-review", std::move(resources))));
    return registry;
}

[[nodiscard]] dmc::rengine::gdspaces::SourceRegistry duplicate_sources() {
    return mount_sources({
        resource("GDataX360.afs/shared.pac"),
        resource("GDataX360.afs/effects.pac"),
        resource("GDataX360.afs/sound.pac"),
    });
}

[[nodiscard]] dmc::rengine::gdspaces::SourceRegistry fallback_sources() {
    return mount_sources({
        resource("GDataX360.afs/missing.lst"),
        resource("GDataX360.afs/config.pac"),
        resource("GDataX360.afs/effects.pac"),
        resource("GDataX360.afs/sound.pac"),
    });
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
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::StageRuntimeLoader;
    using dmc::rengine::profiles::dmc3::StageRuntimeResolver;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    const auto catalog_entry = duplicate_entry();
    const auto load_bootstrap = bootstrap();
    const auto load_bindings = bindings();
    assert(catalog_entry.complete());
    assert(load_bootstrap.valid());
    assert(load_bindings.valid_for(load_bootstrap));

    // Separately evidenced semantic identity and Wave-2 kind16 must survive the
    // raw-row compatibility bridge without replacing technical resource-set ID.
    {
        auto registry = duplicate_sources();
        const auto report = StageRuntimeResolver::resolve_entry(
            catalog_entry,
            load_bootstrap,
            load_bindings,
            registry);
        assert(report.complete());
        assert(report.plan.resource_set_key() == catalog_entry.catalog_entry_id);
        assert(report.plan.semantic_stage_known());
        assert(report.plan.semantic_stage_id == "semantic-stage-review");
        assert(report.plan.resources[0].kind16 == 1U);
        assert(report.plan.resources[3].kind16 == 4U);
        assert(report.resources[0].reference.kind16 == 1U);

        // The two requests intentionally share one basename and therefore
        // resolve to the same canonical physical resource under the evidenced
        // runtime lookup policy. Resolution itself remains valid.
        assert(report.resources[0].runtime.resolved.has_value());
        assert(report.resources[1].runtime.resolved.has_value());
        assert(report.resources[0].runtime.resolved->id.canonical() ==
               report.resources[1].runtime.resolved->id.canonical());
    }

    // Current StageBundle/workspace semantics store one role per canonical
    // resource. A duplicate root binding must therefore fail materialized
    // StageBundle completion instead of silently dropping one role.
    {
        auto registry = duplicate_sources();
        const auto parsers = make_container_parser_registry();
        const auto report = StageRuntimeLoader::load_entry(
            catalog_entry,
            load_bootstrap,
            load_bindings,
            registry,
            parsers);

        assert(report.resolution.complete());
        for (const auto& loaded : report.resources) {
            assert(loaded.complete());
        }
        assert(report.bundle.has_value());
        assert(report.bundle->valid());
        assert(report.bundle->identity().resource_set_key() ==
               catalog_entry.catalog_entry_id);
        assert(report.bundle->identity().semantic_stage_id ==
               "semantic-stage-review");
        assert(report.bundle->size() == 3U);
        assert(has_diagnostic(
            report.diagnostics,
            "dmc3.stage-load.duplicate-root-identity"));
        assert(!report.complete());
    }

    // Wave 2 confirms that kind16 == 0 probes a .lst rewrite after the original
    // path is unavailable. The fallback resource can be found by the central
    // VFS resolver, but must not be promoted to a normal root because list
    // parsing/recursive ownership/lifetime remain open.
    {
        const auto list_entry = fallback_entry();
        auto registry = fallback_sources();
        const auto report = StageRuntimeResolver::resolve_entry(
            list_entry,
            load_bootstrap,
            load_bindings,
            registry);

        assert(report.plan.resources[0].kind16 == 0U);
        assert(report.resources[0].reference.kind16 == 0U);
        assert(report.resources[0].runtime.status == RuntimeResolutionStatus::not_found);
        assert(report.resources[0].list_fallback.has_value());
        assert(report.resources[0].list_fallback->ok());
        assert(report.resources[0].list_fallback_required());
        assert(report.resources[0].list_fallback->resolved->id.logical_path ==
               "GDataX360.afs/missing.lst");
        assert(has_diagnostic(
            report.diagnostics,
            "dmc3.stage-runtime.list-fallback-required"));
        assert(!report.complete());
        assert(report.resolved_candidates().empty());
    }

    return 0;
}
