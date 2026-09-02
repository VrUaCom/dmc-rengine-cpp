#include "dmc_rengine/profiles/dmc3/archive_entry_read_contract.hpp"

namespace dmc::rengine::profiles::dmc3 {

static_assert(ArchiveEntryReadContract::image_base == 0x140000000ULL);
static_assert(ArchiveEntryReadContract::canonical_target_sha256.size() == 64U);
static_assert(ArchiveEntryReadContract::entry_read_window_sha256.size() == 64U);
static_assert(ArchiveEntryReadContract::canonical_target_size == 6'356'432ULL);

static_assert(ArchiveEntryReadContract::rva_of(
    ArchiveEntryReadContract::entry_read_va) == 0x00328F50U);
static_assert(ArchiveEntryReadContract::rva_of(
    ArchiveEntryReadContract::prepare_va) == 0x00328540U);
static_assert(ArchiveEntryReadContract::rva_of(
    ArchiveEntryReadContract::inflate_read_va) == 0x00328820U);
static_assert(ArchiveEntryReadContract::rva_of(
    ArchiveEntryReadContract::backend_read_va) == 0x00327910U);

// ZipEntryRead ends exactly where the compressed seek/reset begins, so the
// recovered body is bounded rather than a window guessed around an anchor.
static_assert(
    ArchiveEntryReadContract::entry_read_va +
        ArchiveEntryReadContract::entry_read_bytes == 0x140328FE0ULL);

static_assert(ArchiveEntryReadContract::entry_inflater_context_offset == 0x40U);
static_assert(ArchiveEntryReadContract::entry_direct_stream_offset == 0x38U);
static_assert(ArchiveEntryReadContract::stream_consumed_offset == 0x0CU);
static_assert(ArchiveEntryReadContract::stream_total_offset == 0x10U);
static_assert(
    ArchiveEntryReadContract::stream_consumed_offset <
    ArchiveEntryReadContract::stream_total_offset);

static_assert(ArchiveEntryReadContract::inflater_refill_bytes == 0x1000U);
static_assert(ArchiveEntryReadContract::prepare_failed_result == -1);
static_assert(ArchiveEntryReadContract::exhausted_result == 0);
static_assert(!ArchiveEntryReadContract::advances_cursor_on_negative_read);
static_assert(!ArchiveEntryReadContract::translates_backend_error);

static_assert(ArchiveEntryReadContract::takes_inflated_branch(true));
static_assert(!ArchiveEntryReadContract::takes_inflated_branch(false));

// The direct branch, as arithmetic.
static_assert(ArchiveDirectReadModel::plan(100U, 0U, 10U, 10).clamped_size == 10U);
static_assert(ArchiveDirectReadModel::plan(100U, 0U, 10U, 10).next_consumed == 10U);

// A request larger than what remains is clamped, not refused.
static_assert(ArchiveDirectReadModel::plan(100U, 90U, 50U, 10).clamped_size == 10U);

// Nothing remaining returns zero and never reaches the backend.
static_assert(ArchiveDirectReadModel::plan(100U, 100U, 8U).result == 0);
static_assert(!ArchiveDirectReadModel::plan(100U, 100U, 8U).reaches_backend);

// A negative backend result is returned unchanged and leaves the cursor alone.
static_assert(ArchiveDirectReadModel::plan(100U, 20U, 8U, -1).result == -1);
static_assert(ArchiveDirectReadModel::plan(100U, 20U, 8U, -1).next_consumed == 20U);

// A short read advances by what was actually read, not by what was asked for.
static_assert(ArchiveDirectReadModel::plan(100U, 0U, 40U, 12).next_consumed == 12U);

} // namespace dmc::rengine::profiles::dmc3
