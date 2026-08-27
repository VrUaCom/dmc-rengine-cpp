#include "dmc_rengine/profiles/dmc3/archive_entry_read_contract.hpp"
#include "dmc_rengine/profiles/dmc3/loaded_resource_pool_contract.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_contract.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"
#include "dmc_rengine/profiles/dmc3/open_game_resource_contract.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_bootstrap_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"

#include <cassert>
#include <utility>
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

void the_product_identifies_types_the_way_the_runtime_does() {
    using Contract = dmc3::ResourceTypeContract;
    namespace gdspaces = dmc::rengine::gdspaces;

    // Every tag the recovered probe compares must reach the classifier, and it
    // must reach it as three bytes. `SCM ` with its trailing space is what the
    // stage files store, but the runtime stops after `SCM`, so a payload that
    // carries `SCMx` is a scene model to the game and must be one here.
    static_assert(Contract::content_tag_bytes == 3U);
    const std::pair<std::string_view, std::string_view> expected[]{
        {"MOD", "mod"}, {"EFM", "efm"}, {"SCM", "scm"},
        {"MRP", "mrp"}, {"SHW", "shw"},
    };
    for (const auto& [tag, format] : expected) {
        assert(Contract::type_for_tag(tag) != Contract::TypeCode::unknown);
        std::vector<std::byte> payload(64U, std::byte{'x'});
        for (std::size_t index = 0U; index < tag.size(); ++index) {
            payload[index] = static_cast<std::byte>(tag[index]);
        }
        const auto classified = gdspaces::ResourceClassifier::classify(
            "slot_0000.bin", std::span<const std::byte>{payload}, false);
        assert(classified.format == format);
        assert(classified.magic_confirmed);
    }

    // A tag the probe does not know stays unknown rather than being pushed
    // into the nearest recognized type.
    assert(Contract::type_for_tag("XYZ") == Contract::TypeCode::unknown);

    // The extension table is enumerated by case, not folded. `.PTx` is a name
    // the runtime does not recognize, and the contract must not quietly claim
    // otherwise.
    bool has_ptx_lower = false;
    bool has_ptx_mixed = false;
    for (const auto& entry : Contract::extension_types) {
        has_ptx_lower = has_ptx_lower || entry.extension == ".ptx";
        has_ptx_mixed = has_ptx_mixed || entry.extension == ".PTx";
    }
    assert(has_ptx_lower);
    assert(!has_ptx_mixed);
    static_assert(Contract::extension_outranks_content_tag);
    static_assert(Contract::table_bytes() == 0x6508U);
}

void the_product_walks_slots_the_way_the_runtime_does() {
    using Walk = dmc3::RelativeSlotWalkContract;
    namespace formats = dmc::rengine::formats;

    static_assert(Walk::pac_magic_bytes == 3U);
    static_assert(Walk::pnst_magic_bytes == 4U);
    static_assert(Walk::slot_count_offset == 0x04U);
    static_assert(Walk::offset_table_offset == 0x08U);
    // The recovered loop starts at dword index 2, which is the byte offset 8
    // the parser uses. Stating both and checking they agree is the point.
    static_assert(
        Walk::first_offset_dword_index * Walk::offset_entry_bytes ==
        Walk::offset_table_offset);
    static_assert(Walk::header_bytes(3U) == 0x14U);

    // Three slots: one absent, two present. The runtime skips the zero and
    // keeps the index, and so must the parser.
    std::vector<std::byte> container(0x80U, std::byte{0});
    container[0] = static_cast<std::byte>('P');
    container[1] = static_cast<std::byte>('A');
    container[2] = static_cast<std::byte>('C');
    // The fourth byte is deliberately not NUL: the recovered comparison never
    // reads it, so a container that carries something else there still parses.
    container[3] = static_cast<std::byte>('?');
    container[4] = std::byte{3};
    container[8U + 4U] = std::byte{0x40};   // slot 1
    container[8U + 8U] = std::byte{0x60};   // slot 2

    const auto parsed = formats::PacParser::parse(container);
    assert(parsed.ok());
    assert(parsed.document->declared_slot_count == 3U);
    assert(!parsed.document->entries[0].populated);
    assert(parsed.document->entries[1].populated);
    assert(parsed.document->entries[1].offset == 0x40U);
    assert(parsed.document->entries[2].slot_index == 2U);

    // PNST compares all four bytes, so the same liberty is not taken there.
    std::vector<std::byte> pnst(0x80U, std::byte{0});
    pnst[0] = static_cast<std::byte>('P');
    pnst[1] = static_cast<std::byte>('N');
    pnst[2] = static_cast<std::byte>('S');
    pnst[3] = static_cast<std::byte>('?');
    pnst[4] = std::byte{1};
    pnst[8] = std::byte{0x40};
    assert(!formats::PnstParser::parse(pnst).ok());
}

