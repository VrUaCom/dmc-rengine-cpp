#include "dmc_rengine/codecs/dds_bcn_encode.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>

namespace dmc::rengine::codecs::dds_bcn {
namespace {

using Px = std::array<float, 4>;

[[nodiscard]] constexpr std::uint32_t fourcc(char a, char b, char c, char d) noexcept {
    return static_cast<std::uint32_t>(static_cast<std::uint8_t>(a)) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(b)) << 8U) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(c)) << 16U) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(d)) << 24U);
}

void put_u16(std::byte* p, std::uint32_t v) noexcept {
    p[0] = static_cast<std::byte>(v & 0xFFU);
    p[1] = static_cast<std::byte>((v >> 8U) & 0xFFU);
}

void put_u32(std::byte* p, std::uint32_t v) noexcept {
    for (std::size_t i = 0U; i < 4U; ++i) p[i] = static_cast<std::byte>((v >> (8U * i)) & 0xFFU);
}

// 128-bit little-endian block writer (bit 0 = LSB of byte 0).
struct BitWriter final {
    std::array<std::uint8_t, 16U> bytes{};
    std::uint32_t pos{};

    void put(std::uint32_t value, std::uint32_t count) noexcept {
        for (std::uint32_t i = 0U; i < count; ++i, ++pos) {
            if (((value >> i) & 1U) != 0U) bytes[pos >> 3U] |= static_cast<std::uint8_t>(1U << (pos & 7U));
        }
    }
};

// Principal axis of `n` points in `dims` dimensions (power iteration).
template <std::size_t D>
void principal_axis(const std::array<std::array<float, D>, 16>& pts, std::size_t n,
                    std::array<float, D>* mean, std::array<float, D>* axis) noexcept {
    mean->fill(0.0F);
    for (std::size_t i = 0U; i < n; ++i) {
        for (std::size_t c = 0U; c < D; ++c) (*mean)[c] += pts[i][c];
    }
    for (auto& m : *mean) m /= static_cast<float>(std::max<std::size_t>(n, 1U));
    std::array<std::array<float, D>, D> cov{};
    for (std::size_t i = 0U; i < n; ++i) {
        for (std::size_t a = 0U; a < D; ++a) {
            for (std::size_t b = 0U; b < D; ++b) {
                cov[a][b] += (pts[i][a] - (*mean)[a]) * (pts[i][b] - (*mean)[b]);
            }
        }
    }
    // Start from the widest channel so flat directions do not stall.
    axis->fill(0.0F);
    std::size_t widest = 0U;
    for (std::size_t c = 1U; c < D; ++c) {
        if (cov[c][c] > cov[widest][widest]) widest = c;
    }
    (*axis)[widest] = 1.0F;
    for (int iter = 0; iter < 8; ++iter) {
        std::array<float, D> next{};
        for (std::size_t a = 0U; a < D; ++a) {
            for (std::size_t b = 0U; b < D; ++b) next[a] += cov[a][b] * (*axis)[b];
        }
        float len = 0.0F;
        for (const auto v : next) len += v * v;
        if (len < 1e-12F) break;
        len = std::sqrt(len);
        for (std::size_t c = 0U; c < D; ++c) (*axis)[c] = next[c] / len;
    }
}

// ---- BC1 / BC2 / BC3 colour half ----

[[nodiscard]] std::uint32_t quant565(float r, float g, float b) noexcept {
    const auto q = [](float v, float scale) {
        return static_cast<std::uint32_t>(std::clamp(std::lround(v * scale / 255.0F), 0L,
                                                     static_cast<long>(scale)));
    };
    return (q(r, 31.0F) << 11U) | (q(g, 63.0F) << 5U) | q(b, 31.0F);
}

[[nodiscard]] std::array<std::int32_t, 3> expand565(std::uint32_t v) noexcept {
    const auto r5 = (v >> 11U) & 31U;
    const auto g6 = (v >> 5U) & 63U;
    const auto b5 = v & 31U;
    return {static_cast<std::int32_t>((r5 * 255U + 15U) / 31U),
            static_cast<std::int32_t>((g6 * 255U + 31U) / 63U),
            static_cast<std::int32_t>((b5 * 255U + 15U) / 31U)};
}

struct ColorBlock final {
    std::uint32_t c0{};
    std::uint32_t c1{};
    std::uint32_t indices{};
    std::uint64_t error{std::numeric_limits<std::uint64_t>::max()};
};

// Palette exactly as dds_bcn decodes it.
void color_palette(std::uint32_t c0, std::uint32_t c1, bool four,
                   std::array<std::array<std::int32_t, 3>, 4>* pal) noexcept {
    const auto a = expand565(c0);
    const auto b = expand565(c1);
    (*pal)[0] = a;
    (*pal)[1] = b;
    for (std::size_t c = 0U; c < 3U; ++c) {
        if (four) {
            (*pal)[2][c] = (2 * a[c] + b[c]) / 3;
            (*pal)[3][c] = (a[c] + 2 * b[c]) / 3;
        } else {
            (*pal)[2][c] = (a[c] + b[c]) / 2;
            (*pal)[3][c] = 0;
        }
    }
}

// Assigns indices for fixed endpoints. `transparent` marks pixels that must
// use index 3 of a three-colour block.
ColorBlock assign_color(const std::uint8_t* px, std::uint32_t c0, std::uint32_t c1, bool four,
                        std::uint16_t transparent) noexcept {
    std::array<std::array<std::int32_t, 3>, 4> pal{};
    color_palette(c0, c1, four, &pal);
    ColorBlock out{c0, c1, 0U, 0U};
    const std::uint32_t choices = four ? 4U : 3U;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        if (((transparent >> i) & 1U) != 0U) {
            out.indices |= 3U << (i * 2U);
            continue;
        }
        std::uint64_t best = std::numeric_limits<std::uint64_t>::max();
        std::uint32_t best_index = 0U;
        for (std::uint32_t k = 0U; k < choices; ++k) {
            std::uint64_t e = 0U;
            for (std::size_t c = 0U; c < 3U; ++c) {
                const auto d = static_cast<std::int64_t>(px[i * 4U + c]) - pal[k][c];
                e += static_cast<std::uint64_t>(d * d);
            }
            if (e < best) {
                best = e;
                best_index = k;
            }
        }
        out.indices |= best_index << (i * 2U);
        out.error += best;
    }
    return out;
}

