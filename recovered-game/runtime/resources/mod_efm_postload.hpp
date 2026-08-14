#pragma once

#include "runtime/resources/resource_manager.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace dmc::recovered::dmc3::runtime::resources {

inline constexpr std::uint64_t mod_postload_helper_va = 0x1402FE3B0ULL;
inline constexpr std::uint64_t efm_postload_helper_va = 0x1402F7A90ULL;

namespace mod_efm_detail {

inline constexpr std::size_t header_object_count_offset = 0x10U;
inline constexpr std::size_t header_pointer_20_offset = 0x20U;
inline constexpr std::size_t first_object_offset = 0x40U;
inline constexpr std::size_t object_stride = 0x40U;
inline constexpr std::size_t object_mesh_count_offset = 0x00U;
inline constexpr std::size_t object_mesh_array_offset = 0x08U;
inline constexpr std::size_t mesh_stride = 0x50U;
inline constexpr std::size_t mesh_index_count_offset = 0x00U;
inline constexpr std::size_t mesh_index_array_offset = 0x30U;
inline constexpr std::size_t mesh_tri_command_offset = 0x40U;
inline constexpr std::size_t mesh_tri_command_count_offset = 0x48U;

[[nodiscard]] inline std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset])) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint8_t>(bytes[offset + 1U]) << 8U);
}

inline void write_u16_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint16_t value) noexcept {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

[[nodiscard]] inline std::uint64_t read_u64_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint64_t value{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

inline void write_u32_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint32_t value) noexcept {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

inline void write_u64_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint64_t value) noexcept {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] inline bool range_fits(
    std::size_t offset,
    std::size_t size,
    std::size_t total) noexcept {
    return offset <= total && size <= total - offset;
}

[[nodiscard]] inline bool add_base_pointer(
    std::span<std::byte> bytes,
    std::size_t field_offset,
    std::uint64_t base,
    std::uint64_t relative) noexcept {
    if (relative > std::numeric_limits<std::uint64_t>::max() - base) {
        return false;
    }
    write_u64_le(bytes, field_offset, base + relative);
    return true;
}

struct MeshPlan final {
    std::size_t mesh_offset{};
    std::size_t index_array_offset{};
    std::size_t tri_command_offset{};
    std::uint16_t index_count{};
    std::array<std::uint64_t, 6> base_relative_fields{};
    std::size_t base_relative_field_count{};
    std::vector<std::uint16_t> cleared_indices;
    std::vector<std::uint16_t> tri_commands;
};

struct ObjectPlan final {
    std::size_t object_offset{};
    std::uint64_t mesh_array_relative{};
    std::vector<MeshPlan> meshes;
};

[[nodiscard]] inline std::vector<std::uint16_t> build_tri_commands(
    std::span<const std::uint16_t> raw_indices) {
    std::vector<std::uint16_t> output;
    output.reserve(raw_indices.size() * 3U);

    std::uint32_t previous_continuation =
        std::numeric_limits<std::uint32_t>::max();
    bool continuation_active = false;

    for (std::uint32_t index = 0U; index < raw_indices.size(); ++index) {
        const bool continues = index > 1U &&
            (raw_indices[index] & 0x8000U) == 0U;
        if (!continues) {
            continuation_active = false;
            continue;
        }

        if (!continuation_active) {
            continuation_active = true;
            if (previous_continuation !=
                std::numeric_limits<std::uint32_t>::max()) {
                output.push_back(
                    static_cast<std::uint16_t>(previous_continuation));
                output.push_back(static_cast<std::uint16_t>(index - 2U));
            }
            output.push_back(static_cast<std::uint16_t>(index - 2U));
            output.push_back(static_cast<std::uint16_t>(index - 1U));
        }

        output.push_back(static_cast<std::uint16_t>(index));
        previous_continuation = index;
    }
    return output;
}

} // namespace mod_efm_detail

enum class ModEfmPostLoadFlavor {
    mod,
    efm,
};

