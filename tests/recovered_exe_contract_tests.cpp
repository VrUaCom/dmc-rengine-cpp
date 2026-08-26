#include "dmc_rengine/profiles/dmc3/archive_entry_read_contract.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_contract.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"
#include "dmc_rengine/profiles/dmc3/open_game_resource_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_bootstrap_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// The contracts state what the executable does. These check that the product
// implementations actually behave that way — the compile-time assertions can
// only prove the numbers agree, not that the code built from them produces the
// recovered shape.

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;

void request_path_matches_the_recovered_shape() {
    const auto plan = dmc3::ResourceLookupPolicy::plan("obj/em000.pac");
    assert(plan.valid());
    assert(plan.basename == "em000.pac");
    assert(plan.attempts.size() ==
        dmc3::OpenGameResourceContract::attempts_per_request);

    // One complete archive pass over the six prefixes, then one physical pass
    // over the same six, each carrying the mask its pass uses.
    const auto& prefixes = dmc3::OpenGameResourceContract::namespace_prefixes;
    for (std::size_t index = 0U; index < plan.attempts.size(); ++index) {
        const auto& attempt = plan.attempts[index];
        const auto pass = index / prefixes.size();
        assert(attempt.attempt_index == index);
        assert(attempt.prefix == prefixes[index % prefixes.size()]);
        assert(attempt.provider_mask ==
            dmc3::OpenGameResourceContract::provider_mask_for_pass(pass));
        assert(attempt.candidate == attempt.prefix + plan.basename);
    }

    // The basename is taken after the last separator of either spelling, and
    // its bytes are preserved: normalization belongs to the provider, later.
    assert(dmc3::ResourceLookupPolicy::plan("A\\B/MiXeD.Pac").basename ==
        "MiXeD.Pac");
}

void overflow_aborts_the_whole_request() {
    // The recovered path builds the first and longest candidate first, and a
    // candidate that will not fit the 0x400 destination releases the slot and
    // returns the miss value. It does not try a shorter prefix, so the longest
    // prefix alone fixes the largest basename the request can ever carry.
    constexpr auto limit = dmc3::OpenGameResourceContract::max_basename_bytes();
    assert(limit == 1009U);

    const auto fits = dmc3::ResourceLookupPolicy::plan(std::string(limit, 'a'));
    assert(fits.valid());
    assert(fits.attempts.size() ==
        dmc3::OpenGameResourceContract::attempts_per_request);

    // One byte more overflows the first candidate. A plan with no attempts is
    // the product's fail-closed spelling of the recovered abort, and it must
    // not degrade into a shorter-prefix retry that would resolve something the
    // original runtime never could.
    const auto overflows =
        dmc3::ResourceLookupPolicy::plan(std::string(limit + 1U, 'a'));
    assert(!overflows.valid());
    assert(overflows.attempts.empty());
    assert(!dmc3::OpenGameResourceOverflowBehavior::advances_prefix_index);
    assert(!dmc3::OpenGameResourceOverflowBehavior::enters_physical_pass);
}

void bootstrap_matches_the_recovered_shape() {
    // Contiguous volumes register ascending and resolve descending, because
    // every mount node is prepended.
    const auto plan = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U, 1U, 2U});
    assert(plan.valid());
    assert(plan.registered_archives.size() == 3U);
    assert(plan.physical_root_registered_before_archives ==
        dmc3::Dmc3ResourceBootstrapContract::physical_root_registered_first);
    assert(plan.mount_list_is_prepend ==
        dmc3::Dmc3ResourceBootstrapContract::mount_list_is_prepend);
    assert((plan.archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 1U, 0U}));

    for (const auto& volume : plan.registered_archives) {
        assert(volume.resolution_rank ==
            dmc3::Dmc3ResourceBootstrapContract::resolution_rank(
                volume.index, 3U));
    }

    // Probing stops at the first gap; what lies past it is discovery evidence,
    // never a mount the recovered runtime could reach.
    const auto gapped = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U, 2U});
    assert(gapped.registered_archives.size() == 1U);
    assert(gapped.first_missing_index == 1U);
    assert((gapped.present_after_first_gap == std::vector<std::uint32_t>{2U}));

    // `%d` is signed decimal, so a suffix past INT32_MAX is a file the product
    // may find but not a name this runtime could have produced.
    assert(dmc3::Dmc3ResourceBootstrapContract::in_runtime_index_domain(
        dmc3::Dmc3ResourceBootstrapContract::runtime_index_max));
    assert(!dmc3::Dmc3ResourceBootstrapContract::in_runtime_index_domain(
        dmc3::Dmc3ResourceBootstrapContract::runtime_index_max + 1U));
    assert(dmc3::VolumeBootstrapPolicy::volume_filename(
        dmc3::Dmc3ResourceBootstrapContract::runtime_index_max) ==
        "DMC3-2147483647.nbz");
    assert(dmc3::VolumeBootstrapPolicy::volume_filename(
        dmc3::Dmc3ResourceBootstrapContract::runtime_index_max + 1U).empty());
}

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

