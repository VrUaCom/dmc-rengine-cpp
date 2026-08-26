#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"
#include "dmc_rengine/profiles/dmc3/original_resolution_observation.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace {

using namespace dmc::rengine::profiles::dmc3;

[[nodiscard]] OriginalArchiveArtifactIdentity archive(
    std::uint32_t index,
    char sha_digit) {
    return OriginalArchiveArtifactIdentity{
        .volume_index = index,
        .filename = VolumeBootstrapPolicy::volume_filename(index),
        .sha256 = std::string(64U, sha_digit),
        .size = 1000U + index,
    };
}

[[nodiscard]] OriginalResolutionObservation archive_hit_observation() {
    const auto& executable = protected_distribution_executable();
    const auto plan = ResourceLookupPolicy::plan("scr\\st001.pac");
    assert(plan.valid());
    const auto& attempt = plan.attempts[0];
    const auto key = ResourcePathPolicy::archive(attempt.candidate);

    return OriginalResolutionObservation{
        .executable_sha256 = std::string{executable.sha256},
        .executable_size = executable.file_size,
        .runtime_mapping_packet_sha256 = std::string(64U, 'a'),
        .pid = 4242U,
        .module_base = 0x7FF600000000ULL,
        .flags = 1U,
        .request = "scr\\st001.pac",
        .basename = "st001.pac",
        .first_missing_archive_volume = 3U,
        .archives = {archive(0U, '1'), archive(1U, '2'), archive(2U, '3')},
        .probes = {
            OriginalResolutionProbe{
                .sequence_index = 0U,
                .lookup_attempt_index = 0U,
                .provider = ResourceProviderClass::archive,
                .candidate = attempt.candidate,
                .provider_key = key,
                .archive_volume_index = 2U,
                .outcome = OriginalResolutionProbeOutcome::miss,
            },
            OriginalResolutionProbe{
                .sequence_index = 1U,
                .lookup_attempt_index = 0U,
                .provider = ResourceProviderClass::archive,
                .candidate = attempt.candidate,
                .provider_key = key,
                .archive_volume_index = 1U,
                .outcome = OriginalResolutionProbeOutcome::selected,
            },
        },
        .selected = OriginalSelectedResourceIdentity{
            .provider = ResourceProviderClass::archive,
            .lookup_attempt_index = 0U,
            .candidate = attempt.candidate,
            .provider_key = key,
            .archive_volume_index = 1U,
            .archive_member_path = "GDataX360.afs/ST001.PAC",
            .physical_relative_path = {},
        },
    };
}

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string source_id,
    std::string logical_path) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = {},
            .offset = 0U,
            .size = 100U,
        },
        .display_name = "ST001.PAC",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };
}

} // namespace

