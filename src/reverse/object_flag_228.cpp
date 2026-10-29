#include "dmc_rengine/reverse/object_flag_228.hpp"

namespace dmc::rengine::reverse {

// A full 32-bit read-modify-write, as the original `or` / `and` dword forms.
void set_object_flag_228(std::uint32_t& field, std::uint8_t enable) noexcept {
    field = enable != 0U ? (field | object_flag_228_bit) : (field & ~object_flag_228_bit);
}

} // namespace dmc::rengine::reverse
