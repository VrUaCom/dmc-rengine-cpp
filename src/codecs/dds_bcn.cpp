#include "dmc_rengine/codecs/dds_bcn.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>

namespace dmc::rengine::codecs::dds_bcn {
namespace {

constexpr std::uint32_t k_max_dimension = 16384U;  // D3D11 2D texture limit

[[nodiscard]] std::uint8_t byte_at(const std::byte* p, std::size_t i) noexcept {
    return std::to_integer<std::uint8_t>(p[i]);
}

[[nodiscard]] std::uint16_t u16(const std::byte* p) noexcept {
    return static_cast<std::uint16_t>(byte_at(p, 0U) | (byte_at(p, 1U) << 8U));
}

[[nodiscard]] std::uint32_t u32(const std::byte* p) noexcept {
    return static_cast<std::uint32_t>(byte_at(p, 0U)) |
        (static_cast<std::uint32_t>(byte_at(p, 1U)) << 8U) |
        (static_cast<std::uint32_t>(byte_at(p, 2U)) << 16U) |
        (static_cast<std::uint32_t>(byte_at(p, 3U)) << 24U);
}

[[nodiscard]] std::uint64_t u64(const std::byte* p) noexcept {
    return static_cast<std::uint64_t>(u32(p)) | (static_cast<std::uint64_t>(u32(p + 4U)) << 32U);
}

[[nodiscard]] constexpr std::uint32_t fourcc(char a, char b, char c, char d) noexcept {
    return static_cast<std::uint32_t>(static_cast<std::uint8_t>(a)) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(b)) << 8U) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(c)) << 16U) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(d)) << 24U);
}

// 128-bit little-endian block as a bit stream (bit 0 = LSB of byte 0).
struct Bits final {
    std::uint64_t lo{};
    std::uint64_t hi{};

    [[nodiscard]] std::uint32_t get(std::uint32_t pos, std::uint32_t count) const noexcept {
        if (count == 0U || pos >= 128U) return 0U;
        std::uint64_t v = 0U;
        if (pos >= 64U) {
            v = hi >> (pos - 64U);
        } else if (pos == 0U) {
            v = lo;
        } else {
            v = (lo >> pos) | (hi << (64U - pos));
        }
        return static_cast<std::uint32_t>(v & ((1ULL << count) - 1ULL));
    }
};

// ---- BC1 / BC2 / BC3 (bit-identical to codecs::dds_bc for DXT1 / DXT5) ----

struct Rgba final {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t a{255U};
};

[[nodiscard]] Rgba decode_565(std::uint16_t value) noexcept {
    const auto r5 = static_cast<std::uint32_t>((value >> 11U) & 0x1FU);
    const auto g6 = static_cast<std::uint32_t>((value >> 5U) & 0x3FU);
    const auto b5 = static_cast<std::uint32_t>(value & 0x1FU);
    return {
        static_cast<std::uint8_t>((r5 * 255U + 15U) / 31U),
        static_cast<std::uint8_t>((g6 * 255U + 31U) / 63U),
        static_cast<std::uint8_t>((b5 * 255U + 15U) / 31U),
        255U,
    };
}

[[nodiscard]] Rgba mix(const Rgba& a, const Rgba& b, std::uint32_t wa, std::uint32_t wb,
                       std::uint32_t divisor) noexcept {
    return {
        static_cast<std::uint8_t>((wa * a.r + wb * b.r) / divisor),
        static_cast<std::uint8_t>((wa * a.g + wb * b.g) / divisor),
        static_cast<std::uint8_t>((wa * a.b + wb * b.b) / divisor),
        255U,
    };
}

void put(std::uint8_t* out, std::uint32_t i, const Rgba& c) noexcept {
    out[i * 4U + 0U] = c.r;
    out[i * 4U + 1U] = c.g;
    out[i * 4U + 2U] = c.b;
    out[i * 4U + 3U] = c.a;
}

// BC1 colour half. `three_color_allowed` is false inside BC2/BC3.
void decode_color(const std::byte* block, bool three_color_allowed, std::uint8_t* out) noexcept {
    const auto c0_raw = u16(block);
    const auto c1_raw = u16(block + 2U);
    std::array<Rgba, 4U> palette{};
    palette[0] = decode_565(c0_raw);
    palette[1] = decode_565(c1_raw);
    if (!three_color_allowed || c0_raw > c1_raw) {
        palette[2] = mix(palette[0], palette[1], 2U, 1U, 3U);
        palette[3] = mix(palette[0], palette[1], 1U, 2U, 3U);
    } else {
        palette[2] = mix(palette[0], palette[1], 1U, 1U, 2U);
        palette[3] = {0U, 0U, 0U, 0U};
    }
    const auto indices = u32(block + 4U);
    for (std::uint32_t i = 0U; i < 16U; ++i) put(out, i, palette[(indices >> (i * 2U)) & 3U]);
}

// BC3 alpha / BC4 UNORM 8-value ramp over one 8-byte half, written to channel `ch`.
void decode_unorm_ramp(const std::byte* block, std::uint8_t* out, std::uint32_t ch) noexcept {
    const std::uint32_t a0 = byte_at(block, 0U);
    const std::uint32_t a1 = byte_at(block, 1U);
    std::array<std::uint8_t, 8U> v{};
    v[0] = static_cast<std::uint8_t>(a0);
    v[1] = static_cast<std::uint8_t>(a1);
    if (a0 > a1) {
        for (std::uint32_t i = 1U; i <= 6U; ++i) {
            v[i + 1U] = static_cast<std::uint8_t>(((7U - i) * a0 + i * a1) / 7U);
        }
    } else {
        for (std::uint32_t i = 1U; i <= 4U; ++i) {
            v[i + 1U] = static_cast<std::uint8_t>(((5U - i) * a0 + i * a1) / 5U);
        }
        v[6] = 0U;
        v[7] = 255U;
    }
    const auto bits = u64(block) >> 16U;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        out[i * 4U + ch] = v[static_cast<std::size_t>((bits >> (i * 3U)) & 7U)];
    }
}

