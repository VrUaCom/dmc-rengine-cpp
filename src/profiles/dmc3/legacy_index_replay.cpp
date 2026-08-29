#include "dmc_rengine/profiles/dmc3/legacy_index_replay.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool valid_digest(std::string_view digest) noexcept {
    return digest.size() == 64U && std::all_of(
        digest.begin(), digest.end(),
        [](unsigned char character) {
            return std::isxdigit(character) != 0;
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

void add_error(
    LegacyIndexReplayBuildResult& result,
    const gdspaces::ResourceId& resource,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = gdspaces::DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

} // namespace

bool LegacyIndexReplayPlan::valid() const noexcept {
    if (!parent_resource.valid() || !manifest_resource.valid() ||
        !valid_digest(manifest_sha256) || raw_entries.empty() ||
        !exact_labels_from_external_index) {
        return false;
    }
    if (!directive.empty() && directive != "PNST") {
        return false;
    }
    return std::all_of(
        raw_entries.begin(), raw_entries.end(),
        [](const std::string& entry) {
            return !entry.empty();
        });
}

std::string LegacyIndexReplayPlan::render_crlf() const {
    if (!valid()) {
        return {};
    }

    std::string result;
    if (!directive.empty()) {
        result.append(directive);
        result.append("\r\n");
    }
    for (const auto& entry : raw_entries) {
        result.append(entry);
        result.append("\r\n");
    }
    return result;
}

bool LegacyIndexReplayBuildResult::ok() const noexcept {
    return plan.valid() && !has_error(diagnostics);
}

LegacyIndexReplayBuildResult LegacyIndexReplayPlanner::build(
    const gdspaces::ContainerNamingIdentitySnapshot& snapshot) {
    LegacyIndexReplayBuildResult result;
    result.plan.parent_resource = snapshot.parent_resource;

    if (!snapshot.ok()) {
        add_error(
            result,
            snapshot.parent_resource,
            "gdspaces.dmc3-index-replay.invalid-snapshot",
            "Legacy .index replay requires a valid reconciled naming snapshot.");
        return result;
    }
    if (!snapshot.external_index_evidence.has_value()) {
        add_error(
            result,
            snapshot.parent_resource,
            "gdspaces.dmc3-index-replay.no-external-index",
            "No observed external .index exists; historical manifest labels will not be fabricated.");
        return result;
    }

    const auto& context = *snapshot.external_index_evidence;
    result.plan.manifest_resource = context.manifest_resource;
    result.plan.manifest_sha256 = context.manifest_sha256;
    result.plan.directive = context.directive;

    std::vector<const gdspaces::ResourceNamingIdentity*> extracted;
    extracted.reserve(context.entry_count);
    for (const auto& child : snapshot.children) {
        if (!child.populated) {
            if (child.extracted_ordinal.has_value() ||
                child.external_index_name.has_value() ||
                child.external_index_raw_label.has_value()) {
                add_error(
                    result,
                    child.resource_id,
                    "gdspaces.dmc3-index-replay.empty-slot-named",
                    "Empty physical slots cannot consume a legacy extracted ordinal.");
                return result;
            }
            continue;
        }

        if (!child.extracted_ordinal.has_value() ||
            !child.external_index_raw_label.has_value() ||
            !child.external_index_name.has_value()) {
            add_error(
                result,
                child.resource_id,
                "gdspaces.dmc3-index-replay.populated-slot-unbound",
                "Every populated payload must retain exact external .index evidence for replay.");
            return result;
        }
        extracted.push_back(&child);
    }

    if (extracted.size() != context.entry_count) {
        add_error(
            result,
            snapshot.parent_resource,
            "gdspaces.dmc3-index-replay.entry-count-mismatch",
            "Observed manifest entry count differs from the populated extracted sequence.");
        return result;
    }

    std::sort(
        extracted.begin(), extracted.end(),
        [](const gdspaces::ResourceNamingIdentity* left,
           const gdspaces::ResourceNamingIdentity* right) {
            return *left->extracted_ordinal < *right->extracted_ordinal;
        });

    result.plan.raw_entries.reserve(extracted.size());
    for (std::size_t ordinal = 0U; ordinal < extracted.size(); ++ordinal) {
        const auto& child = *extracted[ordinal];
        if (*child.extracted_ordinal != ordinal) {
            add_error(
                result,
                child.resource_id,
                "gdspaces.dmc3-index-replay.ordinal-gap",
                "Legacy extracted ordinals must be contiguous from zero across populated payloads.");
            return result;
        }
        result.plan.raw_entries.push_back(*child.external_index_raw_label);
    }

    result.plan.exact_labels_from_external_index = true;
    if (!result.plan.valid()) {
        add_error(
            result,
            snapshot.parent_resource,
            "gdspaces.dmc3-index-replay.invalid-plan",
            "Observed naming evidence could not form a valid legacy .index replay plan.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
