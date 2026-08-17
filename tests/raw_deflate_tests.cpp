#include "dmc_rengine/core/raw_deflate.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> from_hex(std::string_view hex) {
    assert((hex.size() % 2U) == 0U);
    std::vector<std::byte> bytes;
    bytes.reserve(hex.size() / 2U);
    const auto nibble = [](char value) -> std::uint8_t {
        if (value >= '0' && value <= '9') {
            return static_cast<std::uint8_t>(value - '0');
        }
        if (value >= 'a' && value <= 'f') {
            return static_cast<std::uint8_t>(10 + value - 'a');
        }
        assert(false);
        return 0U;
    };
    for (std::size_t index = 0U; index < hex.size(); index += 2U) {
        bytes.push_back(static_cast<std::byte>(
            static_cast<std::uint8_t>((nibble(hex[index]) << 4U) |
                nibble(hex[index + 1U]))));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> ascii_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const auto value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> repeated(std::string_view unit, std::size_t count) {
    std::string text;
    text.reserve(unit.size() * count);
    for (std::size_t index = 0U; index < count; ++index) {
        text.append(unit);
    }
    return ascii_bytes(text);
}

[[nodiscard]] std::vector<std::byte> dynamic_expected() {
    const std::string base =
        std::string(100U, 'a') +
        std::string(40U, 'b') +
        std::string(20U, 'c') +
        std::string(10U, 'd') +
        "efghijklmnopqrstuvwxyz" +
        "efghijklmnopqrstuvwxyz" +
        "efghijklmnopqrstuvwxyz";

    std::string text;
    text.reserve(5000U);
    while (text.size() < 5000U) {
        text.append(base);
    }
    text.resize(5000U);
    return ascii_bytes(text);
}

} // namespace

int main() {
    using dmc::rengine::core::RawDeflate;
    using dmc::rengine::core::RawDeflateStatus;

    const auto stored_compressed = from_hex(
        "015f00a0ff"
        "73746f7265642d626c6f636b2d70726f6f662d"
        "73746f7265642d626c6f636b2d70726f6f662d"
        "73746f7265642d626c6f636b2d70726f6f662d"
        "73746f7265642d626c6f636b2d70726f6f662d"
        "73746f7265642d626c6f636b2d70726f6f662d");
    const auto stored_expected = repeated("stored-block-proof-", 5U);
    assert(stored_expected.size() == 95U);
    const auto stored = RawDeflate::inflate(stored_compressed, stored_expected.size());
    assert(stored.ok());
    assert(stored.status == RawDeflateStatus::ok);
    assert(stored.bytes == stored_expected);

    const auto fixed_compressed = from_hex(
        "4bcbac484d51c8284d4bcb4dcc532828cacf4fb35270747276190e1800");
    std::string fixed_text = "fixed huffman proof: ";
    for (std::size_t index = 0U; index < 50U; ++index) {
        fixed_text.append("ABCD");
    }
    const auto fixed_expected = ascii_bytes(fixed_text);
    assert(fixed_expected.size() == 221U);
    const auto fixed = RawDeflate::inflate(fixed_compressed, fixed_expected.size());
    assert(fixed.ok());
    assert(fixed.bytes == fixed_expected);

    const auto dynamic_compressed = from_hex(
        "edcf2b1680201400d1b53ebf88a88002c2ea692683068f65264ebb22dfd73cac"
        "bda9bbea87514d7a36cbba59e7f723c474e6f2e60a56ac58b162c58a152b56a"
        "c58b162c58a152b56ac3f592b");
    const auto dynamic_data = dynamic_expected();
    const auto dynamic = RawDeflate::inflate(dynamic_compressed, dynamic_data.size());
    assert(dynamic.ok());
    assert(dynamic.bytes == dynamic_data);

    // RFC 1951 3.2.7 permits a dynamic block whose single declared distance
    // code has code-length zero when the block contains literals only. This
    // vector is a one-literal dynamic stream independently accepted by zlib.
    const auto literal_only_dynamic = from_hex("05c081080000000020b6fda54e");
    const auto literal_only = RawDeflate::inflate(literal_only_dynamic, 1U);
    assert(literal_only.ok());
    assert(literal_only.bytes == ascii_bytes("A"));

    const auto too_small = RawDeflate::inflate(fixed_compressed, 220U);
    assert(!too_small.ok());
    assert(too_small.status == RawDeflateStatus::output_limit_exceeded);
    assert(too_small.bytes.empty());

    const auto too_large = RawDeflate::inflate(fixed_compressed, 222U);
    assert(!too_large.ok());
    assert(too_large.status == RawDeflateStatus::output_size_mismatch);
    assert(too_large.bytes.empty());

    auto truncated = fixed_compressed;
    truncated.pop_back();
    const auto truncated_result = RawDeflate::inflate(truncated, 221U);
    assert(!truncated_result.ok());
    assert(truncated_result.status == RawDeflateStatus::truncated_input);
    assert(truncated_result.bytes.empty());

    auto trailing = fixed_compressed;
    trailing.push_back(std::byte{0});
    const auto trailing_result = RawDeflate::inflate(trailing, 221U);
    assert(!trailing_result.ok());
    assert(trailing_result.status == RawDeflateStatus::trailing_input);

    const std::vector<std::byte> reserved{std::byte{0x07}};
    const auto reserved_result = RawDeflate::inflate(reserved, 0U);
    assert(!reserved_result.ok());
    assert(reserved_result.status == RawDeflateStatus::reserved_block_type);

    auto bad_stored_length = stored_compressed;
    bad_stored_length[3] ^= std::byte{0x01};
    const auto bad_stored = RawDeflate::inflate(
        bad_stored_length, stored_expected.size());
    assert(!bad_stored.ok());
    assert(bad_stored.status == RawDeflateStatus::stored_length_mismatch);

    return 0;
}