// BC4 / BC5 SNORM ramp; [-1, 1] mapped to [0, 255].
void decode_snorm_ramp(const std::byte* block, std::uint8_t* out, std::uint32_t ch) noexcept {
    auto r0 = static_cast<std::int32_t>(static_cast<std::int8_t>(byte_at(block, 0U)));
    auto r1 = static_cast<std::int32_t>(static_cast<std::int8_t>(byte_at(block, 1U)));
    r0 = std::max(r0, -127);
    r1 = std::max(r1, -127);
    std::array<float, 8U> v{};
    v[0] = static_cast<float>(r0);
    v[1] = static_cast<float>(r1);
    if (r0 > r1) {
        for (int i = 1; i <= 6; ++i) {
            v[static_cast<std::size_t>(i + 1)] = static_cast<float>((7 - i) * r0 + i * r1) / 7.0F;
        }
    } else {
        for (int i = 1; i <= 4; ++i) {
            v[static_cast<std::size_t>(i + 1)] = static_cast<float>((5 - i) * r0 + i * r1) / 5.0F;
        }
        v[6] = -127.0F;
        v[7] = 127.0F;
    }
    const auto bits = u64(block) >> 16U;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        const float s = v[static_cast<std::size_t>((bits >> (i * 3U)) & 7U)];
        out[i * 4U + ch] = static_cast<std::uint8_t>(std::lround((s + 127.0F) * 255.0F / 254.0F));
    }
}

// ---- BC7 / BC6H shared tables (BPTC spec / D3D11 functional spec) ----

// Two-subset partitions, bit i = subset of pixel i.
constexpr std::array<std::uint16_t, 64U> k_partition2{
    0xcccc, 0x8888, 0xeeee, 0xecc8, 0xc880, 0xfeec, 0xfec8, 0xec80, 0xc800, 0xffec,
    0xfe80, 0xe800, 0xffe8, 0xff00, 0xfff0, 0xf000, 0xf710, 0x008e, 0x7100, 0x08ce,
    0x008c, 0x7310, 0x3100, 0x8cce, 0x088c, 0x3110, 0x6666, 0x366c, 0x17e8, 0x0ff0,
    0x718e, 0x399c, 0xaaaa, 0xf0f0, 0x5a5a, 0x33cc, 0x3c3c, 0x55aa, 0x9696, 0xa55a,
    0x73ce, 0x13c8, 0x324c, 0x3bdc, 0x6996, 0xc33c, 0x9966, 0x0660, 0x0272, 0x04e4,
    0x4e40, 0x2720, 0xc936, 0x936c, 0x39c6, 0x639c, 0x9336, 0x9cc6, 0x817e, 0xe718,
    0xccf0, 0x0fcc, 0x7744, 0xee22,
};

// Three-subset partitions, 2 bits per pixel.
constexpr std::array<std::uint32_t, 64U> k_partition3{
    0xaa685050, 0x6a5a5040, 0x5a5a4200, 0x5450a0a8, 0xa5a50000, 0xa0a05050, 0x5555a0a0,
    0x5a5a5050, 0xaa550000, 0xaa555500, 0xaaaa5500, 0x90909090, 0x94949494, 0xa4a4a4a4,
    0xa9a59450, 0x2a0a4250, 0xa5945040, 0x0a425054, 0xa5a5a500, 0x55a0a0a0, 0xa8a85454,
    0x6a6a4040, 0xa4a45000, 0x1a1a0500, 0x0050a4a4, 0xaaa59090, 0x14696914, 0x69691400,
    0xa08585a0, 0xaa821414, 0x50a4a450, 0x6a5a0200, 0xa9a58000, 0x5090a0a8, 0xa8a09050,
    0x24242424, 0x00aa5500, 0x24924924, 0x24499224, 0x50a50a50, 0x500aa550, 0xaaaa4444,
    0x66660000, 0xa5a0a5a0, 0x50a050a0, 0x69286928, 0x44aaaa44, 0x66666600, 0xaa444444,
    0x54a854a8, 0x95809580, 0x96969600, 0xa85454a8, 0x80959580, 0xaa141414, 0x96960000,
    0xaaaa1414, 0xa05050a0, 0xa0a5a5a0, 0x96000000, 0x40804080, 0xa9a8a9a8, 0xaaaaaa44,
    0x2a4a5254,
};

// Anchor pixel of subset 1 (two subsets), subsets 1 and 2 (three subsets).
constexpr std::array<std::uint8_t, 64U> k_anchor2{
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 2,  8,  2,  2,  8,  8,  15, 2,  8,  2,  2,  8,  8,  2,  2,
    15, 15, 6,  8,  2,  8,  15, 15, 2,  8,  2,  2,  2,  15, 15, 6,
    6,  2,  6,  8,  15, 15, 2,  2,  15, 15, 15, 15, 15, 2,  2,  15,
};
constexpr std::array<std::uint8_t, 64U> k_anchor3a{
    3,  3,  15, 15, 8,  3,  15, 15, 8,  8,  6,  6,  6,  5,  3,  3,
    3,  3,  8,  15, 3,  3,  6,  10, 5,  8,  8,  6,  8,  5,  15, 15,
    8,  15, 3,  5,  6,  10, 8,  15, 15, 3,  15, 5,  15, 15, 15, 15,
    3,  15, 5,  5,  5,  8,  5,  10, 5,  10, 8,  13, 15, 12, 3,  3,
};
constexpr std::array<std::uint8_t, 64U> k_anchor3b{
    15, 8,  8,  3,  15, 15, 3,  8,  15, 15, 15, 15, 15, 15, 15, 8,
    15, 8,  15, 3,  15, 8,  15, 8,  3,  15, 6,  10, 15, 15, 10, 8,
    15, 3,  15, 10, 10, 8,  9,  10, 6,  15, 8,  15, 3,  6,  6,  8,
    15, 3,  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 3,  15, 15, 8,
};

constexpr std::array<std::uint32_t, 4U> k_weights2{0, 21, 43, 64};
constexpr std::array<std::uint32_t, 8U> k_weights3{0, 9, 18, 27, 37, 46, 55, 64};
constexpr std::array<std::uint32_t, 16U> k_weights4{
    0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64,
};

[[nodiscard]] std::uint32_t weight(std::uint32_t index_bits, std::uint32_t index) noexcept {
    if (index_bits == 2U) return k_weights2[index & 3U];
    if (index_bits == 3U) return k_weights3[index & 7U];
    return k_weights4[index & 15U];
}

[[nodiscard]] std::uint32_t subset_of(std::uint32_t subsets, std::uint32_t partition,
                                      std::uint32_t pixel) noexcept {
    if (subsets == 2U) return (k_partition2[partition] >> pixel) & 1U;
    if (subsets == 3U) return (k_partition3[partition] >> (pixel * 2U)) & 3U;
    return 0U;
}

