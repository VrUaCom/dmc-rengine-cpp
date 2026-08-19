#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

bool ArchiveSourceBinding::valid() const noexcept {
    return VolumeBootstrapPolicy::runtime_index_valid(volume_index) &&
        !source_id.empty();
}

bool RuntimeSourceBindings::valid_for(
    const VolumeBootstrapPlan& bootstrap) const noexcept {
    if (!bootstrap.valid() || physical_source_id.empty() ||
        archives.size() != bootstrap.registered_archives.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < archives.size(); ++index) {
        const auto& binding = archives[index];
        if (!binding.valid() ||
            binding.volume_index >= bootstrap.first_missing_index ||
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

struct MountedSourceIndex final {
    std::string source_id;
    gdspaces::ResourceKeyIndex index;
};

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

[[nodiscard]] bool index_census_is_source_clean(
    const gdspaces::ResourceKeyIndex& index,
    std::string_view source_id,
    std::uint32_t expected_flags) noexcept {
    if (!index.valid() || index.source_id() != source_id ||
        index.normalization_flags() != expected_flags) {
        return false;
    }

    const auto& receipt = index.receipt();
    return receipt.rejected_invalid_resource_count == 0U &&
        receipt.rejected_source_mismatch_count == 0U &&
        receipt.rejected_c_string_count == 0U &&
        receipt.rejected_empty_key_count == 0U;
}

[[nodiscard]] bool lookup_contract_valid(
    const gdspaces::ResourceKeyMatchReport& lookup,
    const gdspaces::ResourceKeyIndex& index,
    std::string_view expected_key,
    std::string_view expected_source_id,
    std::uint32_t expected_flags) noexcept {
    return index_census_is_source_clean(
               index, expected_source_id, expected_flags) &&
        lookup.index_valid && lookup.key_valid &&
        lookup.provider_key == expected_key;
}

[[nodiscard]] std::optional<MountedSourceIndex> build_current_source_index(
    const gdspaces::SourceRegistry& sources,
    std::string_view source_id,
    std::uint32_t normalization_flags) {
    const auto* source = sources.find(source_id);
    if (source == nullptr || source->id() != source_id) {
        return std::nullopt;
    }

    auto resources = source->enumerate();
    auto index = gdspaces::ResourceKeyIndex::build(
        std::string{source_id}, normalization_flags, resources);
    if (!index_census_is_source_clean(index, source_id, normalization_flags)) {
        return std::nullopt;
    }

    return MountedSourceIndex{
        .source_id = std::string{source_id},
        .index = std::move(index),
    };
}

[[nodiscard]] const MountedSourceIndex* archive_index(
    const std::vector<std::pair<std::uint32_t, MountedSourceIndex>>& indexes,
    std::uint32_t volume_index) noexcept {
    const auto iterator = std::find_if(
        indexes.begin(), indexes.end(),
        [volume_index](const auto& value) {
            return value.first == volume_index;
        });
    return iterator == indexes.end() ? nullptr : &iterator->second;
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
            "Runtime source bindings do not exactly match the contiguous bootstrap mount set.");
    }

    auto physical_index = build_current_source_index(
        sources, bindings.physical_source_id, ResourcePathPolicy::physical_flags);
    if (!physical_index.has_value()) {
        return invalid_configuration(
            request,
            "The physical source is missing or its current enumeration cannot produce a clean 0x0C product lookup index.");
    }

    std::vector<std::pair<std::uint32_t, MountedSourceIndex>> archive_indexes;
    archive_indexes.reserve(bootstrap.registered_archives.size());
    for (const auto& volume : bootstrap.registered_archives) {
        const auto* binding = bindings.archive(volume.index);
        if (binding == nullptr) {
            return invalid_configuration(
                request,
                "A contiguous runtime archive volume has no source binding.");
        }

        auto index = build_current_source_index(
            sources, binding->source_id, ResourcePathPolicy::archive_flags);
        if (!index.has_value()) {
            return invalid_configuration(
                request,
                "An archive source is missing or its current enumeration cannot produce a clean 0x0E lookup index.");
        }
        archive_indexes.emplace_back(volume.index, std::move(*index));
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
                const auto* mounted_index = archive_index(archive_indexes, volume_index);
                if (binding == nullptr || mounted_index == nullptr ||
                    mounted_index->source_id != binding->source_id) {
                    return invalid_configuration(
                        request,
                        std::move(probes),
                        "Archive precedence references a volume without its current mounted-source index.");
                }

                auto lookup = mounted_index->index.lookup(provider_key);
                probes.push_back(RuntimeResolutionProbe{
                    .lookup_attempt_index = attempt.attempt_index,
                    .provider = attempt.provider,
                    .lookup_evidence = RuntimeLookupEvidenceClass::recovered_archive_index,
                    .candidate = attempt.candidate,
                    .provider_key = provider_key,
                    .source_id = binding->source_id,
                    .archive_volume_index = volume_index,
                    .lookup = lookup,
                });

                if (!lookup_contract_valid(
                        lookup,
                        mounted_index->index,
                        provider_key,
                        binding->source_id,
                        ResourcePathPolicy::archive_flags)) {
                    return invalid_configuration(
                        request,
                        std::move(probes),
                        "Archive ResourceKeyIndex violated its current-source/key/normalization contract.");
                }
                if (lookup.ambiguous()) {
                    return ambiguous_report(
                        request,
                        std::move(probes),
                        lookup,
                        "The current highest-precedence archive source contains multiple physical identities for one normalized runtime key; no semantic duplicate winner is promoted.");
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

        // Evidence boundary: 0x0C normalization and the type-0 physical pass are
        // recovered. The exact downstream Win32 filename comparison/open rule is
        // still unresolved, so this source-derived index is product lookup policy,
        // explicitly tagged as such in every probe.
        auto lookup = physical_index->index.lookup(provider_key);
        probes.push_back(RuntimeResolutionProbe{
            .lookup_attempt_index = attempt.attempt_index,
            .provider = attempt.provider,
            .lookup_evidence = RuntimeLookupEvidenceClass::product_physical_index,
            .candidate = attempt.candidate,
            .provider_key = provider_key,
            .source_id = bindings.physical_source_id,
            .archive_volume_index = std::nullopt,
            .lookup = lookup,
        });

        if (!lookup_contract_valid(
                lookup,
                physical_index->index,
                provider_key,
                bindings.physical_source_id,
                ResourcePathPolicy::physical_flags)) {
            return invalid_configuration(
                request,
                std::move(probes),
                "Physical product index violated its current-source/key/0x0C normalization contract.");
        }
        if (lookup.ambiguous()) {
            return ambiguous_report(
                request,
                std::move(probes),
                lookup,
                "The product physical lookup exposes multiple physical identities for one 0x0C-normalized key; exact original type-0 filename comparison semantics remain unresolved.");
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
        .detail = "All canonical archive and physical lookup attempts completed without a resource hit. Physical probes use explicitly product-classified 0x0C lookup semantics pending exact type-0 backend reverse closure.",
    };
}

} // namespace dmc::rengine::profiles::dmc3
