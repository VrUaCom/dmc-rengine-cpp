#include "dmc_rengine/analysis/mod/mesh_serialized.hpp"

#include <array>

namespace dmc::rengine::analysis::mod {
namespace {

constexpr auto preserved_fields = [] {
    constexpr std::size_t record_offset = 0x10U;
    std::array<std::byte, 0x60U> bytes{};

    // +0x0C u32 = 0xA1B2C3D4.
    bytes[record_offset + 0x0CU] = std::byte{0xD4U};
    bytes[record_offset + 0x0DU] = std::byte{0xC3U};
    bytes[record_offset + 0x0EU] = std::byte{0xB2U};
    bytes[record_offset + 0x0FU] = std::byte{0xA1U};

    // +0x38 u64 = 0x1122334455667788.
    bytes[record_offset + 0x38U] = std::byte{0x88U};
    bytes[record_offset + 0x39U] = std::byte{0x77U};
    bytes[record_offset + 0x3AU] = std::byte{0x66U};
    bytes[record_offset + 0x3BU] = std::byte{0x55U};
    bytes[record_offset + 0x3CU] = std::byte{0x44U};
    bytes[record_offset + 0x3DU] = std::byte{0x33U};
    bytes[record_offset + 0x3EU] = std::byte{0x22U};
    bytes[record_offset + 0x3FU] = std::byte{0x11U};

    // +0x4C u32 = 0xDEADBEEF.
    bytes[record_offset + 0x4CU] = std::byte{0xEFU};
    bytes[record_offset + 0x4DU] = std::byte{0xBEU};
    bytes[record_offset + 0x4EU] = std::byte{0xADU};
    bytes[record_offset + 0x4FU] = std::byte{0xDEU};

    return decode_mesh_serialized_preservation(bytes, record_offset);
}();

static_assert(preserved_fields.has_value());
static_assert(preserved_fields->preserved0c_u32 == 0xA1B2C3D4U);
static_assert(preserved_fields->preserved38_u64 == 0x1122334455667788ULL);
static_assert(preserved_fields->preserved4c_u32 == 0xDEADBEEFU);

constexpr auto truncated = [] {
    std::array<std::byte, 0x4FU> bytes{};
    return decode_mesh_serialized_preservation(bytes, 0U);
}();
static_assert(!truncated.has_value());

} // namespace
} // namespace dmc::rengine::analysis::mod
