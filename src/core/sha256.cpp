#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <array>
#include <bit>

namespace dmc::rengine::core {
namespace {

constexpr std::array<std::uint32_t, 64> round_constants{
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

using State = std::array<std::uint32_t, 8>;

[[nodiscard]] constexpr std::uint32_t choose(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z) noexcept {
    return (x & y) ^ (~x & z);
}

[[nodiscard]] constexpr std::uint32_t majority(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z) noexcept {
    return (x & y) ^ (x & z) ^ (y & z);
}

[[nodiscard]] constexpr std::uint32_t big_sigma0(std::uint32_t value) noexcept {
    return std::rotr(value, 2) ^ std::rotr(value, 13) ^ std::rotr(value, 22);
}

[[nodiscard]] constexpr std::uint32_t big_sigma1(std::uint32_t value) noexcept {
    return std::rotr(value, 6) ^ std::rotr(value, 11) ^ std::rotr(value, 25);
}

[[nodiscard]] constexpr std::uint32_t small_sigma0(std::uint32_t value) noexcept {
    return std::rotr(value, 7) ^ std::rotr(value, 18) ^ (value >> 3U);
}

[[nodiscard]] constexpr std::uint32_t small_sigma1(std::uint32_t value) noexcept {
    return std::rotr(value, 17) ^ std::rotr(value, 19) ^ (value >> 10U);
}

void transform(State& state, std::span<const std::byte, 64> block) {
    std::array<std::uint32_t, 64> words{};
    for (std::size_t index = 0; index < 16U; ++index) {
        const auto offset = index * 4U;
        words[index] =
            (static_cast<std::uint32_t>(
                 std::to_integer<std::uint8_t>(block[offset]))
             << 24U) |
            (static_cast<std::uint32_t>(
                 std::to_integer<std::uint8_t>(block[offset + 1U]))
             << 16U) |
            (static_cast<std::uint32_t>(
                 std::to_integer<std::uint8_t>(block[offset + 2U]))
             << 8U) |
            static_cast<std::uint32_t>(
                std::to_integer<std::uint8_t>(block[offset + 3U]));
    }

    for (std::size_t index = 16U; index < words.size(); ++index) {
        words[index] = small_sigma1(words[index - 2U]) +
            words[index - 7U] +
            small_sigma0(words[index - 15U]) +
            words[index - 16U];
    }

    auto a = state[0];
    auto b = state[1];
    auto c = state[2];
    auto d = state[3];
    auto e = state[4];
    auto f = state[5];
    auto g = state[6];
    auto h = state[7];

    for (std::size_t index = 0; index < words.size(); ++index) {
        const auto temporary1 = h + big_sigma1(e) + choose(e, f, g) +
            round_constants[index] + words[index];
        const auto temporary2 = big_sigma0(a) + majority(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + temporary1;
        d = c;
        c = b;
        b = a;
        a = temporary1 + temporary2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

} // namespace

std::string Sha256Digest::hex() const {
    constexpr char alphabet[] = "0123456789abcdef";
    std::string result;
    result.resize(bytes.size() * 2U);
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        result[index * 2U] = alphabet[bytes[index] >> 4U];
        result[index * 2U + 1U] = alphabet[bytes[index] & 0x0FU];
    }
    return result;
}

Sha256Digest Sha256::compute(std::span<const std::byte> input) {
    State state{
        0x6a09e667U,
        0xbb67ae85U,
        0x3c6ef372U,
        0xa54ff53aU,
        0x510e527fU,
        0x9b05688cU,
        0x1f83d9abU,
        0x5be0cd19U,
    };

    const auto full_blocks = input.size() / 64U;
    for (std::size_t index = 0; index < full_blocks; ++index) {
        const auto block = input.subspan(index * 64U, 64U);
        transform(state, std::span<const std::byte, 64>{block.data(), 64U});
    }

    std::array<std::byte, 128> tail{};
    const auto remainder = input.size() % 64U;
    if (remainder != 0U) {
        const auto source = input.last(remainder);
        std::copy(source.begin(), source.end(), tail.begin());
    }
    tail[remainder] = std::byte{0x80};

    const auto tail_blocks = remainder < 56U ? 1U : 2U;
    const auto bit_length = static_cast<std::uint64_t>(input.size()) * 8U;
    const auto length_offset = tail_blocks * 64U - 8U;
    for (std::size_t index = 0; index < 8U; ++index) {
        const auto shift = static_cast<unsigned>((7U - index) * 8U);
        tail[length_offset + index] = static_cast<std::byte>(
            (bit_length >> shift) & 0xFFU);
    }

    for (std::size_t index = 0; index < tail_blocks; ++index) {
        const auto* data = tail.data() + index * 64U;
        transform(state, std::span<const std::byte, 64>{data, 64U});
    }

    Sha256Digest digest;
    for (std::size_t word = 0; word < state.size(); ++word) {
        for (std::size_t byte = 0; byte < 4U; ++byte) {
            const auto shift = static_cast<unsigned>((3U - byte) * 8U);
            digest.bytes[word * 4U + byte] = static_cast<std::uint8_t>(
                (state[word] >> shift) & 0xFFU);
        }
    }

    return digest;
}

} // namespace dmc::rengine::core