void the_animation_registry_is_a_second_registry() {
    using Animation = dmc3::AnimationTypeContract;
    using Types = dmc3::ResourceTypeContract;

    // Two registries, not one table read twice. Different address, different
    // capacity, different type numbering — and only one of them falls back to
    // the payload's own bytes.
    static_assert(
        Animation::register_and_classify_va != Types::register_and_classify_va);
    static_assert(Animation::table_capacity == 0x400U);
    static_assert(Types::table_capacity == 0x100U);
    static_assert(!Animation::has_content_tag_fallback);
    static_assert(Animation::table_bytes() == 0x19408U);

    assert(Animation::type_for_extension(".mot") ==
        Animation::TypeCode::motion);
    assert(Animation::type_for_extension(".MOT") ==
        Animation::TypeCode::motion);
    assert(Animation::type_for_extension(".tsc") == Animation::TypeCode::tsc);
    // Case is enumerated in pairs, so a spelling neither table lists is not
    // recognized by either.
    assert(Animation::type_for_extension(".Mot") ==
        Animation::TypeCode::unregistered);
    assert(Animation::type_for_extension(".ptx") ==
        Animation::TypeCode::unregistered);

    // `.clt` is the one extension both registries claim, and they number it
    // differently. A type code means nothing without the registry that issued
    // it, and this is the case that proves it.
    assert(Animation::type_for_extension(Animation::shared_extension) ==
        Animation::TypeCode::palette);
    static_assert(
        static_cast<int>(Animation::TypeCode::palette) !=
        static_cast<int>(Types::TypeCode::palette));
    bool shared_in_first = false;
    for (const auto& entry : Types::extension_types) {
        shared_in_first =
            shared_in_first || entry.extension == Animation::shared_extension;
    }
    assert(shared_in_first);

    // Neither registry reads its own type array back. Dispatch happens at
    // registration and the second dispatcher re-probes the tag, so the stored
    // code is recorded state rather than the key that selects a reader. A
    // product that modelled it as a dispatch key would be describing a
    // mechanism the runtime does not have.
    static_assert(!Animation::stored_type_is_read_back);
    static_assert(!Types::stored_type_is_read_back);
    // And this registry's payloads carry a tag the runtime never compares.
    static_assert(!Animation::payload_tag_is_compared);

    static_assert(
        Animation::canonical_target_sha256 == Types::canonical_target_sha256);
    static_assert(Animation::image_base == Types::image_base);
}

