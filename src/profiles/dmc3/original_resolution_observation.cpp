#include "dmc_rengine/profiles/dmc3/original_resolution_observation.hpp"

#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool canonical_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    for (const auto ch : value) {
        if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (ch < 0x20U) {
                output << "\\u00"
                       << hex[(ch >> 4U) & 0x0FU]
                       << hex[ch & 0x0FU];
            } else {
                output << static_cast<char>(ch);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] std::string hex_u64(std::uint64_t value) {
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << value;
    return output.str();
}

[[nodiscard]] bool protected_execution_authority(
    const OriginalResolutionObservation& observation) noexcept {
    const auto match = classify_executable_authority(
        observation.executable_sha256, observation.executable_size);
    if (!match.recognized()) {
        return false;
    }

    const auto& expected = protected_distribution_executable();
    return match.authority == &expected &&
        expected.role == ExecutableAuthorityRole::protected_distribution &&
        expected.distribution_provenance_authority &&
        expected.original_execution_candidate &&
        !expected.instruction_reverse_authority;
}

[[nodiscard]] const OriginalArchiveArtifactIdentity* archive_identity(
    const OriginalResolutionObservation& observation,
    std::uint32_t volume_index) noexcept {
    const auto iterator = std::find_if(
        observation.archives.begin(), observation.archives.end(),
        [volume_index](const OriginalArchiveArtifactIdentity& archive) {
            return archive.volume_index == volume_index;
        });
    return iterator == observation.archives.end() ? nullptr : &*iterator;
}

[[nodiscard]] bool selected_matches_probe(
    const OriginalSelectedResourceIdentity& selected,
    const OriginalResolutionProbe& probe) noexcept {
    if (selected.provider != probe.provider ||
        selected.lookup_attempt_index != probe.lookup_attempt_index ||
        selected.candidate != probe.candidate ||
        selected.provider_key != probe.provider_key) {
        return false;
    }

    if (selected.provider == ResourceProviderClass::archive) {
        return selected.archive_volume_index == probe.archive_volume_index;
    }
    return !probe.archive_volume_index.has_value();
}

[[nodiscard]] bool selected_identity_matches_provider_key(
    const OriginalSelectedResourceIdentity& selected) noexcept {
    switch (selected.provider) {
    case ResourceProviderClass::archive:
        return ResourcePathPolicy::archive(selected.archive_member_path) ==
            selected.provider_key;
    case ResourceProviderClass::physical:
        return ResourcePathPolicy::physical(selected.physical_relative_path) ==
            selected.provider_key;
    }
    return false;
}

} // namespace

bool OriginalArchiveArtifactIdentity::valid() const noexcept {
    return VolumeBootstrapPolicy::runtime_index_valid(volume_index) &&
        filename == VolumeBootstrapPolicy::volume_filename(volume_index) &&
        canonical_sha256(sha256) && size != 0U;
}

bool OriginalResolutionProbe::valid_shape() const noexcept {
    if (lookup_attempt_index >= 12U || candidate.empty() ||
        provider_key.empty()) {
        return false;
    }

    switch (provider) {
    case ResourceProviderClass::archive:
        return archive_volume_index.has_value() &&
            VolumeBootstrapPolicy::runtime_index_valid(*archive_volume_index);
    case ResourceProviderClass::physical:
        return !archive_volume_index.has_value();
    }
    return false;
}

bool OriginalSelectedResourceIdentity::valid_shape() const noexcept {
    if (lookup_attempt_index >= 12U || candidate.empty() ||
        provider_key.empty()) {
        return false;
    }

    switch (provider) {
    case ResourceProviderClass::archive:
        return archive_volume_index.has_value() &&
            VolumeBootstrapPolicy::runtime_index_valid(*archive_volume_index) &&
            !archive_member_path.empty() && physical_relative_path.empty();
    case ResourceProviderClass::physical:
        return !archive_volume_index.has_value() &&
            archive_member_path.empty() && !physical_relative_path.empty();
    }
    return false;
}