// Least-squares endpoints for the current indices.
bool refit_color(const std::uint8_t* px, const ColorBlock& block, bool four, std::uint16_t transparent,
                 std::array<float, 3>* e0, std::array<float, 3>* e1) noexcept {
    // Weight of endpoint 1 per index.
    const std::array<float, 4> w4{0.0F, 1.0F, 1.0F / 3.0F, 2.0F / 3.0F};
    const std::array<float, 4> w3{0.0F, 1.0F, 0.5F, 0.0F};
    float aa = 0.0F, ab = 0.0F, bb = 0.0F;
    std::array<float, 3> ax{}, bx{};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        if (((transparent >> i) & 1U) != 0U) continue;
        const auto idx = (block.indices >> (i * 2U)) & 3U;
        const float t = four ? w4[idx] : w3[idx];
        const float s = 1.0F - t;
        aa += s * s;
        ab += s * t;
        bb += t * t;
        for (std::size_t c = 0U; c < 3U; ++c) {
            ax[c] += s * px[i * 4U + c];
            bx[c] += t * px[i * 4U + c];
        }
    }
    const float det = aa * bb - ab * ab;
    if (std::fabs(det) < 1e-6F) return false;
    for (std::size_t c = 0U; c < 3U; ++c) {
        (*e0)[c] = std::clamp((ax[c] * bb - bx[c] * ab) / det, 0.0F, 255.0F);
        (*e1)[c] = std::clamp((bx[c] * aa - ax[c] * ab) / det, 0.0F, 255.0F);
    }
    return true;
}

// Orders endpoints for the block mode and remaps indices to match.
ColorBlock order_color(ColorBlock b, bool four) noexcept {
    const bool want_greater = four;
    if (b.c0 == b.c1) {
        if (four) {
            // c0 == c1 would read as three-colour mode; every index -> 0 is exact.
            b.indices = 0U;
        }
        return b;
    }
    if ((b.c0 > b.c1) != want_greater) {
        std::swap(b.c0, b.c1);
        std::uint32_t remapped = 0U;
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            auto idx = (b.indices >> (i * 2U)) & 3U;
            if (four) {
                idx = idx < 2U ? idx ^ 1U : idx ^ 1U;  // 0<->1, 2<->3
            } else if (idx < 2U) {
                idx ^= 1U;  // 0<->1, 2 (mid) and 3 (transparent) stay
            }
            remapped |= idx << (i * 2U);
        }
        b.indices = remapped;
    }
    return b;
}

// four: four-colour mode (BC2/BC3 always; BC1 when opaque).
ColorBlock fit_color(const std::uint8_t* px, bool four, std::uint16_t transparent) noexcept {
    std::array<std::array<float, 3>, 16> pts{};
    std::size_t n = 0U;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        if (((transparent >> i) & 1U) != 0U) continue;
        pts[n++] = {static_cast<float>(px[i * 4U]), static_cast<float>(px[i * 4U + 1U]),
                    static_cast<float>(px[i * 4U + 2U])};
    }
    if (n == 0U) {
        ColorBlock all{0U, 0U, 0xFFFFFFFFU, 0U};
        return all;
    }
    std::array<float, 3> mean{}, axis{};
    principal_axis<3>(pts, n, &mean, &axis);
    float lo = std::numeric_limits<float>::max(), hi = -lo;
    for (std::size_t i = 0U; i < n; ++i) {
        float t = 0.0F;
        for (std::size_t c = 0U; c < 3U; ++c) t += (pts[i][c] - mean[c]) * axis[c];
        lo = std::min(lo, t);
        hi = std::max(hi, t);
    }
    std::array<float, 3> e0{}, e1{};
    for (std::size_t c = 0U; c < 3U; ++c) {
        e0[c] = std::clamp(mean[c] + axis[c] * hi, 0.0F, 255.0F);
        e1[c] = std::clamp(mean[c] + axis[c] * lo, 0.0F, 255.0F);
    }
    ColorBlock best{};
    for (int iter = 0; iter < 3; ++iter) {
        const auto q0 = quant565(e0[0], e0[1], e0[2]);
        const auto q1 = quant565(e1[0], e1[1], e1[2]);
        const auto trial = assign_color(px, q0, q1, four, transparent);
        if (trial.error < best.error) best = trial;
        if (trial.error == 0U || !refit_color(px, trial, four, transparent, &e0, &e1)) break;
    }
    // Single-colour blocks: rounding both endpoints the same way can miss the
    // colour by up to half a step; also try the neighbouring 5:6:5 codes.
    if (best.error != 0U) {
        const auto base = best.c0;
        for (std::uint32_t dr = 0U; dr < 2U; ++dr) {
            for (std::uint32_t dg = 0U; dg < 2U; ++dg) {
                for (std::uint32_t db = 0U; db < 2U; ++db) {
                    const auto r = std::min(((base >> 11U) & 31U) + dr, 31U);
                    const auto g = std::min(((base >> 5U) & 63U) + dg, 63U);
                    const auto bl = std::min((base & 31U) + db, 31U);
                    const auto alt = (r << 11U) | (g << 5U) | bl;
                    const auto trial = assign_color(px, alt, best.c1, four, transparent);
                    if (trial.error < best.error) best = trial;
                }
            }
        }
    }
    return order_color(best, four);
}