[[nodiscard]] bool is_anchor(std::uint32_t subsets, std::uint32_t partition,
                             std::uint32_t pixel) noexcept {
    if (pixel == 0U) return true;
    if (subsets == 2U) return pixel == k_anchor2[partition];
    if (subsets == 3U) return pixel == k_anchor3a[partition] || pixel == k_anchor3b[partition];
    return false;
}

// ---- BC7 ----

struct Bc7Mode final {
    std::uint8_t subsets;
    std::uint8_t partition_bits;
    std::uint8_t rotation_bits;
    std::uint8_t index_selection_bits;
    std::uint8_t color_bits;
    std::uint8_t alpha_bits;
    std::uint8_t endpoint_pbits;
    std::uint8_t shared_pbits;
    std::uint8_t index_bits;
    std::uint8_t index2_bits;
};

constexpr std::array<Bc7Mode, 8U> k_bc7_modes{{
    {3, 4, 0, 0, 4, 0, 1, 0, 3, 0},
    {2, 6, 0, 0, 6, 0, 0, 1, 3, 0},
    {3, 6, 0, 0, 5, 0, 0, 0, 2, 0},
    {2, 6, 0, 0, 7, 0, 1, 0, 2, 0},
    {1, 0, 2, 1, 5, 6, 0, 0, 2, 3},
    {1, 0, 2, 0, 7, 8, 0, 0, 2, 2},
    {1, 0, 0, 0, 7, 7, 1, 0, 4, 0},
    {2, 6, 0, 0, 5, 5, 1, 0, 2, 0},
}};

[[nodiscard]] std::uint32_t expand_bits(std::uint32_t v, std::uint32_t bits) noexcept {
    if (bits >= 8U) return v & 0xFFU;
    v <<= (8U - bits);
    return (v | (v >> bits)) & 0xFFU;
}

void decode_bc7(const std::byte* block, std::uint8_t* out) noexcept {
    const Bits b{u64(block), u64(block + 8U)};
    const auto first = byte_at(block, 0U);
    if (first == 0U) {  // reserved mode 8
        std::memset(out, 0, 64U);
        return;
    }
    const auto mode_index = static_cast<std::uint32_t>(std::countr_zero(first));
    const auto& m = k_bc7_modes[mode_index];
    std::uint32_t pos = mode_index + 1U;

    const auto partition = b.get(pos, m.partition_bits);
    pos += m.partition_bits;
    const auto rotation = b.get(pos, m.rotation_bits);
    pos += m.rotation_bits;
    const auto index_selection = b.get(pos, m.index_selection_bits);
    pos += m.index_selection_bits;

    const std::uint32_t endpoints = m.subsets * 2U;
    std::array<std::array<std::uint32_t, 4U>, 6U> ep{};
    for (std::uint32_t c = 0U; c < 3U; ++c) {
        for (std::uint32_t e = 0U; e < endpoints; ++e) {
            ep[e][c] = b.get(pos, m.color_bits);
            pos += m.color_bits;
        }
    }
    for (std::uint32_t e = 0U; e < endpoints; ++e) {
        if (m.alpha_bits != 0U) {
            ep[e][3] = b.get(pos, m.alpha_bits);
            pos += m.alpha_bits;
        } else {
            ep[e][3] = 255U;
        }
    }

    std::uint32_t color_bits = m.color_bits;
    std::uint32_t alpha_bits = m.alpha_bits;
    if (m.endpoint_pbits != 0U || m.shared_pbits != 0U) {
        ++color_bits;
        if (alpha_bits != 0U) ++alpha_bits;
        // One p-bit per endpoint, or one per subset shared by both endpoints.
        for (std::uint32_t e = 0U; e < endpoints; ++e) {
            const auto p = b.get(pos + (m.endpoint_pbits != 0U ? e : e / 2U), 1U);
            for (std::uint32_t c = 0U; c < 3U; ++c) ep[e][c] = (ep[e][c] << 1U) | p;
            if (m.alpha_bits != 0U) ep[e][3] = (ep[e][3] << 1U) | p;
        }
        pos += m.endpoint_pbits != 0U ? endpoints : m.subsets;
    }
    for (std::uint32_t e = 0U; e < endpoints; ++e) {
        for (std::uint32_t c = 0U; c < 3U; ++c) ep[e][c] = expand_bits(ep[e][c], color_bits);
        if (m.alpha_bits != 0U) ep[e][3] = expand_bits(ep[e][3], alpha_bits);
    }

    std::uint32_t index_pos = pos;
    std::uint32_t index2_pos = pos + 16U * m.index_bits - m.subsets;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        const auto s = subset_of(m.subsets, partition, i);
        const auto bits1 = m.index_bits - (is_anchor(m.subsets, partition, i) ? 1U : 0U);
        const auto i1 = b.get(index_pos, bits1);
        index_pos += bits1;
        std::uint32_t color_w = weight(m.index_bits, i1);
        std::uint32_t alpha_w = color_w;
        if (m.index2_bits != 0U) {
            const auto bits2 = m.index2_bits - (i == 0U ? 1U : 0U);
            const auto i2 = b.get(index2_pos, bits2);
            index2_pos += bits2;
            const auto w2 = weight(m.index2_bits, i2);
            if (index_selection != 0U) {
                color_w = w2;
            } else {
                alpha_w = w2;
            }
        }
        const auto& e0 = ep[s * 2U];
        const auto& e1 = ep[s * 2U + 1U];
        std::array<std::uint32_t, 4U> px{};
        for (std::uint32_t c = 0U; c < 4U; ++c) {
            const auto w = c == 3U ? alpha_w : color_w;
            px[c] = ((64U - w) * e0[c] + w * e1[c] + 32U) >> 6U;
        }
        if (rotation != 0U) std::swap(px[rotation - 1U], px[3]);
        for (std::uint32_t c = 0U; c < 4U; ++c) out[i * 4U + c] = static_cast<std::uint8_t>(px[c]);
    }
}

// ---- BC6H ----

struct Bc6Mode final {
    std::uint8_t subsets;
    std::uint8_t transformed;
    std::uint8_t partition_bits;
    std::uint8_t endpoint_bits;
    std::uint8_t delta_r;
    std::uint8_t delta_g;
    std::uint8_t delta_b;
};