bool OriginalResolutionObservation::valid() const noexcept {
    if (!protected_execution_authority(*this) ||
        !canonical_sha256(runtime_mapping_packet_sha256) ||
        pid == 0U || module_base == 0U || flags != 1U || request.empty() ||
        basename.empty() || !selected.has_value() ||
        !selected->valid_shape() ||
        first_missing_archive_volume > VolumeBootstrapPolicy::runtime_index_max() ||
        archives.size() != static_cast<std::size_t>(first_missing_archive_volume)) {
        return false;
    }

    const auto plan = ResourceLookupPolicy::plan(request);
    if (!plan.valid() || plan.basename != basename) {
        return false;
    }

    for (std::uint32_t index = 0U;
         index < first_missing_archive_volume;
         ++index) {
        const auto* identity = archive_identity(*this, index);
        if (identity == nullptr || !identity->valid()) {
            return false;
        }
    }
    for (std::size_t index = 0U; index < archives.size(); ++index) {
        for (std::size_t other = index + 1U; other < archives.size(); ++other) {
            if (archives[index].volume_index == archives[other].volume_index) {
                return false;
            }
        }
    }

    if (probes.empty()) {
        return false;
    }

    std::size_t observed_index = 0U;
    for (const auto& attempt : plan.attempts) {
        if (attempt.provider == ResourceProviderClass::archive) {
            for (std::uint32_t remaining = first_missing_archive_volume;
                 remaining > 0U;
                 --remaining) {
                if (observed_index >= probes.size()) {
                    return false;
                }
                const auto& probe = probes[observed_index];
                const auto volume_index = remaining - 1U;
                const auto expected_key = ResourcePathPolicy::archive(attempt.candidate);
                if (!probe.valid_shape() ||
                    probe.sequence_index != observed_index ||
                    probe.lookup_attempt_index != attempt.attempt_index ||
                    probe.provider != ResourceProviderClass::archive ||
                    probe.candidate != attempt.candidate ||
                    probe.provider_key != expected_key ||
                    probe.archive_volume_index != volume_index) {
                    return false;
                }

                if (probe.outcome == OriginalResolutionProbeOutcome::selected) {
                    return observed_index + 1U == probes.size() &&
                        selected_matches_probe(*selected, probe) &&
                        selected_identity_matches_provider_key(*selected);
                }
                ++observed_index;
            }
            continue;
        }

        if (observed_index >= probes.size()) {
            return false;
        }
        const auto& probe = probes[observed_index];
        const auto expected_key = ResourcePathPolicy::physical(attempt.candidate);
        if (!probe.valid_shape() || probe.sequence_index != observed_index ||
            probe.lookup_attempt_index != attempt.attempt_index ||
            probe.provider != ResourceProviderClass::physical ||
            probe.candidate != attempt.candidate ||
            probe.provider_key != expected_key || probe.archive_volume_index.has_value()) {
            return false;
        }

        if (probe.outcome == OriginalResolutionProbeOutcome::selected) {
            return observed_index + 1U == probes.size() &&
                selected_matches_probe(*selected, probe) &&
                selected_identity_matches_provider_key(*selected);
        }
        ++observed_index;
    }

    return false;
}