void write_color(const ColorBlock& b, std::byte* out) noexcept {
    put_u16(out, b.c0);
    put_u16(out + 2U, b.c1);
    put_u32(out + 4U, b.indices);
}

// ---- interpolated 8-value ramps (BC3 alpha, BC4, BC5) ----

struct Ramp final {
    std::int32_t r0{};
    std::int32_t r1{};
    std::uint64_t indices{};
    std::uint64_t error{std::numeric_limits<std::uint64_t>::max()};
};

// Palette as dds_bcn decodes it. Signed ramps use the float SNORM path; the
// encoder works on the same integer grid (values -127..127).
void ramp_palette(std::int32_t r0, std::int32_t r1, bool is_signed, std::array<float, 8>* v) noexcept {
    (*v)[0] = static_cast<float>(r0);
    (*v)[1] = static_cast<float>(r1);
    if (r0 > r1) {
        for (int i = 1; i <= 6; ++i) {
            const auto num = (7 - i) * r0 + i * r1;
            (*v)[static_cast<std::size_t>(i + 1)] = is_signed ? static_cast<float>(num) / 7.0F
                                                              : static_cast<float>(num / 7);
        }
    } else {
        for (int i = 1; i <= 4; ++i) {
            const auto num = (5 - i) * r0 + i * r1;
            (*v)[static_cast<std::size_t>(i + 1)] = is_signed ? static_cast<float>(num) / 5.0F
                                                              : static_cast<float>(num / 5);
        }
        (*v)[6] = is_signed ? -127.0F : 0.0F;
        (*v)[7] = is_signed ? 127.0F : 255.0F;
    }
}

Ramp assign_ramp(const std::array<std::int32_t, 16>& values, std::int32_t r0, std::int32_t r1,
                 bool is_signed) noexcept {
    std::array<float, 8> pal{};
    ramp_palette(r0, r1, is_signed, &pal);
    Ramp out{r0, r1, 0U, 0U};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        float best = std::numeric_limits<float>::max();
        std::uint64_t best_index = 0U;
        for (std::uint32_t k = 0U; k < 8U; ++k) {
            const float d = static_cast<float>(values[i]) - pal[k];
            if (d * d < best) {
                best = d * d;
                best_index = k;
            }
        }
        out.indices |= best_index << (i * 3U);
        out.error += static_cast<std::uint64_t>(std::lround(best * 16.0F));
    }
    return out;
}

Ramp fit_ramp(const std::array<std::int32_t, 16>& values, bool is_signed) noexcept {
    const std::int32_t floor = is_signed ? -127 : 0;
    const std::int32_t ceil = is_signed ? 127 : 255;
    const auto [mn_it, mx_it] = std::minmax_element(values.begin(), values.end());
    const auto mn = *mn_it, mx = *mx_it;
    Ramp best{};
    // Eight-value mode (r0 > r1): small insets around the range.
    for (std::int32_t a = 0; a <= 3; ++a) {
        for (std::int32_t b = 0; b <= 3; ++b) {
            const auto hi = std::clamp(mx - a, floor, ceil);
            const auto lo = std::clamp(mn + b, floor, ceil);
            if (hi <= lo) continue;
            const auto trial = assign_ramp(values, hi, lo, is_signed);
            if (trial.error < best.error) best = trial;
        }
    }
    // Six-value mode (r0 <= r1) keeps exact extremes in indices 6 / 7.
    std::int32_t inner_lo = ceil, inner_hi = floor;
    for (const auto v : values) {
        if (v == floor || v == ceil) continue;
        inner_lo = std::min(inner_lo, v);
        inner_hi = std::max(inner_hi, v);
    }
    if (inner_lo > inner_hi) inner_lo = inner_hi = mn;
    const auto trial6 = assign_ramp(values, inner_lo, inner_hi, is_signed);
    if (trial6.error < best.error) best = trial6;
    if (mx == mn) {
        const auto flat = assign_ramp(values, mx, mn, is_signed);
        if (flat.error <= best.error) best = flat;
    }
    return best;
}

void write_ramp(const Ramp& r, std::byte* out) noexcept {
    out[0] = static_cast<std::byte>(static_cast<std::uint8_t>(r.r0));
    out[1] = static_cast<std::byte>(static_cast<std::uint8_t>(r.r1));
    for (std::size_t i = 0U; i < 6U; ++i) {
        out[2U + i] = static_cast<std::byte>((r.indices >> (8U * i)) & 0xFFU);
    }
}

[[nodiscard]] std::int32_t to_snorm(std::uint8_t v) noexcept {
    return static_cast<std::int32_t>(std::lround(static_cast<float>(v) * 254.0F / 255.0F)) - 127;
}

[[nodiscard]] std::uint8_t luminance(const std::uint8_t* p) noexcept {
    return static_cast<std::uint8_t>((p[0] * 77U + p[1] * 150U + p[2] * 29U + 128U) >> 8U);
}

// ---- BC7 ----

constexpr std::array<std::uint32_t, 4U> k_w2{0, 21, 43, 64};
constexpr std::array<std::uint32_t, 16U> k_w4{0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};

[[nodiscard]] std::uint32_t lerp64(std::uint32_t a, std::uint32_t b, std::uint32_t w) noexcept {
    return ((64U - w) * a + w * b + 32U) >> 6U;
}

struct Bc7Candidate final {
    std::array<std::uint8_t, 16U> block{};
    std::uint64_t error{std::numeric_limits<std::uint64_t>::max()};
};

