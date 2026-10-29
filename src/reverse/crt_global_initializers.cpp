#include "dmc_rengine/reverse/crt_global_initializers.hpp"

namespace dmc::rengine::reverse {

void cleanup_cb9ed0() noexcept {}
void cleanup_cf2d90() noexcept {}

void initialize_cb9ed0(std::span<CrtRecord16, 768> records, RegisterExit register_exit) {
    // EXE 0x140021A40..0x140021A4E. Do not clear the six unknown bytes.
    for (auto& record : records) {
        record.sentinel = 0xffff;
        record.word_08 = 0;
    }
    (void)register_exit(cleanup_cb9ed0);
}

void initialize_cf2d90(std::span<std::uint32_t, 16> float_bits,
                      std::span<CrtRecord8, 16> records,
                      std::uint8_t& trailing_flag, RegisterExit register_exit) {
    // EXE writes the IEEE-754 bit pattern for 1.0f directly, without arithmetic.
    for (std::size_t i = 0; i < 16; ++i) {
        float_bits[i] = 0x3f800000;
        records[i].float_bits_00 = 0x3f800000;
        records[i].flag_04 = 0;
    }
    trailing_flag = 0;
    (void)register_exit(cleanup_cf2d90);
}

} // namespace dmc::rengine::reverse
