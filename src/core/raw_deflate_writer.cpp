#include "dmc_rengine/core/raw_deflate.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::core {
namespace {

constexpr std::size_t stored_block_limit = 0xFFFFU;
constexpr std::size_t stored_block_overhead = 5U;

void append_u16_le(std::vector<std::byte>& output, std::uint16_t value) {
    output.push_back(static_cast<std::byte>(value & 0xFFU));
    output.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

[[nodiscard]] RawDeflateResult encode_failure(std::string detail) {
    return RawDeflateResult{
        .status = RawDeflateStatus::output_limit_exceeded,
        .bytes = {},
        .detail = std::move(detail),
    };
}

} // namespace

RawDeflateResult RawDeflate::deflate_stored(
    std::span<const std::byte> materialized) {
    const auto block_count = materialized.empty()
        ? std::size_t{1U}
        : 1U + (materialized.size() - 1U) / stored_block_limit;

    if (block_count >
            (std::numeric_limits<std::size_t>::max() - materialized.size()) /
                stored_block_overhead) {
        return encode_failure(
            "The deterministic stored-block DEFLATE output exceeds the host size domain.");
    }

    RawDeflateResult result{
        .status = RawDeflateStatus::ok,
        .bytes = {},
        .detail = {},
    };
    result.bytes.reserve(
        materialized.size() + block_count * stored_block_overhead);

    std::size_t cursor = 0U;
    for (std::size_t block = 0U; block < block_count; ++block) {
        const auto remaining = materialized.size() - cursor;
        const auto amount = std::min(stored_block_limit, remaining);
        const bool final_block = block + 1U == block_count;

        // BFINAL occupies bit 0 and BTYPE=00 occupies bits 1..2. The rest of
        // the header byte is the required zero padding to the next byte boundary.
        result.bytes.push_back(
            static_cast<std::byte>(final_block ? 0x01U : 0x00U));

        const auto length = static_cast<std::uint16_t>(amount);
        const auto inverse = static_cast<std::uint16_t>(~length);
        append_u16_le(result.bytes, length);
        append_u16_le(result.bytes, inverse);

        result.bytes.insert(
            result.bytes.end(),
            materialized.begin() + static_cast<std::ptrdiff_t>(cursor),
            materialized.begin() +
                static_cast<std::ptrdiff_t>(cursor + amount));
        cursor += amount;
    }

    if (cursor != materialized.size()) {
        return encode_failure(
            "The deterministic stored-block DEFLATE encoder did not consume the complete input.");
    }
    return result;
}

} // namespace dmc::rengine::core