// Mode 6: one RGBA line, 7-bit endpoints + p-bit each, 4-bit indices.
Bc7Candidate bc7_mode6(const std::uint8_t* px) noexcept {
    std::array<std::array<float, 4>, 16> pts{};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        for (std::size_t c = 0U; c < 4U; ++c) pts[i][c] = px[i * 4U + c];
    }
    std::array<float, 4> mean{}, axis{};
    principal_axis<4>(pts, 16U, &mean, &axis);
    float lo = std::numeric_limits<float>::max(), hi = -lo;
    for (const auto& p : pts) {
        float t = 0.0F;
        for (std::size_t c = 0U; c < 4U; ++c) t += (p[c] - mean[c]) * axis[c];
        lo = std::min(lo, t);
        hi = std::max(hi, t);
    }
    std::array<float, 4> f0{}, f1{};
    for (std::size_t c = 0U; c < 4U; ++c) {
        f0[c] = std::clamp(mean[c] + axis[c] * lo, 0.0F, 255.0F);
        f1[c] = std::clamp(mean[c] + axis[c] * hi, 0.0F, 255.0F);
    }

    Bc7Candidate best{};
    std::array<std::uint32_t, 4> best_e0{}, best_e1{};
    std::array<std::uint32_t, 16> best_idx{};
    std::uint32_t best_p0 = 0U, best_p1 = 0U;
    for (int iter = 0; iter < 3; ++iter) {
        std::array<std::uint32_t, 16> iter_idx{};
        std::uint64_t iter_err = std::numeric_limits<std::uint64_t>::max();
        for (std::uint32_t p0 = 0U; p0 < 2U; ++p0) {
            for (std::uint32_t p1 = 0U; p1 < 2U; ++p1) {
                std::array<std::uint32_t, 4> e0{}, e1{};
                for (std::size_t c = 0U; c < 4U; ++c) {
                    const auto q0 = std::clamp(std::lround((f0[c] - static_cast<float>(p0)) / 2.0F), 0L, 127L);
                    const auto q1 = std::clamp(std::lround((f1[c] - static_cast<float>(p1)) / 2.0F), 0L, 127L);
                    e0[c] = (static_cast<std::uint32_t>(q0) << 1U) | p0;
                    e1[c] = (static_cast<std::uint32_t>(q1) << 1U) | p1;
                }
                std::array<std::array<std::uint32_t, 4>, 16> pal{};
                for (std::uint32_t k = 0U; k < 16U; ++k) {
                    for (std::size_t c = 0U; c < 4U; ++c) pal[k][c] = lerp64(e0[c], e1[c], k_w4[k]);
                }
                std::uint64_t err = 0U;
                std::array<std::uint32_t, 16> idx{};
                for (std::uint32_t i = 0U; i < 16U; ++i) {
                    std::uint64_t pe = std::numeric_limits<std::uint64_t>::max();
                    for (std::uint32_t k = 0U; k < 16U; ++k) {
                        std::uint64_t e = 0U;
                        for (std::size_t c = 0U; c < 4U; ++c) {
                            const auto d = static_cast<std::int64_t>(px[i * 4U + c]) - pal[k][c];
                            e += static_cast<std::uint64_t>(d * d);
                        }
                        if (e < pe) {
                            pe = e;
                            idx[i] = k;
                        }
                    }
                    err += pe;
                }
                if (err < iter_err) {
                    iter_err = err;
                    iter_idx = idx;
                }
                if (err < best.error) {
                    best.error = err;
                    best_e0 = e0;
                    best_e1 = e1;
                    best_idx = idx;
                    best_p0 = p0;
                    best_p1 = p1;
                }
            }
        }
        if (best.error == 0U) break;
        // Least squares on the best indices of this pass.
        float aa = 0.0F, ab = 0.0F, bb = 0.0F;
        std::array<float, 4> ax{}, bx{};
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            const float t = static_cast<float>(k_w4[iter_idx[i]]) / 64.0F;
            const float s = 1.0F - t;
            aa += s * s;
            ab += s * t;
            bb += t * t;
            for (std::size_t c = 0U; c < 4U; ++c) {
                ax[c] += s * pts[i][c];
                bx[c] += t * pts[i][c];
            }
        }
        const float det = aa * bb - ab * ab;
        if (std::fabs(det) < 1e-6F) break;
        for (std::size_t c = 0U; c < 4U; ++c) {
            f0[c] = std::clamp((ax[c] * bb - bx[c] * ab) / det, 0.0F, 255.0F);
            f1[c] = std::clamp((bx[c] * aa - ax[c] * ab) / det, 0.0F, 255.0F);
        }
    }
    // Anchor: pixel 0 index has 3 bits (MSB implied 0).
    if (best_idx[0] >= 8U) {
        std::swap(best_e0, best_e1);
        std::swap(best_p0, best_p1);
        for (auto& v : best_idx) v = 15U - v;
    }
    BitWriter w;
    w.put(1U << 6U, 7U);
    for (std::size_t c = 0U; c < 4U; ++c) {
        w.put(best_e0[c] >> 1U, 7U);
        w.put(best_e1[c] >> 1U, 7U);
    }
    w.put(best_p0, 1U);
    w.put(best_p1, 1U);
    for (std::uint32_t i = 0U; i < 16U; ++i) w.put(best_idx[i], i == 0U ? 3U : 4U);
    best.block = w.bytes;
    return best;
}

