#pragma once

#include "dmc_rengine/formats/hits.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::hits::writer {

using StableSurfaceId = std::uint64_t;

enum class GridPolicy : std::uint8_t {
    preserve_source_grid = 0,
    fit_bounds_preserve_cell_size = 1,
};

enum class AssignmentPolicy : std::uint8_t {
    capcom_triangle_box_sat = 0,
};

struct Surface final {
    StableSurfaceId stable_id{};
    std::uint32_t flags{};
    formats::hits::Vec3 point_a;
    formats::hits::Vec3 point_b;
    formats::hits::Vec3 point_c;
};

struct RebuildOptions final {
    GridPolicy grid_policy{GridPolicy::preserve_source_grid};
    AssignmentPolicy assignment_policy{
        AssignmentPolicy::capcom_triangle_box_sat};
    float bounds_padding{};
    double overlap_epsilon{1.0e-7};
};

struct SurfaceLocation final {
    StableSurfaceId stable_id{};
    std::uint32_t triangle_index{};
    std::uint32_t triangle_byte_offset{};
    std::uint64_t file_offset{};
};

struct RebuildResult final {
    std::vector<std::byte> bytes;
    formats::hits::Header header;
    std::vector<SurfaceLocation> locations;
    std::vector<formats::ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class SpatialWriter final {
public:
    [[nodiscard]] static std::vector<Surface> surfaces_from_scan(
        const formats::hits::ScanResult& scan,
        StableSurfaceId first_stable_id = 1U);

    [[nodiscard]] static RebuildResult rebuild(
        const formats::hits::ScanResult& source_scan,
        std::span<const std::byte> source_bytes,
        std::span<const Surface> surfaces,
        RebuildOptions options = {});
};

} // namespace dmc::rengine::hits::writer