// Indexed as decoded below: 0..1 (2-bit mode), 2..9 (5-bit 0bxxx10),
// 10..13 (5-bit 0bxxx11).
constexpr std::array<Bc6Mode, 14U> k_bc6_modes{{
    {2, 1, 5, 10, 5, 5, 5},
    {2, 1, 5, 7, 6, 6, 6},
    {2, 1, 5, 11, 5, 4, 4},
    {2, 1, 5, 11, 4, 5, 4},
    {2, 1, 5, 11, 4, 4, 5},
    {2, 1, 5, 9, 5, 5, 5},
    {2, 1, 5, 8, 6, 5, 5},
    {2, 1, 5, 8, 5, 6, 5},
    {2, 1, 5, 8, 5, 5, 6},
    {2, 0, 5, 6, 6, 6, 6},
    {1, 0, 0, 10, 10, 10, 10},
    {1, 1, 0, 11, 9, 9, 9},
    {1, 1, 0, 12, 8, 8, 8},
    {1, 1, 0, 16, 4, 4, 4},
}};

// Endpoint bit layout per mode (BPTC Table.F): each entry is
// (component << 4) | bit, component 0..11 = rw gw bw rx gx bx ry gy by rz gz bz.
constexpr std::array<std::array<std::uint8_t, 75U>, 14U> k_bc6_layout{{
    {116, 132, 180, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   16,  17,
     18,  19,  20,  21,  22,  23,  24,  25,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  48,  49,  50,  51,  52,  164, 112, 113, 114, 115, 64,  65,
     66,  67,  68,  176, 160, 161, 162, 163, 80,  81,  82,  83,  84,  177, 128,
     129, 130, 131, 96,  97,  98,  99,  100, 178, 144, 145, 146, 147, 148, 179},
    {117, 164, 165, 0,  1,   2,   3,   4,   5,   6,   176, 177, 132, 16,  17,
     18,  19,  20,  21, 22,  133, 178, 116, 32,  33,  34,  35,  36,  37,  38,
     179, 181, 180, 48, 49,  50,  51,  52,  53,  112, 113, 114, 115, 64,  65,
     66,  67,  68,  69, 160, 161, 162, 163, 80,  81,  82,  83,  84,  85,  128,
     129, 130, 131, 96, 97,  98,  99,  100, 101, 144, 145, 146, 147, 148, 149},
    {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   16,  17,  18,  19,  20,
     21,  22,  23,  24,  25,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
     48,  49,  50,  51,  52,  10,  112, 113, 114, 115, 64,  65,  66,  67,  26,
     176, 160, 161, 162, 163, 80,  81,  82,  83,  42,  177, 128, 129, 130, 131,
     96,  97,  98,  99,  100, 178, 144, 145, 146, 147, 148, 179},
    {0,  1,   2,   3,   4,   5,   6,   7,   8,   9,   16,  17,  18,  19,  20,
     21, 22,  23,  24,  25,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
     48, 49,  50,  51,  10,  164, 112, 113, 114, 115, 64,  65,  66,  67,  68,
     26, 160, 161, 162, 163, 80,  81,  82,  83,  42,  177, 128, 129, 130, 131,
     96, 97,  98,  99,  176, 178, 144, 145, 146, 147, 116, 179},
    {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   16,  17,  18,  19,  20,
     21,  22,  23,  24,  25,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
     48,  49,  50,  51,  10,  132, 112, 113, 114, 115, 64,  65,  66,  67,  26,
     176, 160, 161, 162, 163, 80,  81,  82,  83,  84,  42,  128, 129, 130, 131,
     96,  97,  98,  99,  177, 178, 144, 145, 146, 147, 180, 179},
    {0,   1,   2,   3,   4,   5,   6,   7,   8,   132, 16,  17,  18,  19,  20,
     21,  22,  23,  24,  116, 32,  33,  34,  35,  36,  37,  38,  39,  40,  180,
     48,  49,  50,  51,  52,  164, 112, 113, 114, 115, 64,  65,  66,  67,  68,
     176, 160, 161, 162, 163, 80,  81,  82,  83,  84,  177, 128, 129, 130, 131,
     96,  97,  98,  99,  100, 178, 144, 145, 146, 147, 148, 179},
    {0,   1,   2,   3,   4,   5,   6,   7,   164, 132, 16,  17,  18,  19,  20,
     21,  22,  23,  178, 116, 32,  33,  34,  35,  36,  37,  38,  39,  179, 180,
     48,  49,  50,  51,  52,  53,  112, 113, 114, 115, 64,  65,  66,  67,  68,
     176, 160, 161, 162, 163, 80,  81,  82,  83,  84,  177, 128, 129, 130, 131,
     96,  97,  98,  99,  100, 101, 144, 145, 146, 147, 148, 149},
    {0,  1,   2,   3,   4,   5,   6,   7,   176, 132, 16,  17,  18,  19,  20,
     21, 22,  23,  117, 116, 32,  33,  34,  35,  36,  37,  38,  39,  165, 180,
     48, 49,  50,  51,  52,  164, 112, 113, 114, 115, 64,  65,  66,  67,  68,
     69, 160, 161, 162, 163, 80,  81,  82,  83,  84,  177, 128, 129, 130, 131,
     96, 97,  98,  99,  100, 178, 144, 145, 146, 147, 148, 179},
    {0,   1,   2,   3,   4,   5,   6,   7,   177, 132, 16,  17,  18,  19,  20,
     21,  22,  23,  133, 116, 32,  33,  34,  35,  36,  37,  38,  39,  181, 180,
     48,  49,  50,  51,  52,  164, 112, 113, 114, 115, 64,  65,  66,  67,  68,
     176, 160, 161, 162, 163, 80,  81,  82,  83,  84,  85,  128, 129, 130, 131,
     96,  97,  98,  99,  100, 178, 144, 145, 146, 147, 148, 179},
    {0,  1,   2,   3,   4,   5,   164, 176, 177, 132, 16,  17,  18,  19,  20,
     21, 117, 133, 178, 116, 32,  33,  34,  35,  36,  37,  165, 179, 181, 180,
     48, 49,  50,  51,  52,  53,  112, 113, 114, 115, 64,  65,  66,  67,  68,
     69, 160, 161, 162, 163, 80,  81,  82,  83,  84,  85,  128, 129, 130, 131,
     96, 97,  98,  99,  100, 101, 144, 145, 146, 147, 148, 149},
    {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89},
    {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 56, 10,
     64, 65, 66, 67, 68, 69, 70, 71, 72, 26, 80, 81, 82, 83, 84, 85, 86, 87, 88, 42},
    {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 54, 55, 11, 10,
     64, 65, 66, 67, 68, 69, 70, 71, 27, 26, 80, 81, 82, 83, 84, 85, 86, 87, 43, 42},
    {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 15, 14, 13, 12, 11, 10,
     64, 65, 66, 67, 31, 30, 29, 28, 27, 26, 80, 81, 82, 83, 47, 46, 45, 44, 43, 42},
}};