// Mode 5: colour line (7-bit endpoints, 2-bit indices) and a separate alpha
// ramp (8-bit endpoints, 2-bit indices); `rotation` swaps a colour channel
// with alpha before encoding (the decoder swaps it back).
Bc7Candidate bc7_mode5(const std::uint8_t* src, std::uint32_t rotation) noexcept {
    std::array<std::uint8_t, 64> px{};
    std::memcpy(px.data(), src, 64U);
    if (rotation != 0U) {
        for (std::uint32_t i = 0U; i < 16U; ++i) std::swap(px[i * 4U + rotation - 1U], px[i * 4U + 3U]);
    }
    // Colour line.
    std::array<std::array<float, 3>, 16> pts{};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        for (std::size_t c = 0U; c < 3U; ++c) pts[i][c] = px[i * 4U + c];
    }
    std::array<float, 3> mean{}, axis{};
    principal_axis<3>(pts, 16U, &mean, &axis);
    float lo = std::numeric_limits<float>::max(), hi = -lo;
    for (const auto& p : pts) {
        float t = 0.0F;
        for (std::size_t c = 0U; c < 3U; ++c) t += (p[c] - mean[c]) * axis[c];
        lo = std::min(lo, t);
        hi = std::max(hi, t);
    }
    std::array<float, 3> f0{}, f1{};
    for (std::size_t c = 0U; c < 3U; ++c) {
        f0[c] = std::clamp(mean[c] + axis[c] * lo, 0.0F, 255.0F);
        f1[c] = std::clamp(mean[c] + axis[c] * hi, 0.0F, 255.0F);
    }
    auto expand7 = [](std::uint32_t q) { return (q << 1U) | (q >> 6U); };
    std::uint64_t color_err = std::numeric_limits<std::uint64_t>::max();
    std::array<std::uint32_t, 3> ce0{}, ce1{};
    std::array<std::uint32_t, 16> cidx{};
    for (int iter = 0; iter < 3; ++iter) {
        std::array<std::uint32_t, 3> q0{}, q1{};
        for (std::size_t c = 0U; c < 3U; ++c) {
            q0[c] = static_cast<std::uint32_t>(std::clamp(std::lround(f0[c] * 127.0F / 255.0F), 0L, 127L));
            q1[c] = static_cast<std::uint32_t>(std::clamp(std::lround(f1[c] * 127.0F / 255.0F), 0L, 127L));
        }
        std::array<std::array<std::uint32_t, 3>, 4> pal{};
        for (std::uint32_t k = 0U; k < 4U; ++k) {
            for (std::size_t c = 0U; c < 3U; ++c) pal[k][c] = lerp64(expand7(q0[c]), expand7(q1[c]), k_w2[k]);
        }
        std::uint64_t err = 0U;
        std::array<std::uint32_t, 16> idx{};
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            std::uint64_t pe = std::numeric_limits<std::uint64_t>::max();
            for (std::uint32_t k = 0U; k < 4U; ++k) {
                std::uint64_t e = 0U;
                for (std::size_t c = 0U; c < 3U; ++c) {
                    const auto d = static_cast<std::int64_t>(px[i * 4U + c]) - pal[k][c];
                    e += static_cast<std::uint64_t>(d * d);
                }
                if (e < pe) {
                    pe = e;
                    idx[i] = k;
                }
            }
            err += pe;
        }
        if (err < color_err) {
            color_err = err;
            ce0 = q0;
            ce1 = q1;
            cidx = idx;
        }
        if (err == 0U) break;
        float aa = 0.0F, ab = 0.0F, bb = 0.0F;
        std::array<float, 3> ax{}, bx{};
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            const float t = static_cast<float>(k_w2[idx[i]]) / 64.0F;
            const float s = 1.0F - t;
            aa += s * s;
            ab += s * t;
            bb += t * t;
            for (std::size_t c = 0U; c < 3U; ++c) {
                ax[c] += s * pts[i][c];
                bx[c] += t * pts[i][c];
            }
        }
        const float det = aa * bb - ab * ab;
        if (std::fabs(det) < 1e-6F) break;
        for (std::size_t c = 0U; c < 3U; ++c) {
            f0[c] = std::clamp((ax[c] * bb - bx[c] * ab) / det, 0.0F, 255.0F);
            f1[c] = std::clamp((bx[c] * aa - ax[c] * ab) / det, 0.0F, 255.0F);
        }
    }
    // Alpha ramp: four levels between two 8-bit endpoints.
    std::uint32_t amin = 255U, amax = 0U;
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        amin = std::min<std::uint32_t>(amin, px[i * 4U + 3U]);
        amax = std::max<std::uint32_t>(amax, px[i * 4U + 3U]);
    }
    std::uint64_t alpha_err = std::numeric_limits<std::uint64_t>::max();
    std::uint32_t a0 = amin, a1 = amax;
    std::array<std::uint32_t, 16> aidx{};
    for (std::uint32_t inset_lo = 0U; inset_lo <= 4U; ++inset_lo) {
        for (std::uint32_t inset_hi = 0U; inset_hi <= 4U; ++inset_hi) {
            const auto lo8 = std::min(amin + inset_lo, 255U);
            const auto hi8 = amax >= inset_hi ? amax - inset_hi : 0U;
            if (lo8 > hi8 && !(inset_lo == 0U && inset_hi == 0U)) continue;
            std::uint64_t err = 0U;
            std::array<std::uint32_t, 16> idx{};
            for (std::uint32_t i = 0U; i < 16U; ++i) {
                std::uint64_t pe = std::numeric_limits<std::uint64_t>::max();
                for (std::uint32_t k = 0U; k < 4U; ++k) {
                    const auto d = static_cast<std::int64_t>(px[i * 4U + 3U]) - lerp64(lo8, hi8, k_w2[k]);
                    if (static_cast<std::uint64_t>(d * d) < pe) {
                        pe = static_cast<std::uint64_t>(d * d);
                        idx[i] = k;
                    }
                }
                err += pe;
            }
            if (err < alpha_err) {
                alpha_err = err;
                a0 = lo8;
                a1 = hi8;
                aidx = idx;
            }
        }
    }
    if (cidx[0] >= 2U) {
        std::swap(ce0, ce1);
        for (auto& v : cidx) v = 3U - v;
    }
    if (aidx[0] >= 2U) {
        std::swap(a0, a1);
        for (auto& v : aidx) v = 3U - v;
    }
    Bc7Candidate out{};
    out.error = color_err + alpha_err;
    BitWriter w;
    w.put(1U << 5U, 6U);
    w.put(rotation, 2U);
    for (std::size_t c = 0U; c < 3U; ++c) {
        w.put(ce0[c], 7U);
        w.put(ce1[c], 7U);
    }
    w.put(a0, 8U);
    w.put(a1, 8U);
    for (std::uint32_t i = 0U; i < 16U; ++i) w.put(cidx[i], i == 0U ? 1U : 2U);
    for (std::uint32_t i = 0U; i < 16U; ++i) w.put(aidx[i], i == 0U ? 1U : 2U);
    out.block = w.bytes;
    return out;
}

