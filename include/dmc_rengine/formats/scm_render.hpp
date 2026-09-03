#pragma once

#include <array>
#include <cstdint>

namespace dmc::rengine::formats::scm {

struct MeshRenderWords final {
    // Serialized mesh +0x04/+0x06/+0x08/+0x0A. Canonical DMC3 consumes
    // these four u16 values in 0x1402F9890. Their higher-level material or
    // render semantics remain unresolved, so keep neutral names.
    std::array<std::uint16_t, 4> values{};

    [[nodiscard]] constexpr std::uint16_t operator[](
        std::size_t index) const noexcept {
        return values[index];
    }
};

// Reconstruct the exact packed runtime state built by 0x1402F9890.
// A zero first word disables the packed state entirely. The stock 68-file
// SCM corpus uses zero for all four words, but the original executable has
// an explicit non-zero path and therefore these bytes are not reserved.
[[nodiscard]] constexpr std::uint64_t pack_mesh_render_words(
    const MeshRenderWords& words) noexcept {
    if (words.values[0] == 0U) return 0U;

    return (static_cast<std::uint64_t>(words.values[0]) << 4U) |
           0x0FULL |
           (static_cast<std::uint64_t>(words.values[1]) << 14U) |
           (static_cast<std::uint64_t>(words.values[2]) << 24U) |
           (static_cast<std::uint64_t>(words.values[3]) << 34U);
}

} // namespace dmc::rengine::formats::scm
