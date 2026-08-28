#include "dmc_rengine/profiles/dmc3/companion_index_locator.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <algorithm>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

void add_error(
    CompanionIndexDiscoveryResult& result,
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

[[nodiscard]] std::string lookup_key(std::string_view path) {
    constexpr auto flags =
        gdspaces::flag_value(gdspaces::ResourcePathNormalizationFlag::lowercase_ascii) |
        gdspaces::flag_value(gdspaces::ResourcePathNormalizationFlag::strip_leading_separators) |
        gdspaces::flag_value(gdspaces::ResourcePathNormalizationFlag::strip_trailing_separators);
    return gdspaces::ResourcePathNormalizer::normalize(path, flags);
}

[[nodiscard]] char separator_for(std::string_view path) noexcept {
    const auto slash = path.find_last_of('/');
    const auto backslash = path.find_last_of('\\');
    if (backslash != std::string_view::npos &&
        (slash == std::string_view::npos || backslash > slash)) {
        return '\\';
    }
    return '/';
}

} // namespace

bool CompanionIndexDiscoveryResult::ok() const noexcept {
    return payload.has_value() && matched_kind.has_value() &&
        !has_error(diagnostics);
}

std::vector<CompanionIndexCandidate> CompanionIndexLocator::candidates_for(
    const gdspaces::ResourceId& container) {
    std::vector<CompanionIndexCandidate> result;
    if (!container.valid() || container.logical_path.empty() ||
        !gdspaces::ResourcePathNormalizer::c_string_compatible(container.logical_path) ||
        container.logical_path.find("::") != std::string::npos) {
        return result;
    }

    const auto separator = container.logical_path.find_last_of("/\\");
    const auto directory = separator == std::string::npos
        ? std::string_view{}
        : std::string_view{container.logical_path}.substr(0U, separator + 1U);
    auto leaf = separator == std::string::npos
        ? std::string_view{container.logical_path}
        : std::string_view{container.logical_path}.substr(separator + 1U);
    if (leaf.empty()) {
        return result;
    }

    const auto dot = leaf.find_last_of('.');
    const auto stem = dot == std::string_view::npos
        ? leaf
        : leaf.substr(0U, dot);
    if (stem.empty()) {
        return result;
    }

    std::string sibling;
    sibling.reserve(directory.size() + stem.size() + 6U);
    sibling.append(directory);
    sibling.append(stem);
    sibling.append(".index");
    result.push_back(CompanionIndexCandidate{
        .kind = CompanionIndexCandidateKind::sibling_manifest,
        .logical_path = std::move(sibling),
    });

    const char nested_separator = separator_for(container.logical_path);
    std::string expanded;
    expanded.reserve(directory.size() + stem.size() * 2U + 7U);
    expanded.append(directory);
    expanded.append(stem);
    expanded.push_back(nested_separator);
    expanded.append(stem);
    expanded.append(".index");
    if (lookup_key(expanded) != lookup_key(result.front().logical_path)) {
        result.push_back(CompanionIndexCandidate{
            .kind = CompanionIndexCandidateKind::expanded_directory_manifest,
            .logical_path = std::move(expanded),
        });
    }
    return result;
}

CompanionIndexDiscoveryResult CompanionIndexLocator::discover(
    const gdspaces::ISource& source,
    const gdspaces::ResourceId& container) {
    CompanionIndexDiscoveryResult result;
    result.candidates = candidates_for(container);
    if (container.source_id != source.id() || result.candidates.empty()) {
        add_error(
            result,
            container,
            "gdspaces.dmc3-companion-index.invalid-container",
            "Companion .index discovery requires a source-native physical/logical container path from the same source.");
        return result;
    }

    const auto resources = source.enumerate();
    struct Match final {
        CompanionIndexCandidateKind kind;
        gdspaces::ResourceRef resource;
    };
    std::vector<Match> matches;

    for (const auto& candidate : result.candidates) {
        const auto candidate_key = lookup_key(candidate.logical_path);
        for (const auto& resource : resources) {
            if (!resource.valid() || resource.id.source_id != container.source_id) {
                continue;
            }
            if (lookup_key(resource.id.logical_path) == candidate_key) {
                matches.push_back(Match{
                    .kind = candidate.kind,
                    .resource = resource,
                });
            }
        }
    }

    if (matches.empty()) {
        return result;
    }
    if (matches.size() != 1U) {
        add_error(
            result,
            container,
            "gdspaces.dmc3-companion-index.ambiguous",
            "More than one exact physical-path companion .index candidate exists; display-name fallbacks and arbitrary first-match selection are forbidden.");
        return result;
    }

    auto payload = source.read(matches.front().resource.id);
    if (!payload.has_value()) {
        add_error(
            result,
            matches.front().resource.id,
            "gdspaces.dmc3-companion-index.read-failed",
            "The exact companion .index resource was resolved but could not be materialized.");
        return result;
    }
    if (has_error(payload->diagnostics)) {
        add_error(
            result,
            matches.front().resource.id,
            "gdspaces.dmc3-companion-index.unreadable",
            "The exact companion .index resource materialized with an error diagnostic.");
        return result;
    }

    result.matched_kind = matches.front().kind;
    result.payload = std::move(payload);
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