// Direct reconstruction of canonical helpers 0x1402FE3B0 (MOD) and
// 0x1402F7A90 (EFM). Both share the same object/mesh/tri-command pass; EFM also
// fixes mesh +0x38 as a base-relative pointer.
//
// The function is transaction-like for malformed research fixtures: it validates
// every range and builds the command streams before mutating bytes. On valid game
// data this preserves the original resulting pointer/index/command state while
// avoiding partial writes on an invalid host-side span.
//
// Status: disassembly-complete for this helper body; real-corpus byte-for-byte
// validation remains a separate promotion gate.
[[nodiscard]] inline bool apply_mod_efm_postload(
    std::span<std::byte> bytes,
    ModEfmPostLoadFlavor flavor) {
    using namespace mod_efm_detail;

    if (!range_fits(0U, first_object_offset, bytes.size())) {
        return false;
    }

    const auto object_count = std::to_integer<std::uint8_t>(
        bytes[header_object_count_offset]);
    if (!range_fits(
            first_object_offset,
            static_cast<std::size_t>(object_count) * object_stride,
            bytes.size())) {
        return false;
    }

    const auto header_pointer_relative = read_u64_le(
        bytes, header_pointer_20_offset);
    const auto base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(bytes.data()));
    if (header_pointer_relative >
        std::numeric_limits<std::uint64_t>::max() - base) {
        return false;
    }

    std::vector<ObjectPlan> objects;
    objects.reserve(object_count);

    for (std::size_t object_index = 0U;
         object_index < object_count;
         ++object_index) {
        const auto object_offset =
            first_object_offset + object_index * object_stride;
        const auto mesh_count = std::to_integer<std::uint8_t>(
            bytes[object_offset + object_mesh_count_offset]);
        const auto mesh_array_relative = read_u64_le(
            bytes, object_offset + object_mesh_array_offset);

        if (mesh_array_relative > bytes.size()) {
            return false;
        }
        if (!range_fits(
                static_cast<std::size_t>(mesh_array_relative),
                static_cast<std::size_t>(mesh_count) * mesh_stride,
                bytes.size())) {
            return false;
        }

        ObjectPlan object{
            .object_offset = object_offset,
            .mesh_array_relative = mesh_array_relative,
            .meshes = {},
        };
        object.meshes.reserve(mesh_count);

        for (std::size_t mesh_index = 0U;
             mesh_index < mesh_count;
             ++mesh_index) {
            const auto mesh_offset =
                static_cast<std::size_t>(mesh_array_relative) +
                mesh_index * mesh_stride;
            const auto index_count = read_u16_le(
                bytes, mesh_offset + mesh_index_count_offset);

            constexpr std::array<std::size_t, 6> pointer_fields{
                0x10U, 0x18U, 0x20U, 0x28U, 0x30U, 0x38U,
            };
            const auto pointer_field_count =
                flavor == ModEfmPostLoadFlavor::efm ? 6U : 5U;

            MeshPlan mesh{
                .mesh_offset = mesh_offset,
                .index_array_offset = 0U,
                .tri_command_offset = 0U,
                .index_count = index_count,
                .base_relative_fields = {},
                .base_relative_field_count = pointer_field_count,
                .cleared_indices = {},
                .tri_commands = {},
            };

            for (std::size_t field = 0U;
                 field < pointer_field_count;
                 ++field) {
                const auto relative = read_u64_le(
                    bytes, mesh_offset + pointer_fields[field]);
                if (relative >
                    std::numeric_limits<std::uint64_t>::max() - base) {
                    return false;
                }
                mesh.base_relative_fields[field] = relative;
            }

            const auto index_relative = mesh.base_relative_fields[4U];
            if (index_relative > bytes.size() ||
                !range_fits(
                    static_cast<std::size_t>(index_relative),
                    static_cast<std::size_t>(index_count) * 2U,
                    bytes.size())) {
                return false;
            }
            mesh.index_array_offset = static_cast<std::size_t>(index_relative);

            const auto tri_relative = read_u64_le(
                bytes, mesh_offset + mesh_tri_command_offset);
            if (tri_relative >
                std::numeric_limits<std::size_t>::max() - mesh_offset) {
                return false;
            }
            mesh.tri_command_offset =
                mesh_offset + static_cast<std::size_t>(tri_relative);
            if (mesh.tri_command_offset > bytes.size()) {
                return false;
            }

            mesh.cleared_indices.reserve(index_count);
            for (std::size_t index = 0U; index < index_count; ++index) {
                const auto raw = read_u16_le(
                    bytes, mesh.index_array_offset + index * 2U);
                mesh.cleared_indices.push_back(
                    static_cast<std::uint16_t>(raw & 0x7FFFU));
            }

            // Command generation uses the pre-clear high-bit markers.
            std::vector<std::uint16_t> raw_indices;
            raw_indices.reserve(index_count);
            for (std::size_t index = 0U; index < index_count; ++index) {
                raw_indices.push_back(read_u16_le(
                    bytes, mesh.index_array_offset + index * 2U));
            }
            mesh.tri_commands = build_tri_commands(raw_indices);
            if (!range_fits(
                    mesh.tri_command_offset,
                    mesh.tri_commands.size() * 2U,
                    bytes.size())) {
                return false;
            }
            if (mesh.tri_commands.size() >
                std::numeric_limits<std::uint32_t>::max()) {
                return false;
            }

            object.meshes.push_back(std::move(mesh));
        }
        objects.push_back(std::move(object));
    }

    write_u64_le(
        bytes,
        header_pointer_20_offset,
        base + header_pointer_relative);

    constexpr std::array<std::size_t, 6> pointer_fields{
        0x10U, 0x18U, 0x20U, 0x28U, 0x30U, 0x38U,
    };
    for (const auto& object : objects) {
        write_u64_le(
            bytes,
            object.object_offset + object_mesh_array_offset,
            base + object.mesh_array_relative);

        for (const auto& mesh : object.meshes) {
            for (std::size_t field = 0U;
                 field < mesh.base_relative_field_count;
                 ++field) {
                if (!add_base_pointer(
                        bytes,
                        mesh.mesh_offset + pointer_fields[field],
                        base,
                        mesh.base_relative_fields[field])) {
                    return false;
                }
            }

            const auto tri_relative = read_u64_le(
                bytes, mesh.mesh_offset + mesh_tri_command_offset);
            const auto tri_absolute = base +
                static_cast<std::uint64_t>(mesh.mesh_offset) + tri_relative;
            write_u64_le(
                bytes,
                mesh.mesh_offset + mesh_tri_command_offset,
                tri_absolute);

            for (std::size_t index = 0U;
                 index < mesh.cleared_indices.size();
                 ++index) {
                write_u16_le(
                    bytes,
                    mesh.index_array_offset + index * 2U,
                    mesh.cleared_indices[index]);
            }
            for (std::size_t command = 0U;
                 command < mesh.tri_commands.size();
                 ++command) {
                write_u16_le(
                    bytes,
                    mesh.tri_command_offset + command * 2U,
                    mesh.tri_commands[command]);
            }
            write_u32_le(
                bytes,
                mesh.mesh_offset + mesh_tri_command_count_offset,
                static_cast<std::uint32_t>(mesh.tri_commands.size()));
        }
    }
    return true;
}

class ModEfmPostLoadBackend final : public IConfirmedPostLoadBackend {
public:
    [[nodiscard]] bool apply(
        PostLoadFormat format,
        std::span<std::byte> bytes) override {
        switch (format) {
        case PostLoadFormat::mod:
            return apply_mod_efm_postload(bytes, ModEfmPostLoadFlavor::mod);
        case PostLoadFormat::efm:
            return apply_mod_efm_postload(bytes, ModEfmPostLoadFlavor::efm);
        case PostLoadFormat::scm:
        case PostLoadFormat::shw:
            return false;
        }
        return false;
    }
};

} // namespace dmc::recovered::dmc3::runtime::resources