[[nodiscard]] std::int32_t sign_extend(std::int32_t v, std::uint32_t bits) noexcept {
    if (bits == 0U || bits >= 32U) return v;
    const auto mask = static_cast<std::int32_t>((1U << bits) - 1U);
    v &= mask;
    if ((v & (1 << (bits - 1U))) != 0) v -= (1 << bits);
    return v;
}

[[nodiscard]] std::int32_t unquantize(std::int32_t x, std::uint32_t bits, bool is_signed) noexcept {
    if (!is_signed) {
        if (bits >= 15U) return x;
        if (x == 0) return 0;
        if (x == static_cast<std::int32_t>((1U << bits) - 1U)) return 0xFFFF;
        return ((x << 15) + 0x4000) >> (bits - 1U);
    }
    if (bits >= 16U) return x;
    bool negative = false;
    if (x < 0) {
        negative = true;
        x = -x;
    }
    std::int32_t q = 0;
    if (x != 0) {
        q = x >= static_cast<std::int32_t>((1U << (bits - 1U)) - 1U)
            ? 0x7FFF
            : ((x << 15) + 0x4000) >> (bits - 1U);
    }
    return negative ? -q : q;
}

[[nodiscard]] float half_to_float(std::uint16_t h) noexcept {
    const std::uint32_t sign = static_cast<std::uint32_t>(h & 0x8000U) << 16U;
    const std::uint32_t exponent = (h >> 10U) & 0x1FU;
    std::uint32_t mantissa = h & 0x3FFU;
    std::uint32_t bits = 0U;
    if (exponent == 0U) {
        if (mantissa == 0U) {
            bits = sign;
        } else {  // subnormal half -> normal float
            std::uint32_t e = 113U;
            while ((mantissa & 0x400U) == 0U) {
                mantissa <<= 1U;
                --e;
            }
            bits = sign | (e << 23U) | ((mantissa & 0x3FFU) << 13U);
        }
    } else if (exponent == 31U) {
        bits = sign | 0x7F800000U | (mantissa << 13U);
    } else {
        bits = sign | ((exponent + 112U) << 23U) | (mantissa << 13U);
    }
    return std::bit_cast<float>(bits);
}

[[nodiscard]] std::uint8_t hdr_to_byte(std::int32_t v, bool is_signed) noexcept {
    std::uint16_t half = 0U;
    if (is_signed) {
        half = v < 0 ? static_cast<std::uint16_t>(0x8000U | static_cast<std::uint32_t>(((-v) * 31) >> 5))
                     : static_cast<std::uint16_t>((v * 31) >> 5);
    } else {
        half = static_cast<std::uint16_t>((v * 31) >> 6);
    }
    const float f = half_to_float(half);
    if (!(f > 0.0F)) return 0U;  // negatives and NaN
    if (f >= 1.0F) return 255U;
    return static_cast<std::uint8_t>(std::lround(f * 255.0F));
}

void decode_bc6h(const std::byte* block, std::uint8_t* out, bool is_signed) noexcept {
    const Bits b{u64(block), u64(block + 8U)};
    const auto low = b.get(0U, 5U);
    std::uint32_t mode = 0U;
    std::uint32_t pos = 5U;
    std::uint32_t layout_bits = 75U;
    std::uint32_t index_bits = 3U;
    if ((low & 3U) < 2U) {
        mode = low & 3U;
        pos = 2U;
    } else if ((low & 3U) == 2U) {
        mode = 2U + (low >> 2U);
        layout_bits = 72U;
    } else {
        mode = 10U + (low >> 2U);
        layout_bits = 60U;
        index_bits = 4U;
    }
    if (mode >= 14U) {  // reserved
        for (std::uint32_t i = 0U; i < 16U; ++i) put(out, i, Rgba{0U, 0U, 0U, 255U});
        return;
    }
    const auto& m = k_bc6_modes[mode];
    std::array<std::int32_t, 12U> ep{};
    for (std::uint32_t i = 0U; i < layout_bits; ++i) {
        const auto code = k_bc6_layout[mode][i];
        ep[code >> 4U] |= static_cast<std::int32_t>(b.get(pos + i, 1U) << (code & 15U));
    }
    pos += layout_bits;
    const auto partition = b.get(pos, m.partition_bits);
    pos += m.partition_bits;

    const std::uint32_t count = m.subsets == 2U ? 12U : 6U;
    const std::array<std::uint32_t, 3U> delta_bits{m.delta_r, m.delta_g, m.delta_b};
    if (is_signed) {
        for (std::uint32_t c = 0U; c < 3U; ++c) ep[c] = sign_extend(ep[c], m.endpoint_bits);
    }
    if (is_signed || m.transformed != 0U) {
        for (std::uint32_t i = 3U; i < count; ++i) ep[i] = sign_extend(ep[i], delta_bits[i % 3U]);
    }
    if (m.transformed != 0U) {
        const auto mask = static_cast<std::int32_t>((1U << m.endpoint_bits) - 1U);
        for (std::uint32_t i = 3U; i < count; ++i) {
            ep[i] = (ep[i] + ep[i % 3U]) & mask;
            if (is_signed) ep[i] = sign_extend(ep[i], m.endpoint_bits);
        }
    }
    for (std::uint32_t i = 0U; i < count; ++i) ep[i] = unquantize(ep[i], m.endpoint_bits, is_signed);

    for (std::uint32_t i = 0U; i < 16U; ++i) {
        const auto s = subset_of(m.subsets, partition, i);
        const auto bits = index_bits - (is_anchor(m.subsets, partition, i) ? 1U : 0U);
        const auto w = static_cast<std::int32_t>(weight(index_bits, b.get(pos, bits)));
        pos += bits;
        Rgba px{};
        std::array<std::uint8_t*, 3U> dst{&px.r, &px.g, &px.b};
        for (std::uint32_t c = 0U; c < 3U; ++c) {
            const auto e0 = ep[s * 6U + c];
            const auto e1 = ep[s * 6U + 3U + c];
            *dst[c] = hdr_to_byte((e0 * (64 - w) + e1 * w + 32) >> 6, is_signed);
        }
        put(out, i, px);
    }
}

