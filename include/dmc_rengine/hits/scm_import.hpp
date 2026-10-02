#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/hits.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace dmc::rengine::hits::scm_import {

struct TriangleSeed final {
    formats::hits::Vec3 point_a;
    formats::hits::Vec3 point_b;
    formats::hits::Vec3 point_c;
    std::size_t source_triangle_ordinal{};
};

struct MeshExtraction final {
    std::size_t object_index{};
    std::size_t mesh_index{};
    std::size_t node_index{};
    std::vector<TriangleSeed> triangles;
};

[[nodiscard]] std::optional<MeshExtraction> extract_mesh(
    const formats::scm::Document& document,
    std::size_t object_index,
    std::size_t mesh_index) noexcept;

} // namespace dmc::rengine::hits::scm_import
