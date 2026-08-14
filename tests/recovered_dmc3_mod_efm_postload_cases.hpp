#pragma once

#include "runtime/resources/mod_efm_postload.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::recovered::dmc3::tests::mod_efm {

inline void write_u16_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

inline void write_u64_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] inline std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset])) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint8_t>(bytes[offset + 1U]) << 8U);
}

[[nodiscard]] inline std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint32_t value{};
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

[[nodiscard]] inline std::uint64_t read_u64_le(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint64_t value{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

[[nodiscard]] inline std::array<std::byte, 0x400> make_fixture(
    bool efm) {
    using namespace runtime::resources::mod_efm_detail;

    std::array<std::byte, 0x400> bytes{};
    bytes[header_object_count_offset] = std::byte{1};
    write_u64_le(bytes, header_pointer_20_offset, 0x300ULL);

    constexpr std::size_t object = first_object_offset;
    bytes[object + object_mesh_count_offset] = std::byte{1};
    write_u64_le(bytes, object + object_mesh_array_offset, 0x100ULL);

    constexpr std::size_t mesh = 0x100U;
    write_u16_le(bytes, mesh + mesh_index_count_offset, 6U);
    write_u64_le(bytes, mesh + 0x10U, 0x310ULL);
    write_u64_le(bytes, mesh + 0x18U, 0x320ULL);
    write_u64_le(bytes, mesh + 0x20U, 0x330ULL);
    write_u64_le(bytes, mesh + 0x28U, 0x340ULL);
    write_u64_le(bytes, mesh + mesh_index_array_offset, 0x200ULL);
    if (efm) {
        write_u64_le(bytes, mesh + 0x38U, 0x350ULL);
    }
    // Unlike the other mesh pointers, +0x40 is relative to the mesh record.
    write_u64_le(bytes, mesh + mesh_tri_command_offset, 0x80ULL);

    constexpr std::array<std::uint16_t, 6> indices{
        0U, 1U, 2U, 3U, static_cast<std::uint16_t>(0x8000U | 4U), 5U,
    };
    for (std::size_t index = 0U; index < indices.size(); ++index) {
        write_u16_le(bytes, 0x200U + index * 2U, indices[index]);
    }
    return bytes;
}

inline void verify_common_result(
    std::span<const std::byte> bytes) {
    using namespace runtime::resources::mod_efm_detail;

    const auto base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(bytes.data()));
    constexpr std::size_t object = first_object_offset;
    constexpr std::size_t mesh = 0x100U;

    assert(read_u64_le(bytes, header_pointer_20_offset) == base + 0x300ULL);
    assert(read_u64_le(bytes, object + object_mesh_array_offset) ==
        base + 0x100ULL);
    assert(read_u64_le(bytes, mesh + 0x10U) == base + 0x310ULL);
    assert(read_u64_le(bytes, mesh + 0x18U) == base + 0x320ULL);
    assert(read_u64_le(bytes, mesh + 0x20U) == base + 0x330ULL);
    assert(read_u64_le(bytes, mesh + 0x28U) == base + 0x340ULL);
    assert(read_u64_le(bytes, mesh + mesh_index_array_offset) ==
        base + 0x200ULL);
    assert(read_u64_le(bytes, mesh + mesh_tri_command_offset) ==
        base + 0x180ULL);

    for (std::uint16_t index = 0U; index < 6U; ++index) {
        assert(read_u16_le(bytes, 0x200U + index * 2U) == index);
    }

    constexpr std::array<std::uint16_t, 9> expected_commands{
        0U, 1U, 2U, 3U, 3U, 3U, 3U, 4U, 5U,
    };
    assert(read_u32_le(bytes, mesh + mesh_tri_command_count_offset) ==
        expected_commands.size());
    for (std::size_t index = 0U; index < expected_commands.size(); ++index) {
        assert(read_u16_le(bytes, 0x180U + index * 2U) ==
            expected_commands[index]);
    }
}

inline void run() {
    using namespace runtime::resources;

    ModEfmPostLoadBackend backend;

    auto mod_bytes = make_fixture(false);
    ResourceRuntimeEntry mod_entry{};
    mod_entry.state = ResourceLoadState::io_complete_pending_postprocess;
    assert(run_confirmed_postload(
        mod_entry, PostLoadFormat::mod, mod_bytes, backend) ==
        ResourceTransitionResult::applied);
    assert(mod_entry.state == ResourceLoadState::ready_postprocessed);
    verify_common_result(mod_bytes);
    // MOD helper does not convert mesh +0x38.
    assert(read_u64_le(mod_bytes, 0x138U) == 0U);

    auto efm_bytes = make_fixture(true);
    ResourceRuntimeEntry efm_entry{};
    efm_entry.state = ResourceLoadState::io_complete_pending_postprocess;
    assert(run_confirmed_postload(
        efm_entry, PostLoadFormat::efm, efm_bytes, backend) ==
        ResourceTransitionResult::applied);
    assert(efm_entry.state == ResourceLoadState::ready_postprocessed);
    verify_common_result(efm_bytes);
    const auto efm_base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(efm_bytes.data()));
    assert(read_u64_le(efm_bytes, 0x138U) == efm_base + 0x350ULL);

    // Backend is intentionally typed, not a replacement dispatcher.
    auto rejected = make_fixture(false);
    assert(!backend.apply(PostLoadFormat::scm, rejected));
    assert(!backend.apply(PostLoadFormat::shw, rejected));

    // Truncated data fails before any recovered pointer writes are committed.
    std::array<std::byte, 0x90> truncated{};
    truncated[mod_efm_detail::header_object_count_offset] = std::byte{1};
    write_u64_le(
        truncated,
        mod_efm_detail::first_object_offset +
            mod_efm_detail::object_mesh_array_offset,
        0x80ULL);
    truncated[mod_efm_detail::first_object_offset] = std::byte{1};
    const auto before_header_pointer = read_u64_le(
        truncated, mod_efm_detail::header_pointer_20_offset);
    assert(!apply_mod_efm_postload(truncated, ModEfmPostLoadFlavor::mod));
    assert(read_u64_le(
        truncated, mod_efm_detail::header_pointer_20_offset) ==
        before_header_pointer);
}

} // namespace dmc::recovered::dmc3::tests::mod_efm