// ---- header ----

[[nodiscard]] ParseResult failure(Status status, std::string_view detail) noexcept {
    return ParseResult{.status = status, .document = {}, .detail = detail};
}

struct FormatInfo final {
    bool ok{};
    Format format{};
    bool srgb{};
    bool premultiplied{};
};

[[nodiscard]] FormatInfo from_fourcc(std::uint32_t code) noexcept {
    switch (code) {
    case fourcc('D', 'X', 'T', '1'): return {true, Format::bc1, false, false};
    case fourcc('D', 'X', 'T', '2'): return {true, Format::bc2, false, true};
    case fourcc('D', 'X', 'T', '3'): return {true, Format::bc2, false, false};
    case fourcc('D', 'X', 'T', '4'): return {true, Format::bc3, false, true};
    case fourcc('D', 'X', 'T', '5'): return {true, Format::bc3, false, false};
    case fourcc('A', 'T', 'I', '1'):
    case fourcc('B', 'C', '4', 'U'): return {true, Format::bc4_unorm, false, false};
    case fourcc('B', 'C', '4', 'S'): return {true, Format::bc4_snorm, false, false};
    case fourcc('A', 'T', 'I', '2'):
    case fourcc('B', 'C', '5', 'U'): return {true, Format::bc5_unorm, false, false};
    case fourcc('B', 'C', '5', 'S'): return {true, Format::bc5_snorm, false, false};
    default: return {};
    }
}

[[nodiscard]] FormatInfo from_dxgi(std::uint32_t dxgi) noexcept {
    switch (dxgi) {
    case 70U: case 71U: return {true, Format::bc1, false, false};
    case 72U: return {true, Format::bc1, true, false};
    case 73U: case 74U: return {true, Format::bc2, false, false};
    case 75U: return {true, Format::bc2, true, false};
    case 76U: case 77U: return {true, Format::bc3, false, false};
    case 78U: return {true, Format::bc3, true, false};
    case 79U: case 80U: return {true, Format::bc4_unorm, false, false};
    case 81U: return {true, Format::bc4_snorm, false, false};
    case 82U: case 83U: return {true, Format::bc5_unorm, false, false};
    case 84U: return {true, Format::bc5_snorm, false, false};
    case 94U: case 95U: return {true, Format::bc6h_uf16, false, false};
    case 96U: return {true, Format::bc6h_sf16, false, false};
    case 97U: case 98U: return {true, Format::bc7, false, false};
    case 99U: return {true, Format::bc7, true, false};
    default: return {};
    }
}

[[nodiscard]] bool level_offset(const Document& d, std::uint32_t level, std::uint64_t* offset) noexcept {
    std::uint64_t total = d.header_size;
    std::uint32_t w = d.width;
    std::uint32_t h = d.height;
    for (std::uint32_t l = 0U; l < level; ++l) {
        std::uint64_t bytes = 0U;
        if (!level_size(w, h, d.format, &bytes)) return false;
        total += bytes;
        w = std::max(1U, w / 2U);
        h = std::max(1U, h / 2U);
    }
    *offset = total;
    return true;
}

// Decodes block row `by` of a level into `strip` (4 rows of `width` RGBA8 pixels).
void decode_block_row(const std::byte* level, const Document& d, std::uint32_t width,
                      std::uint32_t by, std::uint8_t* strip) noexcept {
    const auto blocks_w = (width + 3U) / 4U;
    const auto bsize = block_bytes(d.format);
    std::array<std::uint8_t, 64U> px{};
    for (std::uint32_t bx = 0U; bx < blocks_w; ++bx) {
        decode_block(d.format,
                     level + (static_cast<std::size_t>(by) * blocks_w + bx) * bsize,
                     px.data());
        for (std::uint32_t py = 0U; py < 4U; ++py) {
            for (std::uint32_t pxi = 0U; pxi < 4U; ++pxi) {
                const auto x = bx * 4U + pxi;
                if (x >= width) continue;
                std::memcpy(strip + (static_cast<std::size_t>(py) * width + x) * 4U,
                            px.data() + (py * 4U + pxi) * 4U, 4U);
            }
        }
    }
}

}  // namespace

std::string_view to_string(Status status) noexcept {
    switch (status) {
    case Status::ok: return "ok";
    case Status::truncated: return "truncated";
    case Status::invalid_magic: return "invalid-magic";
    case Status::invalid_header: return "invalid-header";
    case Status::invalid_dimensions: return "invalid-dimensions";
    case Status::invalid_mip_count: return "invalid-mip-count";
    case Status::unsupported_format: return "unsupported-format";
    case Status::payload_overflow: return "payload-overflow";
    case Status::payload_out_of_bounds: return "payload-out-of-bounds";
    }
    return "invalid-header";
}

std::string_view format_name(Format format) noexcept {
    switch (format) {
    case Format::bc1: return "BC1 (DXT1)";
    case Format::bc2: return "BC2 (DXT3)";
    case Format::bc3: return "BC3 (DXT5)";
    case Format::bc4_unorm: return "BC4";
    case Format::bc4_snorm: return "BC4 SNORM";
    case Format::bc5_unorm: return "BC5";
    case Format::bc5_snorm: return "BC5 SNORM";
    case Format::bc6h_uf16: return "BC6H UF16";
    case Format::bc6h_sf16: return "BC6H SF16";
    case Format::bc7: return "BC7";
    }
    return "BC?";
}

std::uint32_t block_bytes(Format format) noexcept {
    return format == Format::bc1 || format == Format::bc4_unorm || format == Format::bc4_snorm
        ? 8U
        : 16U;
}

bool Document::valid() const noexcept {
    return width != 0U && height != 0U && mip_count != 0U && payload_size != 0U &&
        (header_size == legacy_header_size || header_size == dx10_header_size) &&
        total_size == header_size + payload_size;
}

bool ParseResult::ok() const noexcept {
    return status == Status::ok && document.valid();
}

bool RgbaImage::available() const noexcept {
    if (width == 0U || height == 0U) return false;
    const auto pixels = static_cast<std::uint64_t>(width) * height;
    return pixels <= std::numeric_limits<std::size_t>::max() / 4U &&
        rgba8.size() == static_cast<std::size_t>(pixels * 4U);
}