int main() {
    using namespace dmc::rengine::profiles::dmc3;

    const auto valid = archive_hit_observation();
    assert(valid.valid());
    const auto json = original_resolution_observation_to_json(valid);
    assert(!json.empty());
    assert(json.find("dmc-rengine.gdspaces-l2-original-selection.v1") !=
           std::string::npos);
    assert(json.find("original-process-observation") != std::string::npos);
    assert(json.find("bytes_hex") == std::string::npos);

    // Analysis executable may not be laundered into an original-process receipt.
    {
        auto bad = valid;
        const auto& analysis = canonical_analysis_executable();
        bad.executable_sha256 = std::string{analysis.sha256};
        bad.executable_size = analysis.file_size;
        assert(!bad.valid());
    }

    // Mapping authority must be represented by one canonical SHA-256 digest.
    {
        auto bad = valid;
        bad.runtime_mapping_packet_sha256 = "not-a-sha";
        assert(!bad.valid());
    }

    // Archive precedence is exact: volume 2 cannot be silently skipped before
    // selecting volume 1.
    {
        auto bad = valid;
        bad.probes.erase(bad.probes.begin());
        bad.probes[0].sequence_index = 0U;
        assert(!bad.valid());
    }

    // A selected event terminates the trace; later evidence cannot be appended.
    {
        auto bad = valid;
        auto trailing = bad.probes.front();
        trailing.sequence_index = bad.probes.size();
        trailing.outcome = OriginalResolutionProbeOutcome::miss;
        bad.probes.push_back(std::move(trailing));
        assert(!bad.valid());
    }

    // Selected member identity must normalize to the provider key actually
    // observed at the selected probe.
    {
        auto bad = valid;
        bad.selected->archive_member_path = "GData.afs/other.pac";
        assert(!bad.valid());
    }

    // Duplicate/missing archive artifact identities cannot satisfy a contiguous
    // first-gap mount authority.
    {
        auto bad = valid;
        bad.archives[2].volume_index = 1U;
        bad.archives[2].filename = VolumeBootstrapPolicy::volume_filename(1U);
        assert(!bad.valid());
    }

    RuntimeSourceBindings bindings{
        .physical_source_id = "physical",
        .archives = {
            ArchiveSourceBinding{0U, "archive-0"},
            ArchiveSourceBinding{1U, "archive-1"},
            ArchiveSourceBinding{2U, "archive-2"},
        },
    };

    // Product comparison is a separate bounded result, not a promotion of the
    // product probe evidence class.
    {
        RuntimeResolutionReport product{
            .request = valid.request,
            .status = RuntimeResolutionStatus::resolved,
            .resolved = resource("archive-1", "GDataX360.afs/ST001.PAC"),
            .ambiguous_matches = {},
            .probes = {},
            .detail = {},
        };
        const auto comparison = compare_original_to_product(valid, product, bindings);
        assert(comparison.matched());

        product.resolved = resource("archive-0", "GDataX360.afs/ST001.PAC");
        const auto wrong_volume = compare_original_to_product(valid, product, bindings);
        assert(wrong_volume.status ==
               OriginalProductComparisonStatus::provider_identity_mismatch);

        product.resolved = resource("archive-1", "GDataX360.afs/OTHER.PAC");
        const auto wrong_member = compare_original_to_product(valid, product, bindings);
        assert(wrong_member.status ==
               OriginalProductComparisonStatus::resource_identity_mismatch);
    }

    // Physical selection is legal only after the entire archive phase and all
    // earlier physical candidates have missed. Zero mounted archives keeps this
    // test compact while still proving physical prefix order.
    {
        const auto& executable = protected_distribution_executable();
        const auto plan = ResourceLookupPolicy::plan("room\\loose.pac");
        assert(plan.valid());

        OriginalResolutionObservation physical{
            .executable_sha256 = std::string{executable.sha256},
            .executable_size = executable.file_size,
            .runtime_mapping_packet_sha256 = std::string(64U, 'b'),
            .pid = 5000U,
            .module_base = 0x7FF700000000ULL,
            .flags = 1U,
            .request = "room\\loose.pac",
            .basename = "loose.pac",
            .first_missing_archive_volume = 0U,
            .archives = {},
            .probes = {},
            .selected = std::nullopt,
        };

        // With zero archives the first six plan entries produce no provider
        // operations. Physical attempts begin at lookup attempt 6.
        for (std::size_t attempt_index = 6U; attempt_index <= 8U; ++attempt_index) {
            const auto& attempt = plan.attempts[attempt_index];
            physical.probes.push_back(OriginalResolutionProbe{
                .sequence_index = physical.probes.size(),
                .lookup_attempt_index = attempt_index,
                .provider = ResourceProviderClass::physical,
                .candidate = attempt.candidate,
                .provider_key = ResourcePathPolicy::physical(attempt.candidate),
                .archive_volume_index = std::nullopt,
                .outcome = attempt_index == 8U
                    ? OriginalResolutionProbeOutcome::selected
                    : OriginalResolutionProbeOutcome::miss,
            });
        }
        const auto& winner = physical.probes.back();
        physical.selected = OriginalSelectedResourceIdentity{
            .provider = ResourceProviderClass::physical,
            .lookup_attempt_index = winner.lookup_attempt_index,
            .candidate = winner.candidate,
            .provider_key = winner.provider_key,
            .archive_volume_index = std::nullopt,
            .archive_member_path = {},
            .physical_relative_path = winner.candidate,
        };
        assert(physical.valid());

        RuntimeSourceBindings physical_bindings{
            .physical_source_id = "physical",
            .archives = {},
        };
        RuntimeResolutionReport product{
            .request = physical.request,
            .status = RuntimeResolutionStatus::resolved,
            .resolved = resource("physical", winner.candidate),
            .ambiguous_matches = {},
            .probes = {},
            .detail = {},
        };
        assert(compare_original_to_product(
                   physical, product, physical_bindings).matched());

        auto skipped = physical;
        skipped.probes.erase(skipped.probes.begin());
        for (std::size_t index = 0U; index < skipped.probes.size(); ++index) {
            skipped.probes[index].sequence_index = index;
        }
        assert(!skipped.valid());
    }

    return 0;
}