void encode_bc7(const std::uint8_t* px, std::byte* out) noexcept {
    auto best = bc7_mode6(px);
    bool varying_alpha = false;
    for (std::uint32_t i = 1U; i < 16U; ++i) varying_alpha = varying_alpha || px[i * 4U + 3U] != px[3];
    // Mode 5 separates alpha (rotation 0) or one colour channel; it helps
    // blocks whose alpha (or a channel) does not follow the colour line.
    for (std::uint32_t rotation = 0U; rotation < 4U && best.error != 0U; ++rotation) {
        if (rotation == 0U && !varying_alpha) continue;
        const auto trial = bc7_mode5(px, rotation);
        if (trial.error < best.error) best = trial;
    }
    std::memcpy(out, best.block.data(), 16U);
}

// ---- BC6H (mode 11: one region, 10-bit endpoints, 4-bit indices) ----

[[nodiscard]] std::uint16_t float_to_half(float f) noexcept {
    const auto bits = std::bit_cast<std::uint32_t>(f);
    const auto sign = (bits >> 16U) & 0x8000U;
    const auto exp = static_cast<std::int32_t>((bits >> 23U) & 0xFFU) - 127 + 15;
    std::uint32_t mant = bits & 0x7FFFFFU;
    if (exp <= 0) {
        if (exp < -10) return static_cast<std::uint16_t>(sign);
        mant |= 0x800000U;
        const auto shift = static_cast<std::uint32_t>(14 - exp);
        return static_cast<std::uint16_t>(sign | ((mant + (1U << (shift - 1U))) >> shift));
    }
    if (exp >= 31) return static_cast<std::uint16_t>(sign | 0x7BFFU);
    auto h = sign | (static_cast<std::uint32_t>(exp) << 10U) | ((mant + 0x1000U) >> 13U);
    return static_cast<std::uint16_t>(std::min<std::uint32_t>(h, sign | 0x7BFFU));
}

[[nodiscard]] float half_to_float(std::uint16_t h) noexcept {
    const std::uint32_t sign = static_cast<std::uint32_t>(h & 0x8000U) << 16U;
    const std::uint32_t exponent = (h >> 10U) & 0x1FU;
    std::uint32_t mantissa = h & 0x3FFU;
    std::uint32_t bits = 0U;
    if (exponent == 0U) {
        if (mantissa == 0U) {
            bits = sign;
        } else {
            std::uint32_t e = 113U;
            while ((mantissa & 0x400U) == 0U) {
                mantissa <<= 1U;
                --e;
            }
            bits = sign | (e << 23U) | ((mantissa & 0x3FFU) << 13U);
        }
    } else {
        bits = sign | ((exponent + 112U) << 23U) | (mantissa << 13U);
    }
    return std::bit_cast<float>(bits);
}

// Unquantized endpoint for a 10-bit code, as the decoder computes it.
[[nodiscard]] std::int32_t bc6_unquantize(std::int32_t q, bool is_signed) noexcept {
    if (!is_signed) {
        if (q == 0) return 0;
        if (q == 1023) return 0xFFFF;
        return ((q << 15) + 0x4000) >> 9;
    }
    if (q == 0) return 0;
    if (q >= 511) return 0x7FFF;
    return ((q << 15) + 0x4000) >> 9;
}

[[nodiscard]] float bc6_value(std::int32_t v, bool is_signed) noexcept {
    const auto half = is_signed ? static_cast<std::uint16_t>((v * 31) >> 5)
                                : static_cast<std::uint16_t>((v * 31) >> 6);
    return half_to_float(half);
}

void encode_bc6h(const std::uint8_t* px, bool is_signed, std::byte* out) noexcept {
    // Target values in the decoder's interpolation domain.
    std::array<std::array<float, 3>, 16> pts{};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        for (std::size_t c = 0U; c < 3U; ++c) pts[i][c] = static_cast<float>(px[i * 4U + c]) / 255.0F;
    }
    const auto to_code = [&](float f) {
        const float h = static_cast<float>(float_to_half(std::max(f, 0.0F)));
        const float v = is_signed ? h * 32.0F / 31.0F : h * 64.0F / 31.0F;
        const auto maxq = is_signed ? 510L : 1023L;
        return static_cast<std::int32_t>(std::clamp(std::lround((v - 32.0F) / 64.0F), 0L, maxq));
    };
    std::array<float, 3> mean{}, axis{};
    principal_axis<3>(pts, 16U, &mean, &axis);
    float lo = std::numeric_limits<float>::max(), hi = -lo;
    for (const auto& p : pts) {
        float t = 0.0F;
        for (std::size_t c = 0U; c < 3U; ++c) t += (p[c] - mean[c]) * axis[c];
        lo = std::min(lo, t);
        hi = std::max(hi, t);
    }
    std::array<std::int32_t, 3> q0{}, q1{};
    for (std::size_t c = 0U; c < 3U; ++c) {
        q0[c] = to_code(std::clamp(mean[c] + axis[c] * lo, 0.0F, 1.0F));
        q1[c] = to_code(std::clamp(mean[c] + axis[c] * hi, 0.0F, 1.0F));
    }
    std::array<std::uint32_t, 16> idx{};
    for (std::uint32_t i = 0U; i < 16U; ++i) {
        float best = std::numeric_limits<float>::max();
        for (std::uint32_t k = 0U; k < 16U; ++k) {
            float e = 0.0F;
            for (std::size_t c = 0U; c < 3U; ++c) {
                const auto u0 = bc6_unquantize(q0[c], is_signed);
                const auto u1 = bc6_unquantize(q1[c], is_signed);
                const auto w = static_cast<std::int32_t>(k_w4[k]);
                const float d = bc6_value((u0 * (64 - w) + u1 * w + 32) >> 6, is_signed) - pts[i][c];
                e += d * d;
            }
            if (e < best) {
                best = e;
                idx[i] = k;
            }
        }
    }
    if (idx[0] >= 8U) {
        std::swap(q0, q1);
        for (auto& v : idx) v = 15U - v;
    }
    BitWriter w;
    w.put(0b00011U, 5U);  // mode 11
    for (const auto* e : {&q0, &q1}) {
        for (std::size_t c = 0U; c < 3U; ++c) w.put(static_cast<std::uint32_t>((*e)[c]) & 0x3FFU, 10U);
    }
    for (std::uint32_t i = 0U; i < 16U; ++i) w.put(idx[i], i == 0U ? 3U : 4U);
    std::memcpy(out, w.bytes.data(), 16U);
}

