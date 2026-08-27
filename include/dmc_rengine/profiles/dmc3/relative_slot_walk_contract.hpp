#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for how the original runtime walks a
// relative-slot container.
//
// The product had this shape already, derived from parsing real files. That is
// weaker than it looks: a parser that agrees with every file in a corpus can
// still be wrong about the file the corpus does not contain. These two
// routines settle it — the header layout, the sparse-slot rule and the
// relative addressing are all read directly out of the runtime's own walk.
struct RelativeSlotWalkContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // Two walks, recovered independently, agreeing on the layout.
    //
    // The PAC walk sits inside the loaded-resource pool finalizer; the PNST
    // walk sits inside the type dispatcher and recurses into itself, which is
    // what makes a container inside a container work at all.
    static constexpr std::uint64_t pac_walk_va = 0x1401B92E0ULL;
    static constexpr std::uint64_t pnst_walk_va = 0x1401B9FA0ULL;

    // The magic comparisons, as the runtime actually performs them.
    //
    // PAC is three bytes. The stored fourth byte is NUL and the runtime never
    // looks at it, so a reader that demands `PAC\0` is stricter than the game.
    // PNST is four, including the `T`.
    static constexpr std::string_view pac_magic = "PAC";
    static constexpr std::string_view pnst_magic = "PNST";
    static constexpr std::size_t pac_magic_bytes = 3U;
    static constexpr std::size_t pnst_magic_bytes = 4U;

    // Header layout, shared by both containers.
    static constexpr std::size_t slot_count_offset = 0x04U;
    static constexpr std::size_t offset_table_offset = 0x08U;
    static constexpr std::size_t offset_entry_bytes = 0x04U;

    // The walk indexes the container as an array of dwords starting at index 2,
    // which is the same thing as a table at byte offset 8 — recorded both ways
    // because the recovered loop is written the first way.
    static constexpr std::uint32_t first_offset_dword_index = 2U;

    // A zero offset is a slot that carries nothing. The runtime substitutes a
    // null pointer and calls no handler for it. This is the instruction-level
    // proof that a sparse container is intact rather than damaged, and that a
    // slot index is an identity rather than a position in a packed list.
    static constexpr std::uint32_t absent_slot_offset = 0U;
    static constexpr bool absent_slot_is_skipped = true;

    // Offsets are relative to the container's own base, not to the file or to
    // the parent. The runtime adds the container pointer to the stored value.
    static constexpr bool offsets_are_container_relative = true;

    // A non-zero count is required before the table is read at all; a
    // zero-or-negative count ends the walk with no children.
    static constexpr bool signed_slot_count = true;

    // PNST children are walked recursively; the PAC walk dispatches each child
    // by type instead, which is how a PNST nested in a PAC still expands.
    static constexpr bool pnst_walk_is_recursive = true;

    // Tags the dispatcher recognizes and deliberately does not walk. Recorded
    // because "recognized but not expanded" is a real state, distinct from
    // unknown, and a product that expands one would be inventing structure.
    //
    // `EFE` was missed the first time this was read: the compiler collapsed
    // the second comparison into `cmp cl, cl` followed by `cmp [rbx+2], al`,
    // which tests the third byte against `E` — the value `al` still holds from
    // the first byte's own comparison. It reads like dead code and is not.
    static constexpr std::string_view recognized_not_walked = "EFW";
    static constexpr std::uint64_t recognized_not_walked_va = 0x1401BA00DULL;
    static constexpr std::string_view second_recognized_not_walked = "EFE";
    static constexpr std::uint64_t second_recognized_not_walked_va =
        0x1401BA024ULL;

    // The four payload handlers the dispatcher runs before it ever looks for a
    // container, each matched on three bytes.
    static constexpr std::array<std::string_view, 4> dispatched_payload_tags{
        "MOD", "EFM", "SCM", "SHW"};
    static constexpr std::array<std::uint64_t, 4> dispatched_payload_handlers{
        0x1402FE3B0ULL, 0x1402F7A90ULL, 0x1403051B0ULL, 0x1403204C0ULL};

    // The dispatcher walks `PNST` and nothing else.
    //
    // A nested `PAC` is not walked by it: the `P` branch requires `PNST`, so
    // `PAC\0` falls out at the second byte. Only the pool finalizer walks a
    // PAC, and only the one held in a pool slot. `st001.pac` slot 7 is a PAC
    // holding a motion, and the runtime's own dispatcher would not descend
    // into it — this project's expander does, which is a tool going further
    // than the game rather than a shape the game reads.
    static constexpr bool dispatcher_walks_nested_pac = false;

    // The negative that makes unpacking ambiguous, and the reason it is a
    // product decision rather than a recovered one.
    //
    // Neither walk computes or reads a child's size. Both compute
    // `container + stored_offset` and pass that bare pointer to the handler,
    // which finds its own end from its own contents. So a container does not
    // record how long a slot is, and "up to the next non-zero offset" is this
    // project's reading, not the game's.
    static constexpr bool walk_computes_child_size = false;

    // The bound is re-read from the container on every iteration rather than
    // held in a register, so a handler that rewrites the count mid-walk moves
    // the end of the walk. Recorded because a reader that hoists it is not
    // reproducing this loop.
    static constexpr bool slot_count_reread_each_iteration = true;

    // The pool finalizer acts on a slot only in this state, and leaves it in
    // the next one. A slot in any other state is stepped over untouched.
    static constexpr std::int32_t pool_slot_state_walked = 2;
    static constexpr std::int32_t pool_slot_state_after_walk = 3;
    static constexpr std::size_t pool_slot_state_offset = 0x04U;
    static constexpr std::size_t pool_slot_finalizer_offset = 0x10U;
    static constexpr std::size_t pool_slot_payload_offset = 0x20U;

    // A pool slot's payload does not have to be a container. When it is not a
    // PAC the finalizer dispatches it directly, so a slot can hold a bare
    // `SCM`, `MOD` or `PNST`.
    static constexpr bool pool_slot_payload_may_be_a_bare_record = true;

    // The loaded-resource pool the PAC walk finalizes over: a fixed array, not
    // a dynamic list.
    static constexpr std::size_t pool_slot_count = 0x16BU;
    static constexpr std::size_t pool_slot_stride = 0x48U;
    static constexpr std::size_t pool_first_slot_offset = 0x10U;
    static constexpr std::size_t pool_state_offset = 0x04U;
    static constexpr std::size_t pool_payload_offset = 0x20U;
    static constexpr std::int32_t pool_state_loaded = 2;
    static constexpr std::int32_t pool_state_finalized = 3;

    [[nodiscard]] static consteval std::size_t header_bytes(
        std::uint32_t slot_count) noexcept {
        return offset_table_offset +
            static_cast<std::size_t>(slot_count) * offset_entry_bytes;
    }
};

} // namespace dmc::rengine::profiles::dmc3
