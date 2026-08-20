#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> as_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const auto character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> patterned_bytes(std::size_t size) {
    std::vector<std::byte> bytes(size);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(
            static_cast<std::uint8_t>((index * 37U + 11U) & 0xFFU));
    }
    return bytes;
}

void assert_chunked_matches(
    std::span<const std::byte> bytes,
    std::span<const std::size_t> chunk_sizes) {
    dmc::rengine::core::Sha256Accumulator accumulator;
    assert(accumulator.valid());
    assert(accumulator.byte_count() == 0U);

    std::size_t cursor = 0U;
    for (const auto requested : chunk_sizes) {
        const auto remaining = bytes.size() - cursor;
        const auto amount = std::min(requested, remaining);
        assert(accumulator.update(bytes.subspan(cursor, amount)));
        cursor += amount;
        if (cursor == bytes.size()) {
            break;
        }
    }
    if (cursor != bytes.size()) {
        assert(accumulator.update(bytes.subspan(cursor)));
    }

    assert(accumulator.valid());
    assert(accumulator.byte_count() == bytes.size());

    const auto expected = dmc::rengine::core::Sha256::compute(bytes);
    const auto first = accumulator.finalize();
    const auto second = accumulator.finalize();
    assert(first.has_value());
    assert(second.has_value());
    assert(*first == expected);
    assert(*second == expected);

    // finalize() is non-destructive: hashing can continue afterwards.
    const std::array<std::byte, 1> suffix{std::byte{0xA5}};
    assert(accumulator.update(suffix));
    std::vector<std::byte> extended(bytes.begin(), bytes.end());
    extended.push_back(suffix.front());
    const auto extended_digest = accumulator.finalize();
    assert(extended_digest.has_value());
    assert(*extended_digest == dmc::rengine::core::Sha256::compute(extended));
}

} // namespace

int main() {
    const std::vector<std::byte> empty;
    const auto empty_digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{empty});
    assert(empty_digest.hex() ==
        "e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");

    const auto abc = as_bytes("abc");
    const auto abc_digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{abc});
    assert(abc_digest.hex() ==
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad");

    const std::array<std::size_t, 9> boundary_sizes{
        0U, 1U, 55U, 56U, 63U, 64U, 65U, 128U, 4097U};
    const std::array<std::size_t, 8> uneven_chunks{
        1U, 7U, 13U, 31U, 64U, 3U, 127U, 19U};
    const std::array<std::size_t, 5> block_crossing_chunks{
        63U, 1U, 64U, 65U, 2U};

    for (const auto size : boundary_sizes) {
        const auto bytes = patterned_bytes(size);
        assert_chunked_matches(bytes, uneven_chunks);
        assert_chunked_matches(bytes, block_crossing_chunks);

        dmc::rengine::core::Sha256Accumulator bytewise;
        for (const auto byte : bytes) {
            const std::array<std::byte, 1> one{byte};
            assert(bytewise.update(one));
        }
        const auto digest = bytewise.finalize();
        assert(digest.has_value());
        assert(*digest == dmc::rengine::core::Sha256::compute(bytes));
    }

    dmc::rengine::core::Sha256Accumulator abc_incremental;
    assert(abc_incremental.update(std::span<const std::byte>{abc}.first(1U)));
    assert(abc_incremental.update(std::span<const std::byte>{abc}.subspan(1U)));
    const auto abc_incremental_digest = abc_incremental.finalize();
    assert(abc_incremental_digest.has_value());
    assert(abc_incremental_digest->hex() == abc_digest.hex());

    return 0;
}