std::uint32_t maximum_mip_count(std::uint32_t width, std::uint32_t height) noexcept {
    if (width == 0U || height == 0U) return 0U;
    auto dimension = std::max(width, height);
    std::uint32_t count = 1U;
    while (dimension > 1U) {
        dimension /= 2U;
        ++count;
    }
    return count;
}

bool level_size(std::uint32_t width, std::uint32_t height, Format format,
                std::uint64_t* output) noexcept {
    if (output == nullptr || width == 0U || height == 0U) return false;
    const auto blocks = ((static_cast<std::uint64_t>(width) + 3U) / 4U) *
        ((static_cast<std::uint64_t>(height) + 3U) / 4U);
    *output = blocks * block_bytes(format);
    return true;
}

bool chain_size(std::uint32_t width, std::uint32_t height, std::uint32_t mip_count, Format format,
                std::uint64_t* output) noexcept {
    if (output == nullptr || mip_count == 0U || mip_count > maximum_mip_count(width, height)) {
        return false;
    }
    std::uint64_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        std::uint64_t bytes = 0U;
        if (!level_size(width, height, format, &bytes)) return false;
        total += bytes;
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    *output = total;
    return true;
}

ParseResult parse(std::span<const std::byte> bytes) noexcept {
    if (bytes.size() < legacy_header_size) {
        return failure(Status::truncated, "DDS is shorter than the 128-byte header");
    }
    const auto* p = bytes.data();
    if (u32(p) != fourcc('D', 'D', 'S', ' ')) {
        return failure(Status::invalid_magic, "DDS magic is not present");
    }
    if (u32(p + 4U) != 124U || u32(p + 76U) != 32U || (u32(p + 80U) & 4U) == 0U) {
        return failure(Status::invalid_header, "DDS header is not a FourCC image");
    }
    const auto flags = u32(p + 8U);
    const auto height = u32(p + 12U);
    const auto width = u32(p + 16U);
    const auto depth = u32(p + 24U);
    const auto caps2 = u32(p + 112U);
    if (width == 0U || height == 0U || width > k_max_dimension || height > k_max_dimension) {
        return failure(Status::invalid_dimensions, "DDS dimensions are zero or above 16384");
    }
    if (depth > 1U || caps2 != 0U) {
        return failure(Status::invalid_header, "Only single 2D DDS images are supported");
    }

    Document d{};
    d.width = width;
    d.height = height;
    d.fourcc = u32(p + 84U);
    d.header_size = static_cast<std::uint32_t>(legacy_header_size);
    FormatInfo info{};
    if (d.fourcc == fourcc('D', 'X', '1', '0')) {
        if (bytes.size() < dx10_header_size) {
            return failure(Status::truncated, "DDS DX10 extended header is truncated");
        }
        d.dx10_header = true;
        d.header_size = static_cast<std::uint32_t>(dx10_header_size);
        d.dxgi_format = u32(p + 128U);
        const auto dimension = u32(p + 132U);
        const auto misc = u32(p + 136U);
        const auto array_size = u32(p + 140U);
        if (dimension != 3U || (misc & 4U) != 0U || array_size != 1U) {
            return failure(Status::invalid_header,
                           "DDS DX10 header is not a single 2D texture");
        }
        info = from_dxgi(d.dxgi_format);
    } else {
        info = from_fourcc(d.fourcc);
    }
    if (!info.ok) {
        return failure(Status::unsupported_format,
                       "DDS format is not a BC1..BC7 block format");
    }
    d.format = info.format;
    d.srgb = info.srgb;
    d.premultiplied = info.premultiplied;

    constexpr std::uint32_t ddsd_mipmapcount = 0x00020000U;
    const auto raw_mips = u32(p + 28U);
    // DirectXTK treats a zero count as one level, with or without the flag.
    d.mip_count = (flags & ddsd_mipmapcount) != 0U ? std::max(raw_mips, 1U) : 1U;
    if (d.mip_count > maximum_mip_count(width, height)) {
        return failure(Status::invalid_mip_count, "DDS mip count exceeds the image pyramid");
    }
    std::uint64_t payload = 0U;
    if (!chain_size(width, height, d.mip_count, d.format, &payload) ||
        payload > std::numeric_limits<std::uint32_t>::max() - d.header_size) {
        return failure(Status::payload_overflow, "DDS mip chain size overflows");
    }
    const auto total = payload + d.header_size;
    if (total > bytes.size()) {
        return failure(Status::payload_out_of_bounds, "DDS mip chain leaves the input span");
    }
    d.payload_size = static_cast<std::uint32_t>(payload);
    d.total_size = static_cast<std::uint32_t>(total);
    return ParseResult{.status = Status::ok, .document = d, .detail = {}};
}

void decode_block(Format format, const std::byte* block, std::uint8_t* rgba64) noexcept {
    switch (format) {
    case Format::bc1:
        decode_color(block, true, rgba64);
        return;
    case Format::bc2: {
        decode_color(block + 8U, false, rgba64);
        const auto alpha = u64(block);
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            rgba64[i * 4U + 3U] = static_cast<std::uint8_t>(((alpha >> (i * 4U)) & 15U) * 17U);
        }
        return;
    }
    case Format::bc3:
        decode_color(block + 8U, false, rgba64);
        decode_unorm_ramp(block, rgba64, 3U);
        return;
    case Format::bc4_unorm:
    case Format::bc4_snorm:
        if (format == Format::bc4_unorm) {
            decode_unorm_ramp(block, rgba64, 0U);
        } else {
            decode_snorm_ramp(block, rgba64, 0U);
        }
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            rgba64[i * 4U + 1U] = rgba64[i * 4U];
            rgba64[i * 4U + 2U] = rgba64[i * 4U];
            rgba64[i * 4U + 3U] = 255U;
        }
        return;
    case Format::bc5_unorm:
    case Format::bc5_snorm:
        if (format == Format::bc5_unorm) {
            decode_unorm_ramp(block, rgba64, 0U);
            decode_unorm_ramp(block + 8U, rgba64, 1U);
        } else {
            decode_snorm_ramp(block, rgba64, 0U);
            decode_snorm_ramp(block + 8U, rgba64, 1U);
        }
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            rgba64[i * 4U + 2U] = 0U;
            rgba64[i * 4U + 3U] = 255U;
        }
        return;
    case Format::bc6h_uf16:
        decode_bc6h(block, rgba64, false);
        return;
    case Format::bc6h_sf16:
        decode_bc6h(block, rgba64, true);
        return;
    case Format::bc7:
        decode_bc7(block, rgba64);
        return;
    }
}

