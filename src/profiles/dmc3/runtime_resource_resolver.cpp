#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {

bool RuntimeSourceBindings::valid_for(
    const VolumeBootstrapPlan& bootstrap) const noexcept {
    if (!bootstrap.valid() || physical_source_id.empty() ||
        archives.size() != bootstrap.registered_archives.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < archives.size(); ++index) {
        const auto& binding = archives[index];
        if (!binding.valid() || binding.volume_index >= bootstrap.first_missing_index ||
            binding.source_id == physical_source_id) {
            return false;
        }

        for (std::size_t other = index + 1U; other < archives.size(); ++other) {
            if (archives[other].volume_index == binding.volume_index ||
                archives[other].source_id == binding.source_id) {
                return false;
            }
        }
    }

    for (const auto& volume : bootstrap.registered_archives) {
        if (archive(volume.index) == nullptr) {
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
    std::vector<RuntimeResolutionProbe> probes,
    std::string detail) {
    return RuntimeResolutionReport{
        .request = std::string{request},
        .status = RuntimeResolutionStatus::invalid_source_configuration,
        .resolved = std::nullopt,
        .ambiguous_matches = {},
        .probes = std::move(probes),
        .detail = std::move(detail),
    };
}

[[nodiscard]] RuntimeResolutionReport invalid_configuration(
    std::string_view request,
    std::string detail) {
    return invalid_configuration(request, {}, std::move(detail));
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

[[nodiscard]] bool lookup_contract_valid(
    const gdspaces::SourceLookupReport& lookup,
    std::string_view expected_source,
    std::string_view expected_key,
    std::uint32_t expected_flags) noexcept {
    return lookup.provider_key_valid && lookup.source_available &&
        lookup.source_id == expected_source &&
        lookup.provider_key == expected_key &&
        lookup.normalization_flags == expected_flags;
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
            if (provider_key.empty()) {
                return invalid_configuration(
                    request,
                    std::move(probes),
                    "Archive provider normalization unexpectedly rejected a canonical lookup candidate.");
            }

            for (const auto volume_index : bootstrap.archive_resolution_order) {
                const auto* binding = bindings.archive(volume_index);
                if (binding == nullptr) {
                    return invalid_configuration(
                        request,
                        std::move(probes),
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

                if (!lookup_contract_valid(
                        lookup,
                        binding->source_id,
                        provider_key,
                        ResourcePathPolicy::archive_flags)) {
                    return invalid_configuration(
                        request,
                        std::move(probes),
                        "Archive source lookup violated the normalized-key/source availability contract.");
                }
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
        if (provider_key.empty()) {
            return invalid_configuration(
                request,
                std::move(probes),
                "Physical provider normalization unexpectedly rejected a canonical lookup candidate.");
        }

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

        if (!lookup_contract_valid(
                lookup,
                bindings.physical_source_id,
                provider_key,
                ResourcePathPolicy::physical_flags)) {
            return invalid_configuration(
                request,
                std::move(probes),
                "Physical source lookup violated the normalized-key/source availability contract.");
        }
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