std::string original_resolution_observation_to_json(
    const OriginalResolutionObservation& observation) {
    if (!observation.valid()) {
        return {};
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema\": \"dmc-rengine.gdspaces-l2-original-selection.v1\",\n"
           << "  \"evidence_class\": \"original-process-observation\",\n"
           << "  \"executable_sha256\": \""
           << observation.executable_sha256 << "\",\n"
           << "  \"executable_size\": " << observation.executable_size << ",\n"
           << "  \"runtime_mapping_packet_sha256\": \""
           << observation.runtime_mapping_packet_sha256 << "\",\n"
           << "  \"pid\": " << observation.pid << ",\n"
           << "  \"module_base\": \"" << hex_u64(observation.module_base)
           << "\",\n"
           << "  \"flags\": " << observation.flags << ",\n"
           << "  \"request\": \"" << escape_json(observation.request) << "\",\n"
           << "  \"basename\": \"" << escape_json(observation.basename) << "\",\n"
           << "  \"first_missing_archive_volume\": "
           << observation.first_missing_archive_volume << ",\n"
           << "  \"archives\": [";

    for (std::size_t index = 0U; index < observation.archives.size(); ++index) {
        const auto& archive = observation.archives[index];
        output << (index == 0U ? "\n" : ",\n")
               << "    {\"volume_index\": " << archive.volume_index
               << ", \"filename\": \"" << escape_json(archive.filename)
               << "\", \"sha256\": \"" << archive.sha256
               << "\", \"size\": " << archive.size << '}';
    }
    if (!observation.archives.empty()) {
        output << '\n';
    }
    output << "  ],\n  \"probes\": [";

    for (std::size_t index = 0U; index < observation.probes.size(); ++index) {
        const auto& probe = observation.probes[index];
        output << (index == 0U ? "\n" : ",\n")
               << "    {\"sequence_index\": " << probe.sequence_index
               << ", \"lookup_attempt_index\": " << probe.lookup_attempt_index
               << ", \"provider\": \"" << to_string(probe.provider)
               << "\", \"candidate\": \"" << escape_json(probe.candidate)
               << "\", \"provider_key\": \"" << escape_json(probe.provider_key)
               << "\", \"archive_volume_index\": ";
        if (probe.archive_volume_index.has_value()) {
            output << *probe.archive_volume_index;
        } else {
            output << "null";
        }
        output << ", \"outcome\": \"" << to_string(probe.outcome) << "\"}";
    }
    output << "\n  ],\n";

    const auto& selected = *observation.selected;
    output << "  \"selected\": {\n"
           << "    \"provider\": \"" << to_string(selected.provider) << "\",\n"
           << "    \"lookup_attempt_index\": "
           << selected.lookup_attempt_index << ",\n"
           << "    \"candidate\": \"" << escape_json(selected.candidate) << "\",\n"
           << "    \"provider_key\": \"" << escape_json(selected.provider_key)
           << "\",\n"
           << "    \"archive_volume_index\": ";
    if (selected.archive_volume_index.has_value()) {
        output << *selected.archive_volume_index;
    } else {
        output << "null";
    }
    output << ",\n    \"archive_member_path\": \""
           << escape_json(selected.archive_member_path) << "\",\n"
           << "    \"physical_relative_path\": \""
           << escape_json(selected.physical_relative_path) << "\"\n"
           << "  },\n"
           << "  \"proves\": [\n"
           << "    \"original-process-provider-traversal-prefix\",\n"
           << "    \"original-process-selected-resource-identity\"\n"
           << "  ],\n"
           << "  \"does_not_prove\": [\n"
           << "    \"runtime-address-mapping-without-the-bound-mapping-packet\",\n"
           << "    \"retail-archive-collision-freedom\",\n"
           << "    \"product-original-global-equivalence\",\n"
           << "    \"layer-1-or-layer-3-completion\"\n"
           << "  ]\n"
           << "}\n";
    return output.str();
}

OriginalProductResolutionComparison compare_original_to_product(
    const OriginalResolutionObservation& original,
    const RuntimeResolutionReport& product,
    const RuntimeSourceBindings& bindings) noexcept {
    if (!original.valid()) {
        return {
            .status = OriginalProductComparisonStatus::invalid_original_observation,
            .detail = "Original-process observation failed its authority/order/identity contract.",
        };
    }
    if (!product.ok() || !product.resolved.has_value()) {
        return {
            .status = OriginalProductComparisonStatus::product_not_resolved,
            .detail = "GDSpaces product resolution did not produce one selected ResourceRef.",
        };
    }
    if (product.request != original.request) {
        return {
            .status = OriginalProductComparisonStatus::resource_identity_mismatch,
            .detail = "Product and original observations do not refer to the same logical request.",
        };
    }

    const auto& selected = *original.selected;
    const auto& resolved = *product.resolved;
    switch (selected.provider) {
    case ResourceProviderClass::archive: {
        const auto* binding = bindings.archive(*selected.archive_volume_index);
        if (binding == nullptr || resolved.id.source_id != binding->source_id) {
            return {
                .status = OriginalProductComparisonStatus::provider_identity_mismatch,
                .detail = "GDSpaces selected a different archive source/volume identity.",
            };
        }
        if (resolved.id.logical_path != selected.archive_member_path) {
            return {
                .status = OriginalProductComparisonStatus::resource_identity_mismatch,
                .detail = "GDSpaces archive ResourceRef does not match the exact observed member identity.",
            };
        }
        break;
    }
    case ResourceProviderClass::physical:
        if (resolved.id.source_id != bindings.physical_source_id) {
            return {
                .status = OriginalProductComparisonStatus::provider_identity_mismatch,
                .detail = "GDSpaces selected a non-physical source while the original selected the physical provider.",
            };
        }
        if (resolved.id.logical_path != selected.physical_relative_path) {
            return {
                .status = OriginalProductComparisonStatus::resource_identity_mismatch,
                .detail = "GDSpaces physical ResourceRef does not match the observed mounted-root-relative identity.",
            };
        }
        break;
    }

    return {
        .status = OriginalProductComparisonStatus::matched,
        .detail = "Original-process and GDSpaces selected provider/resource identities match at the bounded request scope.",
    };
}

} // namespace dmc::rengine::profiles::dmc3
