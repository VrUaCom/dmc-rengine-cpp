#pragma once

#include "dmc_rengine/binary/document.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::binary {

enum class DiffKind {
    equal,
    modified,
    inserted,
    removed,
};

[[nodiscard]] constexpr std::string_view to_string(DiffKind kind) noexcept {
    switch (kind) {
    case DiffKind::equal: return "equal";
    case DiffKind::modified: return "modified";
    case DiffKind::inserted: return "inserted";
    case DiffKind::removed: return "removed";
    }
    return "modified";
}

struct DiffSpan final {
    DiffKind kind{DiffKind::equal};
    std::optional<ByteRange> left_range;
    std::optional<ByteRange> right_range;
};

struct ByteDiffResult final {
    std::uint64_t left_size{};
    std::uint64_t right_size{};
    std::uint64_t equal_bytes{};
    std::uint64_t modified_bytes{};
    std::uint64_t inserted_bytes{};
    std::uint64_t removed_bytes{};
    std::vector<DiffSpan> spans;

    [[nodiscard]] bool identical() const noexcept;
};

// Produces a deterministic offset-aligned byte diff. Middle insertions are not
// resynchronized in this first cross-port wave; they appear as modified bytes
// followed by an inserted or removed tail.
[[nodiscard]] ByteDiffResult aligned_byte_diff(
    std::span<const std::byte> left,
    std::span<const std::byte> right);

enum class EntropyBand {
    zero_fill,
    low,
    medium,
    high,
};

[[nodiscard]] constexpr std::string_view to_string(EntropyBand band) noexcept {
    switch (band) {
    case EntropyBand::zero_fill: return "zero-fill";
    case EntropyBand::low: return "low";
    case EntropyBand::medium: return "medium";
    case EntropyBand::high: return "high";
    }
    return "medium";
}

struct EntropyOptions final {
    std::size_t window_size{256U};
    std::size_t step_size{256U};
    bool include_partial_window{true};

    [[nodiscard]] bool valid() const noexcept;
};

struct EntropyWindow final {
    ByteRange range;
    double bits_per_byte{};
    double zero_ratio{};
    std::uint16_t unique_byte_count{};
    EntropyBand band{EntropyBand::low};
};

// Computes Shannon entropy for fixed or overlapping windows. Entropy bands are
// visualization heuristics only and must not be treated as format evidence.
[[nodiscard]] std::vector<EntropyWindow> entropy_map(
    std::span<const std::byte> bytes,
    EntropyOptions options = {});

} // namespace dmc::rengine::binary
