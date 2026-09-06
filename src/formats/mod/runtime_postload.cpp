#include "dmc_rengine/formats/mod/runtime_postload.hpp"

#include <array>
#include <limits>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::mod::runtime_postload {
namespace {

[[nodiscard]] std::uint16_t read_u16_le(
    const std::span<const std::byte> bytes,
    const std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset])) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(
                std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U);
}

[[nodiscard]] std::uint64_t read_u64_le(
    const std::span<const std::byte> bytes,
    const std::size_t offset) noexcept {
    std::uint64_t value{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

void write_u16_le(
    const std::span<std::byte> bytes,
    const std::size_t offset,
    const std::uint16_t value) noexcept {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32_le(
    const std::span<std::byte> bytes,
    const std::size_t offset,
    const std::uint32_t value) noexcept {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

void write_u64_le(
    const std::span<std::byte> bytes,
    const std::size_t offset,
    const std::uint64_t value) noexcept {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] bool range_fits(
    const std::size_t offset,
    const std::size_t size,
    const std::size_t total) noexcept {
    return offset <= total && size <= total - offset;
}

[[nodiscard]] bool add_pointer(
    const std::uint64_t base,
    const std::uint64_t relative,
    std::uint64_t& absolute) noexcept {
    if (relative > std::numeric_limits<std::uint64_t>::max() - base) {
        return false;
    }
    absolute = base + relative;
    return true;
}

[[nodiscard]] std::vector<std::uint16_t> build_generated_words(
    const std::span<const std::uint16_t> raw) {
    std::vector<std::uint16_t> out;
    out.reserve(raw.size() * 3U);

    std::uint32_t previous_continuation =
        std::numeric_limits<std::uint32_t>::max();
    bool continuation_active = false;

    for (std::uint32_t index = 0U;
         index < static_cast<std::uint32_t>(raw.size());
         ++index) {
        const bool continues = index > 1U &&
            (raw[index] & 0x8000U) == 0U;
        if (!continues) {
            continuation_active = false;
            continue;
        }

        if (!continuation_active) {
            continuation_active = true;
            if (previous_continuation !=
                std::numeric_limits<std::uint32_t>::max()) {
                out.push_back(
                    static_cast<std::uint16_t>(previous_continuation));
                out.push_back(static_cast<std::uint16_t>(index - 2U));
            }
            out.push_back(static_cast<std::uint16_t>(index - 2U));
            out.push_back(static_cast<std::uint16_t>(index - 1U));
        }

        out.push_back(static_cast<std::uint16_t>(index));
        previous_continuation = index;
    }
    return out;
}

struct MeshPlan final {
    std::size_t mesh_offset{};
    std::size_t packed_stream_offset{};
    std::size_t generated_workspace{};
    std::array<std::uint64_t, 5> relative_fields{};
    std::vector<std::uint16_t> cleared_words;
    std::vector<std::uint16_t> generated_words;
};

struct ObjectPlan final {
    std::size_t object_offset{};
    std::uint64_t mesh_array_relative{};
    std::vector<MeshPlan> meshes;
};

} // namespace

Result apply_in_place(const std::span<std::byte> bytes) {
    Result result;
    if (!range_fits(0U, first_object_offset, bytes.size())) {
        result.status = Status::truncated_header;
        return result;
    }

    const auto object_count = std::to_integer<std::uint8_t>(
        bytes[header_object_count_offset]);
    result.object_count = object_count;
    if (!range_fits(
            first_object_offset,
            static_cast<std::size_t>(object_count) * object_stride,
            bytes.size())) {
        result.status = Status::object_table_out_of_bounds;
        return result;
    }

    const auto base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(bytes.data()));
    const auto document_relative = read_u64_le(
        bytes, header_document_pointer_offset);
    std::uint64_t document_absolute{};
    if (!add_pointer(base, document_relative, document_absolute)) {
        result.status = Status::pointer_overflow;
        return result;
    }

    std::vector<ObjectPlan> objects;
    objects.reserve(object_count);

    constexpr std::array<std::size_t, 5> base_relative_fields{
        0x10U, 0x18U, 0x20U, 0x28U, 0x30U,
    };

    for (std::size_t object_index = 0U;
         object_index < object_count;
         ++object_index) {
        const auto object_offset =
            first_object_offset + object_index * object_stride;
        const auto mesh_count = std::to_integer<std::uint8_t>(
            bytes[object_offset + object_mesh_count_offset]);
        const auto mesh_array_relative = read_u64_le(
            bytes, object_offset + object_mesh_array_offset);

        if (mesh_array_relative > bytes.size() ||
            !range_fits(
                static_cast<std::size_t>(mesh_array_relative),
                static_cast<std::size_t>(mesh_count) * mesh_stride,
                bytes.size())) {
            result.status = Status::mesh_table_out_of_bounds;
            return result;
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
            const auto element_count = read_u16_le(
                bytes, mesh_offset + mesh_element_count_offset);

            MeshPlan mesh{
                .mesh_offset = mesh_offset,
                .packed_stream_offset = 0U,
                .generated_workspace = 0U,
                .relative_fields = {},
                .cleared_words = {},
                .generated_words = {},
            };

            for (std::size_t field = 0U;
                 field < base_relative_fields.size();
                 ++field) {
                const auto relative = read_u64_le(
                    bytes, mesh_offset + base_relative_fields[field]);
                std::uint64_t absolute{};
                if (!add_pointer(base, relative, absolute)) {
                    result.status = Status::pointer_overflow;
                    return result;
                }
                mesh.relative_fields[field] = relative;
            }

            const auto packed_relative = mesh.relative_fields[4U];
            if (packed_relative > bytes.size() ||
                !range_fits(
                    static_cast<std::size_t>(packed_relative),
                    static_cast<std::size_t>(element_count) * 2U,
                    bytes.size())) {
                result.status = Status::packed_stream_out_of_bounds;
                return result;
            }
            mesh.packed_stream_offset =
                static_cast<std::size_t>(packed_relative);

            const auto generated_relative = read_u64_le(
                bytes, mesh_offset + mesh_generated_workspace_offset);
            if (generated_relative >
                std::numeric_limits<std::size_t>::max() - mesh_offset) {
                result.status = Status::generated_workspace_out_of_bounds;
                return result;
            }
            mesh.generated_workspace =
                mesh_offset + static_cast<std::size_t>(generated_relative);
            if (mesh.generated_workspace > bytes.size()) {
                result.status = Status::generated_workspace_out_of_bounds;
                return result;
            }

            std::vector<std::uint16_t> raw;
            raw.reserve(element_count);
            mesh.cleared_words.reserve(element_count);
            for (std::size_t index = 0U;
                 index < element_count;
                 ++index) {
                const auto word = read_u16_le(
                    bytes, mesh.packed_stream_offset + index * 2U);
                raw.push_back(word);
                mesh.cleared_words.push_back(
                    static_cast<std::uint16_t>(word & 0x7FFFU));
            }

            mesh.generated_words = build_generated_words(raw);
            if (!range_fits(
                    mesh.generated_workspace,
                    mesh.generated_words.size() * 2U,
                    bytes.size())) {
                result.status = Status::generated_workspace_out_of_bounds;
                return result;
            }
            if (mesh.generated_words.size() >
                std::numeric_limits<std::uint32_t>::max()) {
                result.status = Status::generated_word_count_overflow;
                return result;
            }

            result.generated_word_count += mesh.generated_words.size();
            ++result.mesh_count;
            object.meshes.push_back(std::move(mesh));
        }
        objects.push_back(std::move(object));
    }

    write_u64_le(bytes, header_document_pointer_offset, document_absolute);
    for (const auto& object : objects) {
        std::uint64_t mesh_array_absolute{};
        (void)add_pointer(base, object.mesh_array_relative, mesh_array_absolute);
        write_u64_le(
            bytes,
            object.object_offset + object_mesh_array_offset,
            mesh_array_absolute);

        for (const auto& mesh : object.meshes) {
            for (std::size_t field = 0U;
                 field < base_relative_fields.size();
                 ++field) {
                std::uint64_t absolute{};
                (void)add_pointer(base, mesh.relative_fields[field], absolute);
                write_u64_le(
                    bytes,
                    mesh.mesh_offset + base_relative_fields[field],
                    absolute);
            }

            const auto generated_relative = read_u64_le(
                bytes, mesh.mesh_offset + mesh_generated_workspace_offset);
            const auto generated_absolute =
                base + static_cast<std::uint64_t>(mesh.mesh_offset) +
                generated_relative;
            write_u64_le(
                bytes,
                mesh.mesh_offset + mesh_generated_workspace_offset,
                generated_absolute);

            for (std::size_t index = 0U;
                 index < mesh.cleared_words.size();
                 ++index) {
                write_u16_le(
                    bytes,
                    mesh.packed_stream_offset + index * 2U,
                    mesh.cleared_words[index]);
            }
            for (std::size_t index = 0U;
                 index < mesh.generated_words.size();
                 ++index) {
                write_u16_le(
                    bytes,
                    mesh.generated_workspace + index * 2U,
                    mesh.generated_words[index]);
            }
            write_u32_le(
                bytes,
                mesh.mesh_offset + mesh_generated_word_count_offset,
                static_cast<std::uint32_t>(mesh.generated_words.size()));
        }
    }

    result.status = Status::ok;
    return result;
}

} // namespace dmc::rengine::formats::mod::runtime_postload
