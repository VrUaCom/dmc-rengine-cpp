#include "dmc_rengine/binary/analysis.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace dmc::rengine::binary {
namespace {

[[nodiscard]] ByteRange make_range(
    std::size_t offset,
    std::size_t size) noexcept {
    return ByteRange{
        .offset = static_cast<std::uint64_t>(offset),
        .size = static_cast<std::uint64_t>(size),
    };
}

[[nodiscard]] EntropyBand classify_entropy(
    double bits_per_byte,
    double zero_ratio) noexcept {
    if (zero_ratio == 1.0) {
        return EntropyBand::zero_fill;
    }
    if (bits_per_byte < 2.0) {
        return EntropyBand::low;
    }
    if (bits_per_byte < 6.0) {
        return EntropyBand::medium;
    }
    return EntropyBand::high;
}

} // namespace

bool ByteDiffResult::identical() const noexcept {
    return left_size == right_size && modified_bytes == 0U &&
        inserted_bytes == 0U && removed_bytes == 0U;
}

ByteDiffResult aligned_byte_diff(
    std::span<const std::byte> left,
    std::span<const std::byte> right) {
    ByteDiffResult result{
        .left_size = static_cast<std::uint64_t>(left.size()),
        .right_size = static_cast<std::uint64_t>(right.size()),
        .equal_bytes = 0U,
        .modified_bytes = 0U,
        .inserted_bytes = 0U,
        .removed_bytes = 0U,
        .spans = {},
    };

    const auto shared_size = std::min(left.size(), right.size());
    std::size_t cursor = 0U;

    while (cursor < shared_size) {
        const bool equal = left[cursor] == right[cursor];
        const auto start = cursor;
        ++cursor;

        while (cursor < shared_size &&
            (left[cursor] == right[cursor]) == equal) {
            ++cursor;
        }

        const auto span_size = cursor - start;
        result.spans.push_back(DiffSpan{
            .kind = equal ? DiffKind::equal : DiffKind::modified,
            .left_range = make_range(start, span_size),
            .right_range = make_range(start, span_size),
        });

        if (equal) {
            result.equal_bytes += static_cast<std::uint64_t>(span_size);
        } else {
            result.modified_bytes += static_cast<std::uint64_t>(span_size);
        }
    }

    if (left.size() > shared_size) {
        const auto removed = left.size() - shared_size;
        result.spans.push_back(DiffSpan{
            .kind = DiffKind::removed,
            .left_range = make_range(shared_size, removed),
            .right_range = std::nullopt,
        });
        result.removed_bytes = static_cast<std::uint64_t>(removed);
    }

    if (right.size() > shared_size) {
        const auto inserted = right.size() - shared_size;
        result.spans.push_back(DiffSpan{
            .kind = DiffKind::inserted,
            .left_range = std::nullopt,
            .right_range = make_range(shared_size, inserted),
        });
        result.inserted_bytes = static_cast<std::uint64_t>(inserted);
    }

    return result;
}

bool EntropyOptions::valid() const noexcept {
    return window_size != 0U && step_size != 0U;
}

std::vector<EntropyWindow> entropy_map(
    std::span<const std::byte> bytes,
    EntropyOptions options) {
    std::vector<EntropyWindow> result;
    if (bytes.empty() || !options.valid()) {
        return result;
    }

    std::size_t start = 0U;
    while (start < bytes.size()) {
        const auto remaining = bytes.size() - start;
        const auto window_length = std::min(options.window_size, remaining);
        if (window_length < options.window_size &&
            !options.include_partial_window) {
            break;
        }

        std::array<std::uint64_t, 256U> frequencies{};
        std::size_t zero_count = 0U;

        for (std::size_t index = 0U; index < window_length; ++index) {
            const auto value = std::to_integer<unsigned int>(bytes[start + index]);
            ++frequencies[value];
            if (value == 0U) {
                ++zero_count;
            }
        }

        double entropy = 0.0;
        std::uint16_t unique_count = 0U;
        const auto divisor = static_cast<double>(window_length);

        for (const auto count : frequencies) {
            if (count == 0U) {
                continue;
            }

            ++unique_count;
            const auto probability = static_cast<double>(count) / divisor;
            entropy -= probability * std::log2(probability);
        }

        const auto zero_ratio = static_cast<double>(zero_count) / divisor;
        result.push_back(EntropyWindow{
            .range = make_range(start, window_length),
            .bits_per_byte = entropy,
            .zero_ratio = zero_ratio,
            .unique_byte_count = unique_count,
            .band = classify_entropy(entropy, zero_ratio),
        });

        if (options.step_size > bytes.size() - start) {
            break;
        }
        start += options.step_size;
    }

    return result;
}

} // namespace dmc::rengine::binary