void the_loaded_resource_pool_is_a_fixed_partition() {
    using Pool = dmc3::LoadedResourcePoolContract;
    using Walk = dmc3::RelativeSlotWalkContract;

    // The pool is an array, not a list, and the two routines that walk it
    // agree about its shape. If these ever drift, one contract is describing a
    // different structure than the other claims.
    static_assert(Pool::record_count == Walk::pool_slot_count);
    static_assert(Pool::record_stride == Walk::pool_slot_stride);
    static_assert(Pool::record_state_offset == Walk::pool_state_offset);
    static_assert(Pool::record_payload_offset == Walk::pool_payload_offset);
    static_assert(
        static_cast<std::int32_t>(Pool::finalize_from) == Walk::pool_state_loaded);
    static_assert(
        static_cast<std::int32_t>(Pool::finalize_to) == Walk::pool_state_finalized);

    // Seven groups tile 363 records exactly. The consteval check above already
    // refuses to compile otherwise; this states the arithmetic in the open so
    // a future edit to either table has to face it.
    static_assert(Pool::partition_tiles_the_pool());
    static_assert(Pool::record_array_bytes() == 0x6618U);
    std::size_t total = 0U;
    for (const auto capacity : Pool::group_capacities) {
        total += capacity;
    }
    assert(total == Pool::record_count);

    // Every group's own wrapper must land inside that group.
    for (std::size_t index = 0U; index < Pool::group_count; ++index) {
        const auto base = Pool::group_bases[index];
        assert(Pool::group_of(base) == index);
        if (Pool::group_capacities[index] > 1U) {
            const std::size_t last = base + Pool::group_capacities[index] - 1U;
            assert(Pool::group_of(last) == index);
        }
    }
    // The first record of group 4 is the only record of group 4: a capacity of
    // one is a real value in this table, not a placeholder.
    assert(Pool::group_capacities[4] == 1U);
    assert(Pool::group_of(Pool::group_bases[4]) == 4U);
    assert(Pool::group_of(Pool::group_bases[4] + 1U) == 5U);

    // Every group has a wrapper now, and exactly one of them searches. Six
    // take the index the caller names; group 5 scans for a free record, which
    // is why it is the only one that is a pool in the usual sense — and why it
    // is the largest.
    std::size_t scanning = 0U;
    for (std::size_t index = 0U; index < Pool::group_count; ++index) {
        assert(Pool::group_wrapper_vas[index] > Pool::image_base);
        scanning += Pool::group_allocation[index] == Pool::Allocation::first_free_scan
            ? 1U : 0U;
    }
    assert(scanning == 1U);
    assert(Pool::group_allocation[Pool::dynamic_group] ==
        Pool::Allocation::first_free_scan);
    // The searching group is *not* the largest — group 1 holds 136 records
    // against its 128, and takes the index the caller names. So "dynamic"
    // here means how a record is chosen, never how many there are, and the
    // obvious reading that the biggest group must be the pool is wrong.
    assert(Pool::group_capacities[1] > Pool::group_capacities[Pool::dynamic_group]);
    assert(Pool::group_allocation[1] == Pool::Allocation::caller_named_index);

    // The runtime has no failure path at capacity: the scan falls out of its
    // loop with a null record and stores into it. Recorded so nothing in this
    // product claims the game degrades gracefully there.
    static_assert(!Pool::exhaustion_is_handled);

    // The record array ends before the pool flag the initializer clears, so
    // the flag is a pool field rather than a record that was miscounted.
    static_assert(Pool::record_array_bytes() < Pool::pool_flag_offset);

    static_assert(
        Pool::canonical_target_sha256 ==
        dmc3::ResourceTypeContract::canonical_target_sha256);
}

