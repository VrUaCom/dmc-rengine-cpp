#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string normalized_path(std::string_view path) {
    std::string result;
    result.reserve(path.size());

    auto previous_slash = false;
    for (const unsigned char character : path) {
        auto normalized = static_cast<char>(std::tolower(character));
        if (normalized == '\\') {
            normalized = '/';
        }

        if (normalized == '/') {
            if (previous_slash) {
                continue;
            }
            previous_slash = true;
        } else {
            previous_slash = false;
        }
        result.push_back(normalized);
    }

    while (result.starts_with("./")) {
        result.erase(0U, 2U);
    }
    while (!result.empty() && result.front() == '/') {
        result.erase(result.begin());
    }
    return result;
}

[[nodiscard]] bool matches_path(
    std::string_view candidate,
    std::string_view expected) {
    const auto normalized_candidate = normalized_path(candidate);
    const auto normalized_expected = normalized_path(expected);
    if (normalized_candidate == normalized_expected) {
        return true;
    }
    if (normalized_candidate.size() <= normalized_expected.size()) {
        return false;
    }

    const auto suffix_offset =
        normalized_candidate.size() - normalized_expected.size();
    return normalized_candidate.compare(suffix_offset, normalized_expected.size(),
                                        normalized_expected) == 0 &&
        normalized_candidate[suffix_offset - 1U] == '/';
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

} // namespace

bool StageResourceRowPlan::valid() const noexcept {
    if (resource_set_key().empty() || evidence_id.empty()) {
        return false;
    }

    const auto& descriptor = phase12_stage_resource_table();
    for (std::size_t index = 0; index < resources.size(); ++index) {
        if (!resources[index].valid() ||
            descriptor.role_for_column(index) != resources[index].role) {
            return false;
        }
    }
    return true;
}

bool StageResourceMatchReport::complete() const noexcept {
    if (!plan.valid() || roles.size() != plan.resources.size()) {
        return false;
    }

    if (std::any_of(
            diagnostics.begin(), diagnostics.end(),
            [](const gdspaces::Diagnostic& diagnostic) {
                return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
            })) {
        return false;
    }

    return std::all_of(
        roles.begin(), roles.end(),
        [](const StageResourceMatch& match) {
            return match.matches.size() == 1U;
        });
}

std::vector<gdspaces::StageMemberCandidate>
StageResourceMatchReport::unique_candidates() const {
    std::vector<gdspaces::StageMemberCandidate> candidates;
    for (const auto& role : roles) {
        if (role.matches.size() != 1U) {
            continue;
        }
        candidates.push_back(gdspaces::StageMemberCandidate{
            .resource = role.matches.front(),
            .category = category_for_role(role.reference.role),
            .role = std::string{to_string(role.reference.role)},
        });
    }
    return candidates;
}

StageResourceRowPlan make_stage_resource_plan_from_table_row(
    std::string resource_set_id,
    std::array<std::string, 4> logical_paths,
    std::string evidence_id) {
    StageResourceRowPlan plan{
        .stage_id = {},
        .evidence_id = std::move(evidence_id),
        .resources = {},
        .resource_set_id = std::move(resource_set_id),
    };

    const auto& descriptor = phase12_stage_resource_table();
    for (std::size_t index = 0; index < plan.resources.size(); ++index) {
        const auto role = descriptor.role_for_column(index);
        plan.resources[index] = StageResourceReference{
            .role = role.value_or(StageResourceRole::script),
            .logical_path = std::move(logical_paths[index]),
        };
    }
    return plan;
}

const StageResourceRowPlan& phase12_st001_resource_plan() noexcept {
    // Historical compatibility fixture only. It intentionally carries semantic
    // stage_id because this helper predates executable StageCatalog ingestion.
    // Production catalog rows use resource_set_id and leave stage_id unresolved.
    static const StageResourceRowPlan plan{
        .stage_id = "st001",
        .evidence_id = "ev-dmc3-stage-resource-table",
        .resources = {
            StageResourceReference{
                .role = StageResourceRole::script,
                .logical_path = "scr/st001.pac",
            },
            StageResourceReference{
                .role = StageResourceRole::room_config,
                .logical_path = "room/st001cfg.pac",
            },
            StageResourceReference{
                .role = StageResourceRole::room_effects,
                .logical_path = "room/st001_effect.pac",
            },
            StageResourceReference{
                .role = StageResourceRole::room_sound,
                .logical_path = "se/snd_r001.pac",
            },
        },
        .resource_set_id = {},
    };
    return plan;
}

StageResourceMatchReport StageResourceMatcher::match(
    StageResourceRowPlan plan,
    std::span<const gdspaces::ResourceRef> resources) {
    StageResourceMatchReport report{
        .plan = std::move(plan),
        .roles = {},
        .diagnostics = {},
    };

    if (!report.plan.valid()) {
        report.diagnostics.push_back(gdspaces::Diagnostic{
            .severity = gdspaces::DiagnosticSeverity::error,
            .code = "dmc3.stage.invalid_resource_plan",
            .message = "The DMC3 stage resource plan is invalid.",
            .resource = std::nullopt,
        });
        return report;
    }

    report.roles.reserve(report.plan.resources.size());
    for (const auto& expected : report.plan.resources) {
        StageResourceMatch match{
            .reference = expected,
            .matches = {},
        };

        for (const auto& resource : resources) {
            if (resource.valid() &&
                matches_path(resource.id.logical_path, expected.logical_path)) {
                match.matches.push_back(resource);
            }
        }

        std::sort(
            match.matches.begin(), match.matches.end(),
            [](const gdspaces::ResourceRef& left,
               const gdspaces::ResourceRef& right) {
                return left.id.canonical() < right.id.canonical();
            });

        if (match.matches.empty()) {
            report.diagnostics.push_back(gdspaces::Diagnostic{
                .severity = gdspaces::DiagnosticSeverity::warning,
                .code = "dmc3.stage.resource_missing",
                .message = "A stage resource role could not be matched: " +
                    std::string{to_string(expected.role)},
                .resource = std::nullopt,
            });
        } else if (match.matches.size() > 1U) {
            report.diagnostics.push_back(gdspaces::Diagnostic{
                .severity = gdspaces::DiagnosticSeverity::error,
                .code = "dmc3.stage.resource_ambiguous",
                .message = "A stage resource role matched multiple resources: " +
                    std::string{to_string(expected.role)},
                .resource = std::nullopt,
            });
        }

        report.roles.push_back(std::move(match));
    }

    return report;
}

} // namespace dmc::rengine::profiles::dmc3
