#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/hits/writer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::hits::editor {

using StableSurfaceId = writer::StableSurfaceId;
using StableMeshId = std::uint64_t;
using Surface = writer::Surface;

enum class CollisionPreset : std::uint8_t {
    blue_raw_00000001 = 0,
    orange_raw_00000009 = 1,
    green_raw_0000000a = 2,
    red_raw_18060001 = 3,
};

struct CollisionPresetInfo final {
    CollisionPreset preset{};
    std::uint32_t raw_flags{};
    std::string_view viewer_color;
    std::string_view label;
    std::string_view evidence_boundary;
};

[[nodiscard]] constexpr CollisionPresetInfo collision_preset_info(
    CollisionPreset preset) noexcept {
    switch (preset) {
    case CollisionPreset::blue_raw_00000001:
        return CollisionPresetInfo{
            .preset = preset,
            .raw_flags = 0x00000001U,
            .viewer_color = "blue",
            .label = "Blue / raw 0x00000001",
            .evidence_boundary =
                "Project-observed preset; exact gameplay semantics remain evidence-gated.",
        };
    case CollisionPreset::orange_raw_00000009:
        return CollisionPresetInfo{
            .preset = preset,
            .raw_flags = 0x00000009U,
            .viewer_color = "orange",
            .label = "Orange / raw 0x00000009",
            .evidence_boundary =
                "Project-observed preset; exact gameplay semantics remain evidence-gated.",
        };
    case CollisionPreset::green_raw_0000000a:
        return CollisionPresetInfo{
            .preset = preset,
            .raw_flags = 0x0000000AU,
            .viewer_color = "green",
            .label = "Green / raw 0x0000000A",
            .evidence_boundary =
                "Project-observed preset; exact gameplay semantics remain evidence-gated.",
        };
    case CollisionPreset::red_raw_18060001:
    default:
        return CollisionPresetInfo{
            .preset = CollisionPreset::red_raw_18060001,
            .raw_flags = 0x18060001U,
            .viewer_color = "red",
            .label = "Red / raw 0x18060001",
            .evidence_boundary =
                "Project-observed selective/query-masked variant; exact gameplay semantics remain evidence-gated.",
        };
    }
}

[[nodiscard]] constexpr std::optional<CollisionPreset>
collision_preset_from_flags(std::uint32_t raw_flags) noexcept {
    switch (raw_flags) {
    case 0x00000001U:
        return CollisionPreset::blue_raw_00000001;
    case 0x00000009U:
        return CollisionPreset::orange_raw_00000009;
    case 0x0000000AU:
        return CollisionPreset::green_raw_0000000a;
    case 0x18060001U:
        return CollisionPreset::red_raw_18060001;
    default:
        return std::nullopt;
    }
}

struct Mesh final {
    StableMeshId stable_id{};
    std::vector<StableSurfaceId> surface_ids;
};

struct ScmImportResult final {
    StableMeshId mesh_id{};
    std::vector<StableSurfaceId> surface_ids;
    std::size_t source_object_index{};
    std::size_t source_mesh_index{};
    std::size_t source_node_index{};
};

struct ScmObjectImportResult final {
    StableMeshId mesh_id{};
    std::vector<StableSurfaceId> surface_ids;
    std::size_t source_object_index{};
    std::size_t source_node_index{};
    std::size_t source_mesh_count{};
};

class Session final {
public:
    [[nodiscard]] static std::optional<Session> open(
        std::span<const std::byte> source_bytes);

    [[nodiscard]] const formats::hits::ScanResult& source_scan() const noexcept;
    [[nodiscard]] std::span<const std::byte> source_bytes() const noexcept;
    [[nodiscard]] std::span<const Surface> surfaces() const noexcept;
    [[nodiscard]] std::span<const Mesh> meshes() const noexcept;

    [[nodiscard]] bool dirty() const noexcept;
    [[nodiscard]] bool can_undo() const noexcept;
    [[nodiscard]] bool can_redo() const noexcept;
    [[nodiscard]] std::uint64_t revision() const noexcept;

    [[nodiscard]] std::optional<std::size_t> index_of(
        StableSurfaceId stable_id) const noexcept;

    [[nodiscard]] std::optional<std::size_t> mesh_index_of(
        StableMeshId stable_id) const noexcept;

    [[nodiscard]] std::optional<StableSurfaceId> add_surface(
        std::uint32_t flags,
        const formats::hits::Vec3& point_a,
        const formats::hits::Vec3& point_b,
        const formats::hits::Vec3& point_c);

    [[nodiscard]] std::optional<StableSurfaceId> duplicate_surface(
        StableSurfaceId stable_id);

    [[nodiscard]] bool erase_surface(StableSurfaceId stable_id);