DecodeResult decode_level_rgba8(std::span<const std::byte> bytes, const Document& d,
                                std::uint32_t level, std::uint64_t max_pixels) noexcept {
    if (!d.valid() || d.total_size > bytes.size() || level >= d.mip_count) {
        return {.ok = false, .image = {}, .mip_level = level, .downscale = 1U,
                .detail = "DDS decode received an invalid document or level"};
    }
    const auto width = std::max(1U, d.width >> level);
    const auto height = std::max(1U, d.height >> level);
    const auto pixels = static_cast<std::uint64_t>(width) * height;
    if (pixels > max_pixels || pixels > std::numeric_limits<std::size_t>::max() / 4U) {
        return {.ok = false, .image = {}, .mip_level = level, .downscale = 1U,
                .detail = "DDS level exceeds the bounded output pixel budget"};
    }
    std::uint64_t offset = 0U;
    std::uint64_t size = 0U;
    if (!level_offset(d, level, &offset) || !level_size(width, height, d.format, &size) ||
        offset + size > d.total_size) {
        return {.ok = false, .image = {}, .mip_level = level, .downscale = 1U,
                .detail = "DDS level leaves the image"};
    }
    DecodeResult result{};
    result.mip_level = level;
    result.image.width = width;
    result.image.height = height;
    try {
        result.image.rgba8.assign(static_cast<std::size_t>(pixels * 4U), 0U);
    } catch (...) {
        return {.ok = false, .image = {}, .mip_level = level, .downscale = 1U,
                .detail = "DDS preview allocation failed"};
    }
    const auto* base = bytes.data() + offset;
    const auto blocks_h = (height + 3U) / 4U;
    std::vector<std::uint8_t> strip;
    try {
        strip.assign(static_cast<std::size_t>(width) * 16U, 0U);
    } catch (...) {
        return {.ok = false, .image = {}, .mip_level = level, .downscale = 1U,
                .detail = "DDS preview allocation failed"};
    }
    for (std::uint32_t by = 0U; by < blocks_h; ++by) {
        decode_block_row(base, d, width, by, strip.data());
        for (std::uint32_t r = 0U; r < 4U; ++r) {
            const auto y = by * 4U + r;
            if (y >= height) break;
            std::memcpy(result.image.rgba8.data() + static_cast<std::size_t>(y) * width * 4U,
                        strip.data() + static_cast<std::size_t>(r) * width * 4U,
                        static_cast<std::size_t>(width) * 4U);
        }
    }
    result.ok = result.image.available();
    result.detail = result.ok ? std::string_view{} : std::string_view{"DDS output failed validation"};
    return result;
}

DecodeResult decode_preview_rgba8(std::span<const std::byte> bytes, const Document& d,
                                  std::uint64_t max_pixels) noexcept {
    if (!d.valid() || d.total_size > bytes.size() || max_pixels == 0U) {
        return {.ok = false, .image = {}, .mip_level = 0U, .downscale = 1U,
                .detail = "DDS decode received an invalid parsed document span"};
    }
    for (std::uint32_t level = 0U; level < d.mip_count; ++level) {
        const auto w = std::max(1U, d.width >> level);
        const auto h = std::max(1U, d.height >> level);
        if (static_cast<std::uint64_t>(w) * h <= max_pixels) {
            return decode_level_rgba8(bytes, d, level, max_pixels);
        }
    }

    // No stored level fits: box-filter mip 0 one block row at a time.
    std::uint32_t k = 2U;
    auto out_w = (d.width + k - 1U) / k;
    auto out_h = (d.height + k - 1U) / k;
    while (static_cast<std::uint64_t>(out_w) * out_h > max_pixels && k < 16384U) {
        k *= 2U;
        out_w = (d.width + k - 1U) / k;
        out_h = (d.height + k - 1U) / k;
    }
    DecodeResult result{};
    result.mip_level = 0U;
    result.downscale = k;
    result.image.width = out_w;
    result.image.height = out_h;
    std::vector<std::uint8_t> strip;
    std::vector<std::uint32_t> sum;
    std::vector<std::uint32_t> count;
    try {
        result.image.rgba8.assign(static_cast<std::size_t>(out_w) * out_h * 4U, 0U);
        strip.assign(static_cast<std::size_t>(d.width) * 16U, 0U);
        sum.assign(static_cast<std::size_t>(out_w) * 4U, 0U);
        count.assign(out_w, 0U);
    } catch (...) {
        return {.ok = false, .image = {}, .mip_level = 0U, .downscale = k,
                .detail = "DDS preview allocation failed"};
    }
    auto flush = [&](std::uint32_t oy) {
        auto* row = result.image.rgba8.data() + static_cast<std::size_t>(oy) * out_w * 4U;
        for (std::uint32_t ox = 0U; ox < out_w; ++ox) {
            const auto n = std::max(1U, count[ox]);
            for (std::uint32_t c = 0U; c < 4U; ++c) {
                row[ox * 4U + c] = static_cast<std::uint8_t>((sum[ox * 4U + c] + n / 2U) / n);
            }
        }
        std::fill(sum.begin(), sum.end(), 0U);
        std::fill(count.begin(), count.end(), 0U);
    };
    const auto* base = bytes.data() + d.header_size;
    const auto blocks_h = (d.height + 3U) / 4U;
    std::uint32_t current = 0U;
    for (std::uint32_t by = 0U; by < blocks_h; ++by) {
        decode_block_row(base, d, d.width, by, strip.data());
        for (std::uint32_t r = 0U; r < 4U; ++r) {
            const auto y = by * 4U + r;
            if (y >= d.height) break;
            const auto oy = y / k;
            if (oy != current) {
                flush(current);
                current = oy;
            }
            const auto* src = strip.data() + static_cast<std::size_t>(r) * d.width * 4U;
            for (std::uint32_t x = 0U; x < d.width; ++x) {
                const auto ox = x / k;
                for (std::uint32_t c = 0U; c < 4U; ++c) sum[ox * 4U + c] += src[x * 4U + c];
                ++count[ox];
            }
        }
    }
    flush(current);
    result.ok = result.image.available();
    result.detail = result.ok ? std::string_view{} : std::string_view{"DDS output failed validation"};
    return result;
}

}  // namespace dmc::rengine::codecs::dds_bcn
