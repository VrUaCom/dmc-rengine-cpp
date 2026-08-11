#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {

bool RuntimeSourceBindings::valid_for(
    const VolumeBootstrapPlan& bootstrap) const noexcept {
    if (!bootstrap.valid() || physical_source_id.empty() ||
        archives.size() != bootstrap.registered_archives.size()) {
        return false;
    }

    std::unordered_set<std::uint32_t> indices;
    std::unordered_set<std::string> source_ids;
    source_ids.insert(physical_source_id);

    for (const auto& binding : archives) {
        if (!binding.valid() || binding.volume_index >= bootstrap.first_missing_index ||
            !indices.insert(binding.volume_index).second ||
            !source_ids.insert(binding.source_id).second) {
            return false;
        }
    }

    for (const auto& volume : bootstrap.registered_archives) {
        if (!indices.contains(volume.index)) {
            return false;
        }
    }
    return true;
}

const ArchiveSourceBinding* RuntimeSourceBindings::archive(
    std::uint32_t volume_index) const noexcept {
    const auto iterator = std::find_if(
        archives.begin(), archives.end(),
        [volume_index](const ArchiveSourceBinding& binding) {
            return binding.volume_index == volume_index;
        });
    return iterator == archives.end() ? nullptr : &*iterator;
}

namespace {

[[nodiscard]] RuntimeResolutionReport invalid_configuration(
    std::string_view request,
    std::string detail) {
    return RuntimeResolutionReport{
        .request = std::string{request},
        .status = RuntimeResolutionStatus::invalid_source_configuration,
        .resolved = std::nullopt,
        .ambiguous_matches = {},
        .probes = {},
        .detail = std::move(detail),
    };
}

[[nodiscard]] RuntimeResolutionReport ambiguous_report(
    std::string_view request,
    std::vector<RuntimeResolutionProbe> probes,
    const gdspaces::SourceLookupReport& lookup,
    std::string detail) {
    return RuntimeResolutionReport{
        .request = std::string{request},
        .status = RuntimeResolutionStatus::ambiguous,
        .resolved = std::nullopt,
        .ambiguous_matches = lookup.matches,
        .probes = std::move(probes),
        .detail = std::move(detail),
    };
}

[[nodiscard]] RuntimeResolutionReport resolved_report(
    std::string_view request,
    std::vector<RuntimeResolutionProbe> probes,
    const gdspaces::ResourceRef& resource) {
    return RuntimeResolutionReport{
        .request = std::string{request},
        .status = RuntimeResolutionStatus::resolved,
        .resolved = resource,
        .ambiguous_matches = {},
        .probes = std::move(probes),
        .detail = {},
    };
}

} // namespace

RuntimeResolutionReport RuntimeResourceResolver::resolve(
    std::string_view request,
    const VolumeBootstrapPlan& bootstrap,
    const RuntimeSourceBindings& bindings,
    const gdspaces::SourceRegistry& sources) {
    const auto plan = ResourceLookupPolicy::plan(request);
    if (!plan.valid()) {
        return RuntimeResolutionReport{
            .request = std::string{request},
            .status = RuntimeResolutionStatus::invalid_request,
            .resolved = std::nullopt,
            .ambiguous_matches = {},
            .probes = {},
            .detail = "The request cannot produce the canonical 12-attempt VFS lookup plan.",
        };
    }

    if (!bindings.valid_for(bootstrap)) {
        return invalid_configuration(
            request,
            "The runtime source bindings do not exactly match the contiguous bootstrap mount set.");
    }
    if (sources.find(bindings.physical_source_id) == nullptr) {
        return invalid_configuration(
            request,
            "The physical runtime source binding is not mounted in SourceRegistry.");
    }
    for (const auto& volume : bootstrap.registered_archives) {
        const auto* binding = bindings.archive(volume.index);
        if (binding == nullptr || sources.find(binding->source_id) == nullptr) {
            return invalid_configuration(
                request,
                "A runtime-equivalent archive volume binding is not mounted in SourceRegistry.");
        }
    }

    std::vector<RuntimeResolutionProbe> probes;

    for (const auto& attempt : plan.attempts) {
        if (attempt.provider == ResourceProviderClass::archive) {
            const auto provider_key = ResourcePathPolicy::archive(attempt.candidate);
            for (const auto volume_index : bootstrap.archive_resolution_order) {
                const auto* binding = bindings.archive(volume_index);
                if (binding == nullptr) {
                    return invalid_configuration(
                        request,
                        "Archive precedence references a volume without a source binding.");
                }

                auto lookup = sources.lookup(
                    binding->source_id,
                    provider_key,
                    ResourcePathPolicy::archive_flags);
                probes.push_back(RuntimeResolutionProbe{
                    .lookup_attempt_index = attempt.attempt_index,
                    .provider = attempt.provider,
                    .candidate = attempt.candidate,
                    .provider_key = provider_key,
                    .source_id = binding->source_id,
                    .archive_volume_index = volume_index,
                    .lookup = lookup,
                });

                if (lookup.ambiguous()) {
                    return ambiguous_report(
                        request,
                        std::move(probes),
                        lookup,
                        "A single archive mount contains multiple physical resources for the same normalized runtime key; duplicate-key winner behavior is not yet promoted.");
                }
                if (lookup.unique()) {
                    return resolved_report(
                        request, std::move(probes), lookup.matches.front());
                }
            }
            continue;
        }

        const auto provider_key = ResourcePathPolicy::physical(attempt.candidate);
        auto lookup = sources.lookup(
            bindings.physical_source_id,
            provider_key,
            ResourcePathPolicy::physical_flags);
        probes.push_back(RuntimeResolutionProbe{
            .lookup_attempt_index = attempt.attempt_index,
            .provider = attempt.provider,
            .candidate = attempt.candidate,
            .provider_key = provider_key,
            .source_id = bindings.physical_source_id,
            .archive_volume_index = std::nullopt,
            .lookup = lookup,
        });

        if (lookup.ambiguous()) {
            return ambiguous_report(
                request,
                std::move(probes),
                lookup,
                "The physical provider exposes multiple resources for one normalized runtime key.");
        }
        if (lookup.unique()) {
            return resolved_report(
                request, std::move(probes), lookup.matches.front());
        }
    }

    return RuntimeResolutionReport{
        .request = std::string{request},
        .status = RuntimeResolutionStatus::not_found,
        .resolved = std::nullopt,
        .ambiguous_matches = {},
        .probes = std::move(probes),
        .detail = "All canonical archive and physical lookup attempts completed without a resource hit.",
    };
}

} // namespace dmc::rengine::profiles::dmc3
