#pragma once

#include <array>
#include <cstdint>

// DMC3 HD vertex formats built by 0x1400473F0 (Gate 48 v0.3d). For each
// format it takes a shader record (table 0x140C0CFA0, lookup 0x1400461B0),
// creates two D3D11 input layouts from static D3D11_INPUT_ELEMENT_DESC arrays
// in .rdata (ID3D11Device slot +0x58, element count at record +0x10) and
// stores a D3D9-style FVF code (+0x28) and the stride (+0x30).
//
// Evidence: docs/research/dmc3-light-pipeline-gates-2026-10-29.md; the FVF
// immediates and the layout references are re-checked by
// tools/reverse/dmc3_gates.cpp (dmc3.exe SHA-256 e454272e...dcbdd082).
namespace dmc::rengine::profiles::dmc3::vertex_layouts {

inline constexpr std::uint64_t builder_va = 0x1400473F0ULL;
inline constexpr std::uint64_t shader_table_va = 0x140C0CFA0ULL;
inline constexpr std::uint64_t shader_lookup_va = 0x1400461B0ULL;
// The legacy GS vertex emitter: DIFFUSE as a packed dword, fog byte x 1/255.
inline constexpr std::uint64_t legacy_gs_emitter_va = 0x14002BAD0ULL;

inline constexpr std::uint32_t fvf_xyz = 0x002U;
inline constexpr std::uint32_t fvf_normal = 0x010U;
inline constexpr std::uint32_t fvf_diffuse = 0x040U;
inline constexpr std::uint32_t fvf_tex_count_shift = 8U;  // 0x100 = TEX1

struct VertexFormat final {
    int shader_id;             // -1: the record is not identified
    std::uint64_t layout_a_va;
    std::uint64_t layout_b_va;
    std::uint32_t fvf;
    std::uint32_t stride;
};

inline constexpr std::array<VertexFormat, 8> formats{{
    {0x5, 0x140562620ULL, 0x140562920ULL, 0x80030112U, 0x28U},
    {0x8, 0x1405626C0ULL, 0x1405629C0ULL, 0x80030152U, 0x38U},
    {0xD, 0x140562780ULL, 0x140562A80ULL, 0x80000152U, 0x30U},
    {0xF, 0x140562800ULL, 0x140562800ULL, 0x00000002U, 0x0CU},
    {0xE, 0x140562820ULL, 0x140562820ULL, 0x40000002U, 0x10U},
    {0x4, 0x140562880ULL, 0x140562B40ULL, 0x00000102U, 0x14U},
    {0x1, 0x1405628C0ULL, 0x140562B80ULL, 0x00000142U, 0x24U},
    {-1, 0x140562840ULL, 0x140562B00ULL, 0x00000042U, 0x1CU},
}};

// The stride the FVF bits give, with the reading that fits all eight formats:
// XYZ 12, NORMAL 12, DIFFUSE 16 (four floats, not a packed dword: confirmed by
// the strides), 8 per texture set. Two engine bits are inferred from the
// strides only (candidate): (fvf >> 16) & 3 == 3 adds 8, 0x40000000 adds 4;
// 0x80000000 adds nothing.
[[nodiscard]] constexpr std::uint32_t fvf_stride(std::uint32_t fvf) noexcept {
    std::uint32_t stride = 0U;
    if ((fvf & fvf_xyz) != 0U) stride += 12U;
    if ((fvf & fvf_normal) != 0U) stride += 12U;
    if ((fvf & fvf_diffuse) != 0U) stride += 16U;
    stride += 8U * ((fvf >> fvf_tex_count_shift) & 0xFU);
    if (((fvf >> 16U) & 3U) == 3U) stride += 8U;
    if ((fvf & 0x40000000U) != 0U) stride += 4U;
    return stride;
}

} // namespace dmc::rengine::profiles::dmc3::vertex_layouts
