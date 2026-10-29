#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::reverse {

// 0x14002AB40 (20 bytes, the Android foundation gate's first lift):
//   test dl, dl; je +8; or dword [rcx+0x228], 4; ret; and dword [rcx+0x228], 0xFFFFFFFB; ret
// Sets bit 2 of the dword at object +0x228 when `enable` (dl) is non-zero and
// clears it otherwise; every other bit is kept. The field's meaning is unknown.
inline constexpr std::uint64_t object_flag_228_va = 0x14002AB40ULL;
inline constexpr std::size_t object_flag_228_offset = 0x228U;
inline constexpr std::uint32_t object_flag_228_bit = 0x4U;

void set_object_flag_228(std::uint32_t& field, std::uint8_t enable) noexcept;

} // namespace dmc::rengine::reverse