constexpr std::array<Format, 10U> k_writable{
    Format::bc1, Format::bc2, Format::bc3, Format::bc4_unorm, Format::bc4_snorm,
    Format::bc5_unorm, Format::bc5_snorm, Format::bc6h_uf16, Format::bc6h_sf16, Format::bc7,
};

}  // namespace

void encode_block(Format format, const std::uint8_t* px, std::byte* out) noexcept {
    switch (format) {
    case Format::bc1: {
        std::uint16_t transparent = 0U;
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            if (px[i * 4U + 3U] < 128U) transparent = static_cast<std::uint16_t>(transparent | (1U << i));
        }
        write_color(fit_color(px, transparent == 0U, transparent), out);
        return;
    }
    case Format::bc2: {
        std::uint64_t alpha = 0U;
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            alpha |= static_cast<std::uint64_t>((px[i * 4U + 3U] * 15U + 127U) / 255U) << (i * 4U);
        }
        for (std::size_t i = 0U; i < 8U; ++i) out[i] = static_cast<std::byte>((alpha >> (8U * i)) & 0xFFU);
        write_color(fit_color(px, true, 0U), out + 8U);
        return;
    }
    case Format::bc3: {
        std::array<std::int32_t, 16> a{};
        for (std::uint32_t i = 0U; i < 16U; ++i) a[i] = px[i * 4U + 3U];
        write_ramp(fit_ramp(a, false), out);
        write_color(fit_color(px, true, 0U), out + 8U);
        return;
    }
    case Format::bc4_unorm:
    case Format::bc4_snorm: {
        const bool s = format == Format::bc4_snorm;
        std::array<std::int32_t, 16> v{};
        for (std::uint32_t i = 0U; i < 16U; ++i) {
            const auto l = luminance(px + i * 4U);
            v[i] = s ? to_snorm(l) : l;
        }
        write_ramp(fit_ramp(v, s), out);
        return;
    }
    case Format::bc5_unorm:
    case Format::bc5_snorm: {
        const bool s = format == Format::bc5_snorm;
        for (std::size_t ch = 0U; ch < 2U; ++ch) {
            std::array<std::int32_t, 16> v{};
            for (std::uint32_t i = 0U; i < 16U; ++i) {
                const auto b = px[i * 4U + ch];
                v[i] = s ? to_snorm(b) : b;
            }
            write_ramp(fit_ramp(v, s), out + ch * 8U);
        }
        return;
    }
    case Format::bc6h_uf16:
        encode_bc6h(px, false, out);
        return;
    case Format::bc6h_sf16:
        encode_bc6h(px, true, out);
        return;
    case Format::bc7:
        encode_bc7(px, out);
        return;
    }
}

bool encode_level(Format format, const RgbaImage& image, std::vector<std::byte>* out) {
    if (out == nullptr || !image.available()) return false;
    const auto bw = (image.width + 3U) / 4U;
    const auto bh = (image.height + 3U) / 4U;
    const auto bsize = block_bytes(format);
    try {
        out->assign(static_cast<std::size_t>(bw) * bh * bsize, std::byte{0});
    } catch (...) {
        return false;
    }
    std::array<std::uint8_t, 64> block{};
    for (std::uint32_t by = 0U; by < bh; ++by) {
        for (std::uint32_t bx = 0U; bx < bw; ++bx) {
            for (std::uint32_t py = 0U; py < 4U; ++py) {
                const auto y = std::min(by * 4U + py, image.height - 1U);
                for (std::uint32_t pxi = 0U; pxi < 4U; ++pxi) {
                    const auto x = std::min(bx * 4U + pxi, image.width - 1U);
                    std::memcpy(block.data() + (py * 4U + pxi) * 4U,
                                image.rgba8.data() + (static_cast<std::size_t>(y) * image.width + x) * 4U, 4U);
                }
            }
            encode_block(format, block.data(),
                         out->data() + (static_cast<std::size_t>(by) * bw + bx) * bsize);
        }
    }
    return true;
}

RgbaImage downsample(const RgbaImage& image) {
    RgbaImage out;
    if (!image.available()) return out;
    out.width = std::max(1U, image.width / 2U);
    out.height = std::max(1U, image.height / 2U);
    out.rgba8.assign(static_cast<std::size_t>(out.width) * out.height * 4U, 0U);
    for (std::uint32_t y = 0U; y < out.height; ++y) {
        for (std::uint32_t x = 0U; x < out.width; ++x) {
            std::array<std::uint32_t, 4> sum{};
            std::uint32_t n = 0U;
            for (std::uint32_t dy = 0U; dy < 2U; ++dy) {
                for (std::uint32_t dx = 0U; dx < 2U; ++dx) {
                    const auto sx = x * 2U + dx;
                    const auto sy = y * 2U + dy;
                    if (sx >= image.width || sy >= image.height) continue;
                    const auto* p = image.rgba8.data() + (static_cast<std::size_t>(sy) * image.width + sx) * 4U;
                    for (std::size_t c = 0U; c < 4U; ++c) sum[c] += p[c];
                    ++n;
                }
            }
            auto* d = out.rgba8.data() + (static_cast<std::size_t>(y) * out.width + x) * 4U;
            for (std::size_t c = 0U; c < 4U; ++c) d[c] = static_cast<std::uint8_t>((sum[c] + n / 2U) / n);
        }
    }
    return out;
}

