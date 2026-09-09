#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::formats::mod {

// MOD-specific serialized object fields proven live by canonical dmc3.exe.
// Keep these outside model_family::ObjectCoreAbi: identical physical offsets
// in SCM/EFM do not establish identical semantics.
struct ObjectSerializedAbi final {
    static constexpr std::size_t record_size = 0x40U;
    static constexpr std::size_t conditional_parameter_f32_field = 0x18U;
    static constexpr std::size_t conditional_parameter_u32_field = 0x1CU;
};

struct ObjectConditionalParameters final {
    float parameter_f32{};
    std::uint32_t parameter_u32{};
};

[[nodiscard]] constexpr std::optional<ObjectConditionalParameters>
decode_object_conditional_parameters(
    std::span<const std::byte> bytes,
    std::size_t object_record_offset) noexcept {
    constexpr auto end =
        ObjectSerializedAbi::conditional_parameter_u32_field + sizeof(std::uint32_t);
    if (object_record_offset > bytes.size() ||
        end > bytes.size() - object_record_offset) {
        return std::nullopt;
    }

    const auto read_u32_le = [&](std::size_t relative) constexpr {
        std::uint32_t value{};
        for (std::size_t i = 0; i < 4; ++i) {
            value |= static_cast<std::uint32_t>(
                std::to_integer<std::uint8_t>(
                    bytes[object_record_offset + relative + i])) << (8U * i);
        }
        return value;
    };

    const auto raw_f32 = read_u32_le(
        ObjectSerializedAbi::conditional_parameter_f32_field);
    return ObjectConditionalParameters{
        .parameter_f32 = std::bit_cast<float>(raw_f32),
        .parameter_u32 = read_u32_le(
            ObjectSerializedAbi::conditional_parameter_u32_field),
    };
}

[[nodiscard]] constexpr bool conditional_parameters_are_active(
    std::uint32_t source_flags) noexcept {
    return (source_flags & 0x00000600U) != 0U;
}

// Direct executable chain, canonical dmc3.exe:
//   source object = source_header + 0x40 + object_index*0x40
//   0x140302D22: source +0x18 f32 -> runtime object +0x16C
//   0x140302D37: source +0x1C u32 -> runtime object +0x170
// repeated for source flag 0x00000400 at 0x140302D82/0x140302D97.
// Both 0x00000200 and 0x00000400 request this copy. Runtime +0x170 is later
// converted to float at 0x1403040D9 and written into per-mesh/pass state;
// runtime +0x160..+0x16C is copied as a four-float group at 0x1403040C6.
// The artistic meaning remains intentionally unnamed.
static_assert(ObjectSerializedAbi::record_size == 0x40U);
static_assert(ObjectSerializedAbi::conditional_parameter_f32_field == 0x18U);
static_assert(ObjectSerializedAbi::conditional_parameter_u32_field == 0x1CU);
static_assert(conditional_parameters_are_active(0x00000200U));
static_assert(conditional_parameters_are_active(0x00000400U));
static_assert(!conditional_parameters_are_active(0x00000000U));

} // namespace dmc::rengine::formats::mod
