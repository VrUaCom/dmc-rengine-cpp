#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void add_diagnostic(
    std::vector<gdspaces::Diagnostic>& diagnostics,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::optional<gdspaces::ResourceId> resource = std::nullopt) {
    diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::move(resource),
    });
}

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

[[nodiscard]] gdspaces::StageResourceCategory category_for_role(
    StageResourceRole role) noexcept {
    switch (role) {
    case StageResourceRole::script:
        return gdspaces::StageResourceCategory::scripts;
    case StageResourceRole::room_config:
        return gdspaces::StageResourceCategory::unknown;
    case StageResourceRole::room_effects:
        return gdspaces::StageResourceCategory::effects;
    case StageResourceRole::room_sound:
        return gdspaces::StageResourceCategory::sounds;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] std::string lower_copy(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return value;
}

[[nodiscard]] gdspaces::StageResourceCategory nested_category_for(
    const gdspaces::ResourceRef& resource) {
    const auto format = lower_copy(resource.format);
    if (format == "txt") {
        return gdspaces::StageResourceCategory::scripts;
    }
    if (format == "scm" || format == "mod") {
        return gdspaces::StageResourceCategory::models;
    }
    if (format == "dds" || format == "ptx") {
        return gdspaces::StageResourceCategory::textures;
    }
    if (format == "cam") {
        return gdspaces::StageResourceCategory::cameras;
    }
    if (format == "lig" || format == "lig2") {
        return gdspaces::StageResourceCategory::lighting;
    }
    if (format == "hits") {
        return gdspaces::StageResourceCategory::collision;
    }
    if (format == "wav" || format == "ogg" || format == "snd" ||
        format == "se") {
        return gdspaces::StageResourceCategory::sounds;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] bool payload_identity_matches(
    const gdspaces::ResourcePayload& payload,
    const gdspaces::ResourceRef& resolved) {
    return payload.resource.id.canonical() == resolved.id.canonical();
}

[[nodiscard]] bool provenance_matches_payload(
    const gdspaces::ResourcePayload& payload) noexcept {
    if (!payload.byte_provenance.has_value() ||
        !payload.byte_provenance->valid()) {
        return false;
    }
    return payload.byte_provenance->materialized_size ==
        static_cast<std::uint64_t>(payload.bytes.size());
}

void append_nested_candidates(
    const StageRuntimeLoadedResource& loaded,
    std::vector<gdspaces::StageMemberCandidate>& candidates) {
    if (!loaded.expansion.has_value()) {
        return;
    }

    const auto parent_role = std::string{to_string(loaded.resolution.reference.role)};
    for (const auto& expansion : loaded.expansion->expansions) {
        for (const auto& child : expansion.children) {
            if (!child.entry.populated || !child.payload.readable() ||
                !child.payload.resource.valid()) {
                continue;
            }
            candidates.push_back(gdspaces::StageMemberCandidate{
                .resource = child.payload.resource,
                .category = nested_category_for(child.payload.resource),
                .role = "nested:" + parent_role + ":slot/" +
                    std::to_string(child.entry.slot_index),
            });
        }
    }
}

} // namespace

bool StageRuntimeLoadedResource::payload_valid() const noexcept {
    if (!resolution.resolved() || !payload.has_value() ||
        !payload->readable()) {
        return false;
    }
    return payload_identity_matches(*payload, *resolution.runtime.resolved) &&
        provenance_matches_payload(*payload);
}

bool StageRuntimeLoadedResource::complete() const noexcept {
    if (!payload_valid() || has_error(diagnostics)) {
        return false;
    }
    if (payload->resource.container) {
        return expansion.has_value() && expansion->complete();
    }
    return !expansion.has_value();
}

bool StageRuntimeLoadReport::complete() const noexcept {
    if (!resolution.complete() || !bundle.has_value() || !bundle->valid() ||
        has_error(diagnostics)) {
        return false;
    }
    return std::all_of(
        resources.begin(), resources.end(),
        [](const StageRuntimeLoadedResource& resource) {
            return resource.complete();
        });
}

StageRuntimeLoadReport StageRuntimeLoader::load_entry(
    const StageCatalogEntry& entry,
    const VolumeBootstrapPlan& bootstrap,
    const RuntimeSourceBindings& bindings,
    const gdspaces::SourceRegistry& sources,
    const formats::ContainerParserRegistry& parsers,
    gdspaces::ContainerTreeExpansionLimits limits) {
    StageRuntimeLoadReport report{
        .resolution = StageRuntimeResolver::resolve_entry(
            entry, bootstrap, bindings, sources),
        .resources = {},
        .bundle = std::nullopt,
        .diagnostics = {},
    };

    report.diagnostics.insert(
        report.diagnostics.end(),
        report.resolution.diagnostics.begin(),
        report.resolution.diagnostics.end());

    if (!report.resolution.complete()) {
        add_diagnostic(
            report.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-load.resolution-incomplete",
            "Stage Level-C loading stopped because one or more catalog resource references did not resolve uniquely.");
        return report;
    }

    std::vector<gdspaces::StageMemberCandidate> candidates;
    candidates.reserve(16U);

    for (std::size_t index = 0U; index < report.resources.size(); ++index) {
        auto& loaded = report.resources[index];
        loaded.resolution = report.resolution.resources[index];

        const auto& resolved = *loaded.resolution.runtime.resolved;
        auto payload = sources.read(resolved.id);
        if (!payload.has_value()) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.source-read-failed",
                "The resolved GDSpaces resource could not be materialized by its owning source.",
                resolved.id);
            report.diagnostics.insert(
                report.diagnostics.end(),
                loaded.diagnostics.begin(), loaded.diagnostics.end());
            continue;
        }

        loaded.payload = std::move(*payload);
        loaded.diagnostics.insert(
            loaded.diagnostics.end(),
            loaded.payload->diagnostics.begin(),
            loaded.payload->diagnostics.end());

        if (!loaded.payload->readable()) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.payload-unreadable",
                "The source returned a payload carrying error diagnostics.",
                resolved.id);
        }
        if (!payload_identity_matches(*loaded.payload, resolved)) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.identity-mismatch",
                "The materialized payload identity does not match the ResourceId selected by runtime resolution.",
                resolved.id);
        }
        if (!loaded.payload->byte_provenance.has_value()) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.provenance-missing",
                "The materialized resource has no byte provenance.",
                resolved.id);
        } else if (!loaded.payload->byte_provenance->valid()) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.provenance-invalid",
                "The materialized resource byte provenance is internally invalid.",
                resolved.id);
        } else if (loaded.payload->byte_provenance->materialized_size !=
                   static_cast<std::uint64_t>(loaded.payload->bytes.size())) {
            add_diagnostic(
                loaded.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.stage-load.materialized-size-mismatch",
                "Byte provenance materialized_size does not match the actual payload byte count.",
                resolved.id);
        }

        if (!has_error(loaded.diagnostics) && loaded.payload->resource.container) {
            loaded.expansion = gdspaces::ContainerTreeExpander::expand(
                *loaded.payload, parsers, limits);
            loaded.diagnostics.insert(
                loaded.diagnostics.end(),
                loaded.expansion->diagnostics.begin(),
                loaded.expansion->diagnostics.end());
            if (!loaded.expansion->complete()) {
                add_diagnostic(
                    loaded.diagnostics,
                    gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-load.container-expansion-incomplete",
                    "The resolved stage container could not be fully expanded within the evidence-bounded parser/limit contract.",
                    resolved.id);
            }
        }

        // Preserve all safely materialized data even if a later expansion step
        // fails. Completion remains false through scoped diagnostics, but a
        // partial failure must not erase a valid root resource or readable
        // children that were already bounded and classified successfully.
        if (loaded.payload_valid()) {
            candidates.push_back(gdspaces::StageMemberCandidate{
                .resource = loaded.payload->resource,
                .category = category_for_role(loaded.resolution.reference.role),
                .role = std::string{to_string(loaded.resolution.reference.role)},
            });
            append_nested_candidates(loaded, candidates);
        }

        report.diagnostics.insert(
            report.diagnostics.end(),
            loaded.diagnostics.begin(), loaded.diagnostics.end());
    }

    const auto semantic_stage_id = entry.semantic_stage_id.value_or(std::string{});
    const auto display_name = semantic_stage_id.empty()
        ? "Stage resource set " + entry.catalog_entry_id
        : "Stage " + semantic_stage_id;
    report.bundle = gdspaces::StageBundleAssembler::assemble(
        gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = entry.catalog_entry_id,
            .display_name = display_name,
            .exe_evidence_id = entry.evidence_id,
            .resource_set_id = entry.catalog_entry_id,
            .semantic_stage_id = semantic_stage_id,
        },
        candidates);

    report.diagnostics.insert(
        report.diagnostics.end(),
        report.bundle->diagnostics().begin(),
        report.bundle->diagnostics().end());

    if (std::any_of(
            report.resources.begin(), report.resources.end(),
            [](const StageRuntimeLoadedResource& resource) {
                return !resource.complete();
            })) {
        add_diagnostic(
            report.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-load.incomplete",
            "The StageBundle contains the safely materialized subset; Level-C completion was not reached.");
    }

    return report;
}

} // namespace dmc::rengine::profiles::dmc3
