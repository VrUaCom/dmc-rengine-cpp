#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for L1's loaded-resource pool.
//
// Thirty byte windows of this layer were acquired in an earlier pass and none
// of them became code — the addresses sat in documents while the product
// modelled resource lifetime its own way. This is that layer read out.
//
// The finding that matters: the pool is **not** a dynamic list. It is a fixed
// array of 363 records, statically partitioned into seven groups whose bases
// and counts live in the image as data. A resource does not go "somewhere in
// the pool"; it goes into a specific group with a fixed capacity, and when
// that group is full the original runtime has nowhere else to put it.
struct LoadedResourcePoolContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t registry_init_va = 0x1401B8380ULL;
    static constexpr std::uint64_t acquire_va = 0x1401B84E0ULL;
    static constexpr std::uint64_t materialization_dispatch_va = 0x1401B8CA0ULL;
    static constexpr std::uint64_t completion_state1_to_2_va = 0x1401B8DC0ULL;
    static constexpr std::uint64_t state2_finalizer_va = 0x1401B92D0ULL;
    static constexpr std::uint64_t state4_cleanup_va = 0x1401B8F00ULL;
    static constexpr std::uint64_t normal_release_va = 0x1401B9530ULL;
    static constexpr std::uint64_t group_reset_va = 0x1401B9560ULL;
    static constexpr std::uint64_t full_reset_va = 0x1401B95E0ULL;
    static constexpr std::uint64_t loose_materializer_va = 0x1401B85C0ULL;

    // The pool is a single global object, not an instance passed around.
    // Three routines address it directly and agree: the acquire path computes
    // a record's handle by subtracting this, the completion helper adds it
    // back, and the deferred sweep walks from it.
    static constexpr std::uint64_t pool_global_va = 0x140C99D30ULL;

    // A record handle is that record's *byte offset* from the pool base, not
    // an index. The completion helper traps deliberately on an odd handle —
    // it writes through a null pointer rather than continuing — because the
    // record stride is even and an odd offset can never name a record.
    static constexpr bool handle_is_byte_offset = true;
    static constexpr bool odd_handle_traps = true;

    // Record geometry, from the initializer and the finalizer agreeing.
    static constexpr std::size_t record_stride = 0x48U;
    static constexpr std::size_t record_group_offset = 0x00U;
    static constexpr std::size_t record_state_offset = 0x04U;
    static constexpr std::size_t record_payload_offset = 0x20U;
    static constexpr std::size_t record_embedded_offset = 0x28U;
    static constexpr std::size_t record_count = 0x16BU;

    // The state machine, as the recovered routines move through it.
    enum class State : std::int32_t {
        free = 0,
        requested = 1,
        loaded = 2,
        relocated = 3,
        releasing = 4,
    };

    // The finalizer walks records in `loaded`, relocates the payload and
    // leaves them `relocated`. A group wrapper skips a record already there.
    static constexpr State finalize_from = State::loaded;
    static constexpr State finalize_to = State::relocated;

    // Every transition has a routine, and the set closes: there is no state
    // the pool can reach and not leave.
    struct Transition final {
        State from;
        State to;
        std::uint64_t routine_va;
        std::string_view what;
    };
    static constexpr std::array<Transition, 6> transitions{
        Transition{State::free, State::requested, acquire_va,
                   "allocate the payload and materialize it"},
        Transition{State::requested, State::loaded, completion_state1_to_2_va,
                   "the load completed"},
        Transition{State::loaded, State::relocated, state2_finalizer_va,
                   "relocate offsets to pointers, dispatching each payload by tag"},
        Transition{State::relocated, State::free, normal_release_va,
                   "destroy the embedded object and free the record"},
        Transition{State::releasing, State::free, state4_cleanup_va,
                   "the deferred sweep, over every record marked for release"},
        Transition{State::relocated, State::free, full_reset_va,
                   "reset every record in the pool unconditionally"},
    };

    // The request descriptor a caller hands to acquire, kept at the record.
    static constexpr std::size_t record_request_offset = 0x18U;
    static constexpr std::size_t record_user_offset = 0x10U;
    static constexpr std::size_t request_kind_offset = 0x00U;
    static constexpr std::size_t request_pointer_offset = 0x08U;
    static constexpr std::size_t request_kind_bytes = 2U;

    // Materialization is where L1 meets the loose-container layer. The
    // dispatch reads the request's `u16` kind; a non-zero kind goes straight
    // to the generic materializer, and only kind zero — container-backed —
    // consults the representation selector, whose three outcomes are:
    //
    //   0        refuse
    //   1        the packed container wins, materialize it generically
    //   anything the `.lst` list representation
    //
    // Those are `LooseContainerContract`'s own addresses, reached from here.
    // Until now that contract had no recovered caller; this is it.
    static constexpr std::int32_t representation_refused = 0;
    static constexpr std::int32_t representation_packed = 1;

    // The alternate allocator. When the pool flag is 1 and the descriptor
    // pointer is non-null, acquire allocates through the pool's own arena
    // instead of the shared loader.
    static constexpr std::size_t pool_arena_descriptor_offset = 0x6718U;
    static constexpr std::size_t pool_arena_pointer_offset = 0x6720U;
    static constexpr std::uint8_t pool_flag_arena_enabled = 1U;

    // The static partition. Both tables are image data, not computed.
    static constexpr std::uint64_t partition_count_table_va = 0x140581A10ULL;
    static constexpr std::uint64_t partition_base_table_va = 0x140581A20ULL;
    static constexpr std::size_t group_count = 7U;

    static constexpr std::array<std::uint16_t, group_count> group_capacities{
        4U, 136U, 60U, 28U, 1U, 128U, 6U};
    static constexpr std::array<std::uint16_t, group_count> group_bases{
        0U, 4U, 140U, 200U, 228U, 229U, 357U};

    // One wrapper per group, each reading its own base out of the table above.
    static constexpr std::array<std::uint64_t, group_count> group_wrapper_vas{
        0x1401B8F50ULL,  // group 0, base 0, capacity 4
        0x1401B90B0ULL,  // group 1, base 4, capacity 136
        0x1401B9160ULL,  // group 2, base 140, capacity 60
        0x1401B8FF0ULL,  // group 3, base 200, capacity 28
        0x1401B8D60ULL,  // group 4, base 228, capacity 1
        0x1401B8DF0ULL,  // group 5, base 229, capacity 128
        0x1401B9270ULL,  // group 6, base 357, capacity 6
    };

    // How a group hands out a record. Six groups take the index the caller
    // names; group 5 searches. That is the answer to "what picks a group": the
    // caller does, by calling that group's own wrapper, and only one group is
    // a pool in the usual sense.
    //
    // It is not the largest one — group 1 holds 136 records to its 128 and
    // still takes a named index. "Dynamic" describes how a record is chosen,
    // never how many there are.
    enum class Allocation : std::uint8_t {
        caller_named_index,
        first_free_scan,
    };
    static constexpr std::array<Allocation, group_count> group_allocation{
        Allocation::caller_named_index,
        Allocation::caller_named_index,
        Allocation::caller_named_index,
        Allocation::caller_named_index,
        Allocation::caller_named_index,
        Allocation::first_free_scan,
        Allocation::caller_named_index,
    };
    static constexpr std::size_t dynamic_group = 5U;

    // What the original runtime does when the dynamic group is full: it writes
    // through a null pointer. There is no failure path — the scan falls out of
    // its loop with a null record and stores into it immediately.
    //
    // This is recorded because it bounds what may honestly be said about the
    // game's behavior. A tool must not claim loading degrades gracefully at
    // capacity, and must not reproduce this either.
    static constexpr bool exhaustion_is_handled = false;

    // A pool-level flag the initializer clears last, past the record array.
    static constexpr std::size_t pool_flag_offset = 0x6760U;

    [[nodiscard]] static consteval std::size_t record_array_bytes() noexcept {
        return record_count * record_stride;
    }

    // The partition must tile the pool exactly: every record belongs to one
    // group, and no group reaches past the end. This is the invariant that
    // makes the two tables trustworthy, and it is checked rather than assumed.
    [[nodiscard]] static consteval bool partition_tiles_the_pool() noexcept {
        std::size_t cursor = 0U;
        for (std::size_t index = 0U; index < group_count; ++index) {
            if (group_bases[index] != cursor) {
                return false;
            }
            cursor += group_capacities[index];
        }
        return cursor == record_count;
    }

    [[nodiscard]] static constexpr std::size_t record_offset(
        std::size_t slot) noexcept {
        return slot * record_stride;
    }

    [[nodiscard]] static constexpr std::size_t group_of(
        std::size_t slot) noexcept {
        for (std::size_t index = group_count; index-- > 0U;) {
            if (slot >= group_bases[index]) {
                return index;
            }
        }
        return 0U;
    }
};

static_assert(
    LoadedResourcePoolContract::partition_tiles_the_pool(),
    "the recovered group partition must tile the recovered pool exactly");

} // namespace dmc::rengine::profiles::dmc3
