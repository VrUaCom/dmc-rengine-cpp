#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string lower_copy(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return value;
}

} // namespace

StageBundle StageBundleAssembler::assemble(
    StageIdentity identity,
    std::span<const StageMemberCandidate> candidates) {
    StageBundle bundle(std::move(identity));

    for (const auto& candidate : candidates) {
        if (!candidate.resource.valid()) {
            bundle.add_diagnostic(Diagnostic{
                .severity = DiagnosticSeverity::error,
                .code = "gdspaces.stage.invalid_candidate",
                .message = "A stage candidate has an invalid resource reference.",
                .resource = std::nullopt,
            });
            continue;
        }

        const auto category = candidate.category.value_or(
            infer_category(candidate.resource));
        const auto role = candidate.role.empty()
            ? std::string{to_string(category)}
            : candidate.role;

        if (!bundle.add(StageMember{
                .category = category,
                .resource = candidate.resource,
                .role = role,
            })) {
            bundle.add_diagnostic(Diagnostic{
                .severity = DiagnosticSeverity::warning,
                .code = "gdspaces.stage.duplicate_member",
                .message = "A duplicate stage member was ignored.",
                .resource = candidate.resource.id,
            });
        }
    }

    return bundle;
}

StageResourceCategory StageBundleAssembler::infer_category(
    const ResourceRef& resource) {
    const auto format = lower_copy(resource.format);
    const auto path = lower_copy(resource.id.logical_path);

    if (path.find("_effect") != std::string::npos ||
        path.find("/effect/") != std::string::npos ||
        path.find("\\effect\\") != std::string::npos) {
        return StageResourceCategory::effects;
    }

    if (format == "txt") {
        return StageResourceCategory::scripts;
    }
    if (format == "scm" || format == "mod") {
        return StageResourceCategory::models;
    }
    if (format == "dds" || format == "ptx") {
        return StageResourceCategory::textures;
    }
    if (format == "cam") {
        return StageResourceCategory::cameras;
    }
    if (format == "lig" || format == "lig2") {
        return StageResourceCategory::lighting;
    }
    if (format == "hits") {
        return StageResourceCategory::collision;
    }
    if (format == "wav" || format == "ogg" || format == "snd" ||
        format == "se") {
        return StageResourceCategory::sounds;
    }

    return StageResourceCategory::unknown;
}

} // namespace dmc::rengine::gdspaces
