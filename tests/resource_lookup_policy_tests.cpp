#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"
#include "dmc_rengine/profiles/dmc3/original_resolution_observation.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;
namespace gdspaces = dmc::rengine::gdspaces;

[[nodiscard]] dmc3::OriginalArchiveArtifactIdentity archive(
    std::uint32_t index,
    char sha_digit) {
    return dmc3::OriginalArchiveArtifactIdentity{
        .volume_index = index,
        .filename = dmc3::VolumeBootstrapPolicy::volume_filename(index),
        .sha256 = std::string(64U, sha_digit),
        .size = 1000U + index,
    };
}

[[nodiscard]] dmc3::OriginalResolutionObservation archive_hit_observation() {
    const auto& executable = dmc3::protected_distribution_executable();
    const auto plan = dmc3::ResourceLookupPolicy::plan("scr\\st001.pac");
    assert(plan.valid());
    const auto& attempt = plan.attempts[0];
    const auto key = dmc3::ResourcePathPolicy::archive(attempt.candidate);

    return dmc3::OriginalResolutionObservation{
        .executable_sha256 = std::string{executable.sha256},
        .executable_size = executable.file_size,
        .runtime_mapping_packet_sha256 = std::string(64U, 'a'),
        .observer_id = "dmc-rengine-l2-observer",
        .observer_version = "test-contract-v2",
        .observer_build_sha256 = std::string(64U, 'c'),
        .trace_complete = true,
        .dropped_event_count = 0U,
        .pid = 4242U,
        .process_creation_filetime = 0x01DA000000004242ULL,
        .module_base = 0x7FF600000000ULL,
        .flags = 1U,
        .request = "scr\\st001.pac",
        .basename = "st001.pac",
        .first_missing_archive_volume = 3U,
        .archives = {archive(0U, '1'), archive(1U, '2'), archive(2U, '3')},
        .probes = {
            dmc3::OriginalResolutionProbe{
                .sequence_index = 0U,
                .lookup_attempt_index = 0U,
                .provider = dmc3::ResourceProviderClass::archive,
                .candidate = attempt.candidate,
                .provider_key = key,
                .archive_volume_index = 2U,
                .outcome = dmc3::OriginalResolutionProbeOutcome::miss,
            },
            dmc3::OriginalResolutionProbe{
                .sequence_index = 1U,
                .lookup_attempt_index = 0U,
                .provider = dmc3::ResourceProviderClass::archive,
                .candidate = attempt.candidate,
                .provider_key = key,
                .archive_volume_index = 1U,
                .outcome = dmc3::OriginalResolutionProbeOutcome::selected,
            },
        },
        .selected = dmc3::OriginalSelectedResourceIdentity{
            .provider = dmc3::ResourceProviderClass::archive,
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
    using dmc3::ResourceLookupPolicy;
    using dmc3::ResourceProviderClass;

    const auto& prefixes = ResourceLookupPolicy::namespace_prefixes();
    assert(prefixes.size() == 6U);
    assert(prefixes[0] == "GDataX360.afs/");
    assert(prefixes[1] == "GData.afs/");
    assert(prefixes[2] == "Video/");
    assert(prefixes[3] == "afs/sound/");
    assert(prefixes[4] == "SAVEDATA/");
    assert(prefixes[5].empty());

    assert(ResourceLookupPolicy::basename_of("scr\\st001.pac") == "st001.pac");
    assert(ResourceLookupPolicy::basename_of("room/st001cfg.pac") == "st001cfg.pac");
    assert(ResourceLookupPolicy::basename_of("st001.pac") == "st001.pac");
    assert(ResourceLookupPolicy::basename_of("room/").empty());

    const auto plan = ResourceLookupPolicy::plan("folder/sub\\st001.pac");
    assert(plan.valid());
    assert(plan.original_request == "folder/sub\\st001.pac");
    assert(plan.basename == "st001.pac");
    assert(plan.attempts.size() == 12U);

    for (std::size_t index = 0U; index < 6U; ++index) {
        const auto& attempt = plan.attempts[index];
        assert(attempt.attempt_index == index);
        assert(attempt.provider == ResourceProviderClass::archive);
        assert(attempt.provider_mask == 1U);
        assert(attempt.prefix_index == index);
        assert(attempt.prefix == prefixes[index]);
        assert(attempt.candidate == std::string{prefixes[index]} + "st001.pac");
    }
    for (std::size_t index = 0U; index < 6U; ++index) {
        const auto& attempt = plan.attempts[index + 6U];
        assert(attempt.attempt_index == index + 6U);
        assert(attempt.provider == ResourceProviderClass::physical);
        assert(attempt.provider_mask == 2U);
        assert(attempt.prefix_index == index);
        assert(attempt.prefix == prefixes[index]);
        assert(attempt.candidate == std::string{prefixes[index]} + "st001.pac");
    }

    const auto mixed_case = ResourceLookupPolicy::plan("ROOM\\St001CFG.PAC");
    assert(mixed_case.valid());
    assert(mixed_case.basename == "St001CFG.PAC");
    assert(mixed_case.attempts[0].candidate == "GDataX360.afs/St001CFG.PAC");

    const auto empty = ResourceLookupPolicy::plan("");
    assert(!empty.valid());
    assert(empty.attempts.empty());

    const std::string embedded_nul{"room/st001\0evil.pac", 19U};
    const auto nul_rejected = ResourceLookupPolicy::plan(embedded_nul);
    assert(!nul_rejected.valid());
    assert(nul_rejected.attempts.empty());
    assert(ResourceLookupPolicy::basename_of(embedded_nul).empty());

    auto forged_request = plan;
    forged_request.original_request = "other/st001cfg.pac";
    assert(!forged_request.valid());

    const std::string too_long(ResourceLookupPolicy::candidate_buffer_bytes, 'x');
    const auto rejected = ResourceLookupPolicy::plan(too_long);
    assert(!rejected.valid());
    assert(rejected.attempts.empty());

    const std::string prefix_overflow(
        ResourceLookupPolicy::candidate_buffer_bytes - 4U, 'y');
    const auto prefix_rejected = ResourceLookupPolicy::plan(prefix_overflow);
    assert(!prefix_rejected.valid());
    assert(prefix_rejected.attempts.empty());

    // R3 original-process observation guardrails. Synthetic construction here
    // validates only the receipt contract; it is not original-process evidence.
    const auto valid = archive_hit_observation();
    assert(valid.valid());
    const auto json = dmc3::original_resolution_observation_to_json(valid);
    assert(!json.empty());
    assert(json.find("dmc-rengine.gdspaces-l2-original-selection.v2") !=
           std::string::npos);
    assert(json.find("original-process-observation") != std::string::npos);
    assert(json.find("dmc-rengine-l2-observer") != std::string::npos);
    assert(json.find("\"trace_complete\": true") != std::string::npos);
    assert(json.find("\"process_creation_filetime\": 133419138960867906") !=
           std::string::npos);
    assert(json.find("bytes_hex") == std::string::npos);

    // Analysis EXE cannot be laundered into protected original-process evidence.
    {
        auto bad = valid;
        const auto& analysis = dmc3::canonical_analysis_executable();
        bad.executable_sha256 = std::string{analysis.sha256};
        bad.executable_size = analysis.file_size;
        assert(!bad.valid());
    }

    // Authority recognition is case-tolerant elsewhere, but evidence receipts
    // require one canonical lowercase SHA spelling.
    {
        auto bad = valid;
        for (auto& ch : bad.executable_sha256) {
            if (ch >= 'a' && ch <= 'f') {
                ch = static_cast<char>(ch - 'a' + 'A');
            }
        }
        assert(!bad.valid());
    }

    {
        auto bad = valid;
        bad.runtime_mapping_packet_sha256 = "not-a-sha";
        assert(!bad.valid());
    }

    {
        auto bad = valid;
        bad.observer_version.clear();
        assert(!bad.valid());
    }

    {
        auto bad = valid;
        bad.observer_build_sha256 = "not-a-build-sha";
        assert(!bad.valid());
    }

    // Process-instance identity is mandatory in the v2 content contract.
    {
        auto bad = valid;
        bad.process_creation_filetime = 0U;
        assert(!bad.valid());
    }

    // An incomplete or lossy trace cannot support a winner claim.
    {
        auto bad = valid;
        bad.trace_complete = false;
        assert(!bad.valid());
    }
    {
        auto bad = valid;
        bad.dropped_event_count = 1U;
        assert(!bad.valid());
    }

    // Volume 2 is higher precedence than volume 1 and cannot be omitted.
    {
        auto bad = valid;
        bad.probes.erase(bad.probes.begin());
        bad.probes[0].sequence_index = 0U;
        assert(!bad.valid());
    }

    // No event is legal after the selected probe.
    {
        auto bad = valid;
        auto trailing = bad.probes.front();
        trailing.sequence_index = bad.probes.size();
        trailing.outcome = dmc3::OriginalResolutionProbeOutcome::miss;
        bad.probes.push_back(std::move(trailing));
        assert(!bad.valid());
    }

    // Selected physical/member identity must normalize to the selected key.
    {
        auto bad = valid;
        bad.selected->archive_member_path = "GData.afs/other.pac";
        assert(!bad.valid());
    }

    // Contiguous volume artifact identity cannot contain a duplicate index.
    {
        auto bad = valid;
        bad.archives[2].volume_index = 1U;
        bad.archives[2].filename = dmc3::VolumeBootstrapPolicy::volume_filename(1U);
        assert(!bad.valid());
    }

    dmc3::RuntimeSourceBindings bindings{
        .physical_source_id = "physical",
        .archives = {
            dmc3::ArchiveSourceBinding{0U, "archive-0"},
            dmc3::ArchiveSourceBinding{1U, "archive-1"},
            dmc3::ArchiveSourceBinding{2U, "archive-2"},
        },
    };

    {
        dmc3::RuntimeResolutionReport product{
            .request = valid.request,
            .status = dmc3::RuntimeResolutionStatus::resolved,
            .resolved = resource("archive-1", "GDataX360.afs/ST001.PAC"),
            .ambiguous_matches = {},
            .probes = {},
            .detail = {},
        };
        assert(dmc3::compare_original_to_product(
                   valid, product, bindings).candidate_content_matched());

        auto invalid_bindings = bindings;
        invalid_bindings.archives[2].source_id = "archive-1";
        assert(dmc3::compare_original_to_product(
                   valid, product, invalid_bindings).status ==
               dmc3::OriginalProductComparisonStatus::invalid_product_configuration);

        product.resolved = resource("archive-0", "GDataX360.afs/ST001.PAC");
        assert(dmc3::compare_original_to_product(valid, product, bindings).status ==
               dmc3::OriginalProductComparisonStatus::provider_identity_mismatch);

        product.resolved = resource("archive-1", "GDataX360.afs/OTHER.PAC");
        assert(dmc3::compare_original_to_product(valid, product, bindings).status ==
               dmc3::OriginalProductComparisonStatus::resource_identity_mismatch);
    }

    // Zero mounted archives still preserve recovered provider-phase indexing:
    // physical candidates are attempts 6..11 and prefixes cannot be skipped.
    {
        const auto& executable = dmc3::protected_distribution_executable();
        const auto physical_plan = ResourceLookupPolicy::plan("room\\loose.pac");
        assert(physical_plan.valid());

        dmc3::OriginalResolutionObservation physical{
            .executable_sha256 = std::string{executable.sha256},
            .executable_size = executable.file_size,
            .runtime_mapping_packet_sha256 = std::string(64U, 'b'),
            .observer_id = "dmc-rengine-l2-observer",
            .observer_version = "test-contract-v2",
            .observer_build_sha256 = std::string(64U, 'd'),
            .trace_complete = true,
            .dropped_event_count = 0U,
            .pid = 5000U,
            .process_creation_filetime = 0x01DA000000005000ULL,
            .module_base = 0x7FF700000000ULL,
            .flags = 1U,
            .request = "room\\loose.pac",
            .basename = "loose.pac",
            .first_missing_archive_volume = 0U,
            .archives = {},
            .probes = {},
            .selected = std::nullopt,
        };

        for (std::size_t attempt_index = 6U; attempt_index <= 8U; ++attempt_index) {
            const auto& attempt = physical_plan.attempts[attempt_index];
            physical.probes.push_back(dmc3::OriginalResolutionProbe{
                .sequence_index = physical.probes.size(),
                .lookup_attempt_index = attempt_index,
                .provider = ResourceProviderClass::physical,
                .candidate = attempt.candidate,
                .provider_key = dmc3::ResourcePathPolicy::physical(attempt.candidate),
                .archive_volume_index = std::nullopt,
                .outcome = attempt_index == 8U
                    ? dmc3::OriginalResolutionProbeOutcome::selected
                    : dmc3::OriginalResolutionProbeOutcome::miss,
            });
        }
        const auto& winner = physical.probes.back();
        physical.selected = dmc3::OriginalSelectedResourceIdentity{
            .provider = ResourceProviderClass::physical,
            .lookup_attempt_index = winner.lookup_attempt_index,
            .candidate = winner.candidate,
            .provider_key = winner.provider_key,
            .archive_volume_index = std::nullopt,
            .archive_member_path = {},
            .physical_relative_path = winner.candidate,
        };
        assert(physical.valid());

        dmc3::RuntimeSourceBindings physical_bindings{
            .physical_source_id = "physical",
            .archives = {},
        };
        dmc3::RuntimeResolutionReport product{
            .request = physical.request,
            .status = dmc3::RuntimeResolutionStatus::resolved,
            .resolved = resource("physical", winner.candidate),
            .ambiguous_matches = {},
            .probes = {},
            .detail = {},
        };
        assert(dmc3::compare_original_to_product(
                   physical, product, physical_bindings).candidate_content_matched());

        auto skipped = physical;
        skipped.probes.erase(skipped.probes.begin());
        for (std::size_t index = 0U; index < skipped.probes.size(); ++index) {
            skipped.probes[index].sequence_index = index;
        }
        assert(!skipped.valid());
    }

    return 0;
}
