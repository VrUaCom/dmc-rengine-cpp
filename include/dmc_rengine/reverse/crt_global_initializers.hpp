#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::reverse {

// Address-derived names: the application meaning of these banks is still open.
// Only written fields are named. Unknown bytes must survive initialization.
struct CrtRecord16 {
    std::uint16_t sentinel;
    std::array<std::byte, 6> preserved_02;
    std::uint64_t word_08;
};
static_assert(sizeof(CrtRecord16) == 0x10);
static_assert(offsetof(CrtRecord16, word_08) == 8);

struct CrtRecord8 {
    std::uint32_t float_bits_00;
    std::uint8_t flag_04;
    std::array<std::byte, 3> preserved_05;
};
static_assert(sizeof(CrtRecord8) == 8);
static_assert(offsetof(CrtRecord8, flag_04) == 4);

using ExitCallback = void (*)();
using RegisterExit = int (*)(ExitCallback);

// 0x140021A20: [0x140CB9ED0, 0x140CBCED0), 768 records.
// Requires a valid registration function. Its status is discarded by this void
// projection; the EXE tail-jump may leave that status in EAX, unused by _initterm.
void initialize_cb9ed0(std::span<CrtRecord16, 768> records, RegisterExit register_exit);

// 0x1400236A0: 16 dwords at 0x140CF2D90; 16 records at 0x140CF2E10;
// byte at 0x140CF2E90. The 0x40-byte gap and record padding are not written.
void initialize_cf2d90(std::span<std::uint32_t, 16> float_bits,
                      std::span<CrtRecord8, 16> records,
                      std::uint8_t& trailing_flag, RegisterExit register_exit);

// 0x14034EA50 / 0x14034EB50: both are ret 0, with no memory effects.
void cleanup_cb9ed0() noexcept;
void cleanup_cf2d90() noexcept;

} // namespace dmc::rengine::reverse
