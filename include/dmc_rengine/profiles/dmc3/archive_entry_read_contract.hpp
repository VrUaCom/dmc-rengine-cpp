#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the recovered archive member read.
//
// `ZipEntryRead` is where the original runtime decides how a member's bytes
// reach the caller: straight off the backing stream, or through the inflater.
// The decision is not made from the member's compression method at this point —
// it is made from whether an inflater context has been attached — and that
// distinction is the whole reason this boundary is worth stating.
//
// Every window below was acquired through the canonical `extract-exe-window`
// path against the analysis image named by `canonical_target_sha256`, and each
// carries the SHA-256 of its exact bytes so the recovery can be re-checked
// against the same image rather than believed.
struct ArchiveEntryReadContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t canonical_target_size = 6'356'432ULL;
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // ZipEntryRead(entry, destination, size, flags) -> signed count.
    static constexpr std::uint64_t entry_read_va = 0x140328F50ULL;
    static constexpr std::uint32_t entry_read_bytes = 0x90U;
    static constexpr std::string_view entry_read_window_sha256 =
        "382c09eaf41035c4b686cd0611e96c55c2f5dc1f40d9c5459502db70c2a0dc40";

    // The positioning/preparation step every read begins with. A false return
    // aborts the read before either branch is taken.
    static constexpr std::uint64_t prepare_va = 0x140328540ULL;
    static constexpr std::uint32_t prepare_bytes = 0x140U;

    // The inflated branch, and the shared backend read primitive both branches
    // ultimately call.
    static constexpr std::uint64_t inflate_read_va = 0x140328820ULL;
    static constexpr std::uint32_t inflate_read_bytes = 0x220U;
    static constexpr std::uint64_t backend_read_va = 0x140327910ULL;

    // Entry object offsets. The branch key is the inflater context: non-null
    // selects the inflated path, null selects the direct path. Nothing at this
    // point re-reads the member's compression method.
    static constexpr std::size_t entry_inflater_context_offset = 0x40U;
    static constexpr std::size_t entry_direct_stream_offset = 0x38U;

    // Backing stream cursor, shared by both branches. The inflater refills its
    // input window from the same object with the same discipline, which is why
    // one layout serves both.
    static constexpr std::size_t stream_handle_offset = 0x00U;
    static constexpr std::size_t stream_consumed_offset = 0x0CU;
    static constexpr std::size_t stream_total_offset = 0x10U;

    // The inflater's input window lives inside its context and refills a page
    // at a time.
    static constexpr std::size_t inflater_input_buffer_offset = 0x10U;
    static constexpr std::uint32_t inflater_refill_bytes = 0x1000U;

    // A failed preparation returns -1. An exhausted stream returns 0, which is
    // an answer rather than an error: nothing remained to read.
    static constexpr std::int64_t prepare_failed_result = -1;
    static constexpr std::int64_t exhausted_result = 0;

    // The consumed cursor advances only on a non-negative backend result, and
    // a negative result is returned unchanged rather than translated.
    static constexpr bool advances_cursor_on_negative_read = false;
    static constexpr bool translates_backend_error = false;

    [[nodiscard]] static consteval std::uint32_t rva_of(
        std::uint64_t virtual_address) noexcept {
        return static_cast<std::uint32_t>(virtual_address - image_base);
    }

    [[nodiscard]] static constexpr bool takes_inflated_branch(
        bool inflater_context_present) noexcept {
        return inflater_context_present;
    }
};

// What the recovered direct branch does with one read request.
//
// A pure model, in the same spirit as PhysicalProviderModel: it lets product
// behavior be compared against the recovered arithmetic without dragging the
// original object layout into portable code.
struct ArchiveDirectReadPlan final {
    std::uint32_t remaining{};
    std::uint32_t clamped_size{};
    std::uint32_t next_consumed{};
    std::int64_t result{};
    bool reaches_backend{false};
};

class ArchiveDirectReadModel final {
public:
    // `backend_result` is what the backend read returned, or nullopt when the
    // recovered path never reaches it.
    [[nodiscard]] static constexpr ArchiveDirectReadPlan plan(
        std::uint32_t total,
        std::uint32_t consumed,
        std::uint32_t requested,
        std::optional<std::int64_t> backend_result = std::nullopt) noexcept {
        ArchiveDirectReadPlan result;
        result.remaining = total > consumed ? total - consumed : 0U;
        result.clamped_size =
            requested > result.remaining ? result.remaining : requested;
        result.next_consumed = consumed;

        if (result.clamped_size == 0U) {
            // Nothing remained. The recovered path returns zero without
            // touching the backend at all.
            result.result = ArchiveEntryReadContract::exhausted_result;
            return result;
        }

        result.reaches_backend = true;
        result.result = backend_result.value_or(0);
        if (result.result >= 0) {
            result.next_consumed =
                consumed + static_cast<std::uint32_t>(result.result);
        }
        return result;
    }
};

} // namespace dmc::rengine::profiles::dmc3