void loose_container_matches_the_recovered_grammar() {
    // The correction the contract exists to hold: `/` comments and `#`
    // directs. An older shorthand had these the other way round, and copying
    // it would turn every directive into a comment and every comment into a
    // magic.
    const auto document = dmc3::LooseContainerListPolicy::parse(
        bytes_of("#PNST\r\n/ a comment line\r\nem000.mod\r\n"));
    assert(document.magic_from_directive);
    assert(document.magic[0] == static_cast<std::byte>('P'));
    assert(document.magic[1] == static_cast<std::byte>('N'));

    // Only the declared child slot counts. The directive and the comment do
    // not.
    assert(document.entries.size() == 1U);
    assert(document.entries.front().token == "em000.mod");
    assert(!dmc3::LooseContainerContract::directive_increments_slot_count);
    assert(!dmc3::LooseContainerContract::comment_increments_slot_count);

    // With no directive the synthesized container takes the default magic, so
    // `#PNST` above is an ordinary four-byte capture rather than a special
    // parser.
    const auto plain = dmc3::LooseContainerListPolicy::parse(
        bytes_of("em000.mod\r\n"));
    assert(!plain.magic_from_directive);
    for (std::size_t index = 0U;
         index < dmc3::LooseContainerContract::directive_magic_bytes;
         ++index) {
        assert(plain.magic[index] == static_cast<std::byte>(
            dmc3::LooseContainerContract::default_magic[index]));
    }

    // An LF-only normal line is not proven equivalent, so it fails closed
    // instead of being advertised as original-compatible.
    const auto lf_only =
        dmc3::LooseContainerListPolicy::parse(bytes_of("em000.mod\n"));
    assert(lf_only.status == dmc3::LooseContainerStatus::lf_only_normal_line);
    assert(!dmc3::LooseContainerContract::lf_only_normal_line_is_equivalent);

    // The rewrite replaces an existing extension. A path with no extension
    // boundary cannot enter the fallback at all, so it has no `.lst` form.
    assert(dmc3::LooseContainerListPolicy::list_path_for("obj/em000.pac") ==
        "obj/em000.lst");
    assert(!dmc3::LooseContainerListPolicy::list_path_for("obj/em000").has_value());
    assert(dmc3::LooseContainerContract::requires_existing_extension);
}

void archive_entry_read_matches_the_recovered_branch() {
    // The branch key is the inflater context, not the member's compression
    // method. A stored member that has been given a context still goes through
    // the inflater, and that is the recovered behavior rather than an oddity to
    // normalize away.
    assert(dmc3::ArchiveEntryReadContract::takes_inflated_branch(true));
    assert(!dmc3::ArchiveEntryReadContract::takes_inflated_branch(false));

    // Direct branch arithmetic: remaining is total minus consumed, the request
    // is clamped to it, and the cursor advances by what the backend actually
    // returned.
    const auto ordinary = dmc3::ArchiveDirectReadModel::plan(100U, 30U, 20U, 20);
    assert(ordinary.remaining == 70U);
    assert(ordinary.clamped_size == 20U);
    assert(ordinary.reaches_backend);
    assert(ordinary.next_consumed == 50U);

    const auto clamped = dmc3::ArchiveDirectReadModel::plan(100U, 95U, 40U, 5);
    assert(clamped.clamped_size == 5U);
    assert(clamped.next_consumed == 100U);

    // Exhausted is an answer, not a failure, and it never reaches the backend.
    const auto exhausted = dmc3::ArchiveDirectReadModel::plan(100U, 100U, 16U);
    assert(exhausted.remaining == 0U);
    assert(exhausted.clamped_size == 0U);
    assert(!exhausted.reaches_backend);
    assert(exhausted.result == dmc3::ArchiveEntryReadContract::exhausted_result);

    // A backend error is returned unchanged and the cursor does not move.
    const auto failed = dmc3::ArchiveDirectReadModel::plan(100U, 40U, 16U, -1);
    assert(failed.result == -1);
    assert(failed.next_consumed == 40U);
    assert(!dmc3::ArchiveEntryReadContract::advances_cursor_on_negative_read);
    assert(!dmc3::ArchiveEntryReadContract::translates_backend_error);

    // A short read advances by the short count, so the next request sees the
    // real position rather than the optimistic one.
    const auto short_read = dmc3::ArchiveDirectReadModel::plan(100U, 0U, 64U, 12);
    assert(short_read.next_consumed == 12U);
}

void contracts_are_bound_to_one_image() {
    // Two contracts recovered from the same binary must say so identically.
    // Addresses from different images cannot be reasoned about together, and
    // an unbound address is a number rather than evidence.
    assert(dmc3::OpenGameResourceContract::canonical_target_sha256 ==
        dmc3::Dmc3ResourceBootstrapContract::canonical_target_sha256);
    assert(dmc3::OpenGameResourceContract::image_base ==
        dmc3::Dmc3ResourceBootstrapContract::image_base);
    assert(dmc3::OpenGameResourceContract::canonical_target_sha256 ==
        dmc3::LooseContainerContract::canonical_target_sha256);
    assert(dmc3::OpenGameResourceContract::image_base ==
        dmc3::LooseContainerContract::image_base);
    assert(dmc3::OpenGameResourceContract::canonical_target_sha256 ==
        dmc3::ArchiveEntryReadContract::canonical_target_sha256);
    assert(dmc3::OpenGameResourceContract::image_base ==
        dmc3::ArchiveEntryReadContract::image_base);
}

} // namespace

int main() {
    request_path_matches_the_recovered_shape();
    overflow_aborts_the_whole_request();
    bootstrap_matches_the_recovered_shape();
    loose_container_matches_the_recovered_grammar();
    archive_entry_read_matches_the_recovered_branch();
    contracts_are_bound_to_one_image();
    return 0;
}