void the_l1_lifecycle_closes() {
    using Pool = dmc3::LoadedResourcePoolContract;
    using Loose = dmc3::LooseContainerContract;

    // Every state the pool can reach has a routine that leaves it. A state
    // with an entry and no exit would be a leak the recovered code does not
    // have, and asserting the closure is what keeps a future edit from
    // inventing one.
    const Pool::State states[]{
        Pool::State::free, Pool::State::requested, Pool::State::loaded,
        Pool::State::relocated, Pool::State::releasing,
    };
    for (const auto state : states) {
        bool leaves = false;
        for (const auto& transition : Pool::transitions) {
            leaves = leaves || transition.from == state;
        }
        assert(leaves);
    }
    // Free is reachable from three different places: an ordinary release, the
    // deferred sweep and the full reset. That is not redundancy — they are
    // three different lifetimes ending the same way.
    std::size_t to_free = 0U;
    for (const auto& transition : Pool::transitions) {
        to_free += transition.to == Pool::State::free ? 1U : 0U;
        assert(transition.routine_va > Pool::image_base);
        assert(!transition.what.empty());
    }
    assert(to_free == 3U);

    // Cancellation is deferred: it moves an in-flight record to `releasing`
    // and leaves the destroying to the sweep. Both unfinished states can be
    // cancelled and `relocated` cannot — a load that got that far is finished,
    // not in flight.
    static_assert(Pool::cancellation_is_deferred);
    bool cancels_requested = false;
    bool cancels_loaded = false;
    bool cancels_relocated = false;
    for (const auto& transition : Pool::transitions) {
        if (transition.routine_va != Pool::cancel_inflight_va) {
            continue;
        }
        assert(transition.to == Pool::State::releasing);
        cancels_requested = cancels_requested || transition.from == Pool::State::requested;
        cancels_loaded = cancels_loaded || transition.from == Pool::State::loaded;
        cancels_relocated = cancels_relocated || transition.from == Pool::State::relocated;
    }
    assert(cancels_requested && cancels_loaded && !cancels_relocated);

    // The pool is one global object. A record handle is its byte offset from
    // that base, so an odd handle cannot name a record — and the recovered
    // completion helper traps on one rather than continuing.
    static_assert(Pool::pool_global_va > Pool::image_base);
    static_assert(Pool::handle_is_byte_offset);
    static_assert(Pool::odd_handle_traps);
    static_assert(Pool::record_stride % 2U == 0U);

    // The junction. Materialization consults the loose-container selector
    // only for a container-backed request, and that contract's own constant
    // says which kind that is. Two contracts recovered a week apart have to
    // agree here or one of them is describing a different branch.
    static_assert(Loose::container_backed_kind16 == 0U);
    static_assert(Pool::request_kind_bytes == sizeof(std::uint16_t));
    static_assert(Pool::representation_packed == 1);
    static_assert(Pool::representation_refused == 0);
    // The packed outcome is the one the loose contract calls the winner.
    static_assert(Loose::packed_representation_wins);
    // And the two materializers the dispatch jumps to are that contract's.
    static_assert(Pool::loose_materializer_va == Loose::loose_materializer_va);
    static_assert(
        Pool::canonical_target_sha256 == Loose::canonical_target_sha256);
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
    static_assert(
        dmc3::OpenGameResourceContract::canonical_target_sha256 ==
        dmc3::ResourceTypeContract::canonical_target_sha256);
    static_assert(
        dmc3::OpenGameResourceContract::canonical_target_sha256 ==
        dmc3::RelativeSlotWalkContract::canonical_target_sha256);
    static_assert(
        dmc3::ResourceTypeContract::image_base ==
        dmc3::RelativeSlotWalkContract::image_base);
    // Both dispatchers are the same routine seen from two sides; the contracts
    // must agree on its address or one of them is describing a different
    // function than it claims.
    static_assert(
        dmc3::ResourceTypeContract::type_dispatch_va ==
        dmc3::RelativeSlotWalkContract::pnst_walk_va);
}

} // namespace

int main() {
    request_path_matches_the_recovered_shape();
    overflow_aborts_the_whole_request();
    bootstrap_matches_the_recovered_shape();
    loose_container_matches_the_recovered_grammar();
    archive_entry_read_matches_the_recovered_branch();
    the_product_identifies_types_the_way_the_runtime_does();
    the_product_walks_slots_the_way_the_runtime_does();
    the_animation_registry_is_a_second_registry();
    the_loaded_resource_pool_is_a_fixed_partition();
    the_l1_lifecycle_closes();
    contracts_are_bound_to_one_image();
    return 0;
}