    [[nodiscard]] bool set_flags(
        StableSurfaceId stable_id,
        std::uint32_t flags);

    [[nodiscard]] bool set_flags(
        std::span<const StableSurfaceId> stable_ids,
        std::uint32_t flags);

    [[nodiscard]] bool set_collision_preset(
        StableSurfaceId stable_id,
        CollisionPreset preset);

    [[nodiscard]] bool set_collision_preset(
        std::span<const StableSurfaceId> stable_ids,
        CollisionPreset preset);

    [[nodiscard]] bool set_geometry(
        StableSurfaceId stable_id,
        const formats::hits::Vec3& point_a,
        const formats::hits::Vec3& point_b,
        const formats::hits::Vec3& point_c);

    [[nodiscard]] bool translate_surface(
        StableSurfaceId stable_id,
        const formats::hits::Vec3& delta);

    [[nodiscard]] bool translate_surfaces(
        std::span<const StableSurfaceId> stable_ids,
        const formats::hits::Vec3& delta);

    // Meshes are editor-side logical groups of HITS surfaces. HITS itself is a
    // flat triangle collection + spatial index, so merging meshes does not
    // collapse or decimate triangles in the serialized resource.
    [[nodiscard]] std::optional<StableMeshId> create_mesh(
        std::span<const StableSurfaceId> stable_ids);

    [[nodiscard]] std::optional<StableMeshId> merge_meshes(
        std::span<const StableMeshId> mesh_ids);

    [[nodiscard]] bool erase_mesh(StableMeshId mesh_id);

    [[nodiscard]] bool set_mesh_collision_preset(
        StableMeshId mesh_id,
        CollisionPreset preset);

    [[nodiscard]] bool translate_mesh(
        StableMeshId mesh_id,
        const formats::hits::Vec3& delta);

    [[nodiscard]] std::optional<ScmImportResult> import_scm_mesh(
        const formats::scm::Document& document,
        std::size_t object_index,
        std::size_t mesh_index,
        CollisionPreset preset);

    [[nodiscard]] std::optional<ScmObjectImportResult> import_scm_object(
        const formats::scm::Document& document,
        std::size_t object_index,
        CollisionPreset preset);

    [[nodiscard]] std::optional<StableMeshId> add_quad(
        const formats::hits::Vec3& point_a,
        const formats::hits::Vec3& point_b,
        const formats::hits::Vec3& point_c,
        const formats::hits::Vec3& point_d,
        CollisionPreset preset);

    // Creates four vertical walls (8 triangles) around an axis-aligned box.
    // Floor and ceiling are intentionally not synthesized by this operation.
    [[nodiscard]] std::optional<StableMeshId> create_rectangular_boundary(
        const formats::hits::Vec3& minimum,
        const formats::hits::Vec3& maximum,
        CollisionPreset preset);

    [[nodiscard]] bool undo();
    [[nodiscard]] bool redo();
    [[nodiscard]] bool reset_to_source();

    [[nodiscard]] writer::RebuildResult rebuild() const;
    [[nodiscard]] writer::RebuildResult rebuild(
        writer::RebuildOptions options) const;

private:
    struct Snapshot final {
        std::vector<Surface> surfaces;
        StableSurfaceId next_stable_id{};
        std::vector<Mesh> meshes;
        StableMeshId next_mesh_id{};
        std::uint64_t revision{};
    };

    Session(
        std::vector<std::byte> source_bytes,
        formats::hits::ScanResult source_scan,
        std::vector<Surface> surfaces,
        StableSurfaceId next_stable_id);

    [[nodiscard]] Snapshot snapshot() const;
    void restore(Snapshot state);
    void begin_mutation();

    [[nodiscard]] std::optional<StableSurfaceId> allocate_stable_id() noexcept;
    [[nodiscard]] std::optional<StableMeshId> allocate_mesh_id() noexcept;

    [[nodiscard]] std::optional<StableMeshId> append_triangle_mesh(
        std::span<const std::array<formats::hits::Vec3, 3U>> triangles,
        std::uint32_t flags);

    [[nodiscard]] std::optional<std::vector<std::size_t>>
    resolve_surface_indices(
        std::span<const StableSurfaceId> stable_ids) const;

    std::vector<std::byte> source_bytes_;
    formats::hits::ScanResult source_scan_;
    std::vector<Surface> source_surfaces_;
    StableSurfaceId source_next_stable_id_{};

    std::vector<Surface> surfaces_;
    StableSurfaceId next_stable_id_{};

    std::vector<Mesh> meshes_;
    StableMeshId next_mesh_id_{1U};

    std::vector<Snapshot> undo_;
    std::vector<Snapshot> redo_;

    std::uint64_t revision_{};
    std::uint64_t next_revision_{1U};
};

} // namespace dmc::rengine::hits::editor