std::optional<std::uint32_t> legacy_fourcc(Format format) noexcept {
    switch (format) {
    case Format::bc1: return fourcc('D', 'X', 'T', '1');
    case Format::bc2: return fourcc('D', 'X', 'T', '3');
    case Format::bc3: return fourcc('D', 'X', 'T', '5');
    case Format::bc4_unorm: return fourcc('B', 'C', '4', 'U');
    case Format::bc4_snorm: return fourcc('B', 'C', '4', 'S');
    case Format::bc5_unorm: return fourcc('B', 'C', '5', 'U');
    case Format::bc5_snorm: return fourcc('B', 'C', '5', 'S');
    case Format::bc6h_uf16:
    case Format::bc6h_sf16:
    case Format::bc7: return std::nullopt;
    }
    return std::nullopt;
}

std::uint32_t dxgi_format(Format format) noexcept {
    switch (format) {
    case Format::bc1: return 71U;
    case Format::bc2: return 74U;
    case Format::bc3: return 77U;
    case Format::bc4_unorm: return 80U;
    case Format::bc4_snorm: return 81U;
    case Format::bc5_unorm: return 83U;
    case Format::bc5_snorm: return 84U;
    case Format::bc6h_uf16: return 95U;
    case Format::bc6h_sf16: return 96U;
    case Format::bc7: return 98U;
    }
    return 0U;
}

DdsEncodeResult encode_dds(Format format, std::span<const RgbaImage> levels,
                           const DdsEncodeOptions& options) {
    DdsEncodeResult result{};
    if (levels.empty() || !levels.front().available()) {
        result.detail = "DDS encode needs at least one RGBA level";
        return result;
    }
    const auto width = levels.front().width;
    const auto height = levels.front().height;
    if (levels.size() > maximum_mip_count(width, height)) {
        result.detail = "DDS encode received more levels than the image pyramid has";
        return result;
    }
    for (std::size_t l = 0U; l < levels.size(); ++l) {
        if (!levels[l].available() || levels[l].width != std::max(1U, width >> l) ||
            levels[l].height != std::max(1U, height >> l)) {
            result.detail = "DDS encode levels do not halve from level 0";
            return result;
        }
    }
    const auto legacy = legacy_fourcc(format);
    const bool dx10 = options.force_dx10 || !legacy.has_value();
    const std::size_t header = dx10 ? dx10_header_size : legacy_header_size;
    std::vector<std::byte> payload;
    try {
        for (const auto& level : levels) {
            std::vector<std::byte> encoded;
            if (!encode_level(format, level, &encoded)) {
                result.detail = "DDS level encode failed";
                return result;
            }
            payload.insert(payload.end(), encoded.begin(), encoded.end());
        }
        result.bytes.assign(header + payload.size(), std::byte{0});
    } catch (...) {
        result.detail = "DDS encode allocation failed";
        return result;
    }
    auto* p = result.bytes.data();
    const bool mips = levels.size() > 1U;
    std::uint64_t level0 = 0U;
    (void)level_size(width, height, format, &level0);
    put_u32(p, fourcc('D', 'D', 'S', ' '));
    put_u32(p + 4U, 124U);
    put_u32(p + 8U, 0x1007U | 0x80000U | (mips ? 0x20000U : 0U));
    put_u32(p + 12U, height);
    put_u32(p + 16U, width);
    put_u32(p + 20U, static_cast<std::uint32_t>(level0));
    put_u32(p + 28U, mips ? static_cast<std::uint32_t>(levels.size()) : 0U);
    put_u32(p + 76U, 32U);
    put_u32(p + 80U, 4U);
    put_u32(p + 84U, dx10 ? fourcc('D', 'X', '1', '0') : *legacy);
    put_u32(p + 108U, 0x1000U | (mips ? 0x400008U : 0U));
    if (dx10) {
        put_u32(p + 128U, dxgi_format(format));
        put_u32(p + 132U, 3U);  // TEXTURE2D
        put_u32(p + 140U, 1U);  // array size
    }
    std::memcpy(p + header, payload.data(), payload.size());
    result.ok = true;
    return result;
}

std::optional<Format> parse_format_name(std::string_view name) noexcept {
    std::string n;
    try {
        for (const char ch : name) {
            if (ch == '-' || ch == ' ') continue;
            n.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
    } catch (...) {
        return std::nullopt;
    }
    if (n == "bc1" || n == "dxt1") return Format::bc1;
    if (n == "bc2" || n == "dxt3" || n == "dxt2") return Format::bc2;
    if (n == "bc3" || n == "dxt5" || n == "dxt4") return Format::bc3;
    if (n == "bc4" || n == "bc4u" || n == "ati1") return Format::bc4_unorm;
    if (n == "bc4s") return Format::bc4_snorm;
    if (n == "bc5" || n == "bc5u" || n == "ati2") return Format::bc5_unorm;
    if (n == "bc5s") return Format::bc5_snorm;
    if (n == "bc6h" || n == "bc6" || n == "bc6huf16") return Format::bc6h_uf16;
    if (n == "bc6hsf16" || n == "bc6hs") return Format::bc6h_sf16;
    if (n == "bc7") return Format::bc7;
    return std::nullopt;
}

std::span<const Format> writable_formats() noexcept {
    return k_writable;
}

}  // namespace dmc::rengine::codecs::dds_bcn
