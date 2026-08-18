#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

bool ArchiveResourceIndexBinding::valid() const noexcept {
    return VolumeBootstrapPolicy::runtime_index_valid(volume_index) &&
        !source_id.empty() && key_index.valid() &&
        key_index.source_id() == source_id &&
        key_index.normalization_flags() == ResourcePathPolicy::archive_flags;
}

std::optional<RuntimeResourceIndexBindings> RuntimeResourceIndexBindings::build(
    const VolumeBootstrapPlan& bootstrap,
    std::string physical_source_id,
    std::span<const ArchiveSourceBinding> archives,
    const gdspaces::SourceRegistry& sources) {
    if (!bootstrap.valid() || physical_source_id.empty() ||
        archives.size() != bootstrap.registered_archives.size()) {
        return std::nullopt;
    }

    const auto* physical_source = sources.find(physical_source_id);
    if (physical_source == nullptr) {
        return std::nullopt;
    }

    RuntimeResourceIndexBindings result;
    result.physical_source_id_ = std::move(physical_source_id);
    const auto physical_resources = physical_source->enumerate();
    result.physical_index_ = gdspaces::ResourceKeyIndex::build(
        result.physical_source_id_,
        ResourcePathPolicy::physical_flags,
        physical_resources);
    if (!result.physical_index_.valid()) {
        return std::nullopt;
    }

    result.archives_.reserve(archives.size());
    for (std::size_t index = 0U; index < archives.size(); ++index) {
        const auto& input = archives[index];
        if (!input.valid() || input.source_id == result.physical_source_id_) {
            return std::nullopt;
        }

        for (std::size_t other = index + 1U; other < archives.size(); ++other) {
            if (archives[other].volume_index == input.volume_index ||
                archives[other].source_id == input.source_id) {
                return std::nullopt;
            }
        }

        const auto* source = sources.find(input.source_id);
        if (source == nullptr) {
            return std::nullopt;
        }

        const auto resources = source->enumerate();
        auto key_index = gdspaces::ResourceKeyIndex::build(
            input.source_id,
            ResourcePathPolicy::archive_flags,
            resources);
        if (!key_index.valid()) {
            return std::nullopt;
        }

        result.archives_.push_back(ArchiveResourceIndexBinding{
            .volume_index = input.volume_index,
            .source_id = input.source_id,
            .key_index = std::move(key_index),
        });
    }

    if (!result.valid_for(bootstrap, sources)) {
        return std::nullopt;
    }
    return result;
}

bool RuntimeResourceIndexBindings::valid_for(
    const VolumeBootstrapPlan& bootstrap,
    const gdspaces::SourceRegistry& sources) const noexcept {
    if (!bootstrap.valid() || physical_source_id_.empty() ||
        archives_.size() != bootstrap.registered_archives.size() ||
        !physical_index_.valid() ||
        physical_index_.source_id() != physical_source_id_ ||
        physical_index_.normalization_flags() !=
            ResourcePathPolicy::physical_flags ||
        sources.find(physical_source_id_) == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < archives_.size(); ++index) {
        const auto& binding = archives_[index];
        if (!binding.valid() || binding.source_id == physical_source_id_ ||
            sources.find(binding.source_id) == nullptr ||
            binding.volume_index >= bootstrap.first_missing_index) {
            return false;
        }

        for (std::size_t other = index + 1U; other < archives_.size(); ++other) {
            if (archives_[other].volume_index == binding.volume_index ||
                archives_[other].source_id == binding.source_id) {
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

std::string_view RuntimeResourceIndexBindings::physical_source_id() const noexcept {
    return physical_source_id_;
}

const gdspaces::ResourceKeyIndex&
RuntimeResourceIndexBindings::physical_index() const noexcept {
    return physical_index_;
}

const ArchiveResourceIndexBinding* RuntimeResourceIndexBindings::archive(
    std::uint32_t volume_index) const noexcept {
    const auto iterator = std::find_if(
        archives_.begin(), archives_.end(),
        [volume_index](const ArchiveResourceIndexBinding& binding) {
            return binding.volume_index == volume_index;
        });
    return iterator == archives_.end() ? nullptr : &*iterator;
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
    const gdspaces::ResourceKeyMatchReport& lookup,
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
    const gdspaces::ResourceKeyMatchReport& lookup,
    std::string_view expected_source,
    std::string_view expected_key) noexcept {
    if (!lookup.index_valid || !lookup.key_valid ||
        lookup.provider_key != expected_key) {
        return false;
    }

    return std::all_of(
        lookup.matches.begin(), lookup.matches.end(),
        [expected_source](const gdspaces::ResourceRef& resource) {
            return resource.valid() && resource.id.source_id == expected_source;
        });
}

} // namespace

RuntimeResolutionReport RuntimeResourceResolver::resolve(
    std::string_view request,
    const VolumeBootstrapPlan& bootstrap,
    const RuntimeResourceIndexBindings& bindings,
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

    if (!bindings.valid_for(bootstrap, sources)) {
        return invalid_configuration(
            request,
            "The runtime source/index bindings do not exactly match the contiguous bootstrap mount set.");
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
                        "Archive precedence references a volume without a validated source/index binding.");
                }

                auto lookup = binding->key_index.lookup(provider_key);
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
                        lookup, binding->source_id, provider_key)) {
                    return invalid_configuration(
                        request,
                        std::move(probes),
                        "Archive ResourceKeyIndex violated the expected key/source contract.");
                }
                if (lookup.ambiguous()) {
                    return ambiguous_report(
                        request,
                        std::move(probes),
                        lookup,
                        "A single archive mount contains multiple physical resources for the same normalized runtime key; no evidence-backed semantic winner is selected.");
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

        auto lookup = bindings.physical_index().lookup(provider_key);
        probes.push_back(RuntimeResolutionProbe{
            .lookup_attempt_index = attempt.attempt_index,
            .provider = attempt.provider,
            .candidate = attempt.candidate,
            .provider_key = provider_key,
            .source_id = std::string{bindings.physical_source_id()},
            .archive_volume_index = std::nullopt,
            .lookup = lookup,
        });

        if (!lookup_contract_valid(
                lookup, bindings.physical_source_id(), provider_key)) {
            return invalid_configuration(
                request,
                std::move(probes),
                "Physical ResourceKeyIndex violated the expected key/source contract.");
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
