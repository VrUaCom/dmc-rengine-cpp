#include "dmc_rengine/hits/editor.hpp"

#include "dmc_rengine/hits/edit.hpp"
#include "dmc_rengine/hits/scm_import.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <unordered_map>
#include <utility>

namespace dmc::rengine::hits::editor {
namespace {

using formats::hits::Vec3;

[[nodiscard]] Vec3 add(const Vec3& left, const Vec3& right) noexcept {
    return Vec3{
        .x = left.x + right.x,
        .y = left.y + right.y,
        .z = left.z + right.z,
    };
}

[[nodiscard]] bool has_diagnostic(
    const writer::RebuildResult& result,
    const char* code) {
    return std::any_of(
        result.diagnostics.begin(),
        result.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

using PointKey = std::array<std::uint32_t, 3U>;

struct EdgeKey final {
    PointKey first;
    PointKey second;

    friend bool operator==(const EdgeKey&, const EdgeKey&) = default;
};

struct EdgeKeyHash final {
    [[nodiscard]] std::size_t operator()(
        const EdgeKey& edge) const noexcept {
        std::size_t value = 0xcbf29ce484222325ULL;
        const auto mix = [&value](std::uint32_t part) {
            value ^= static_cast<std::size_t>(part);
            value *= 0x100000001b3ULL;
        };
        for (const auto part : edge.first) {
            mix(part);
        }
        for (const auto part : edge.second) {
            mix(part);
        }
        return value;
    }
};

[[nodiscard]] PointKey point_key(const Vec3& point) noexcept {
    const auto canonical_bits = [](float value) noexcept {
        if (value == 0.0F) {
            value = 0.0F;
        }
        return std::bit_cast<std::uint32_t>(value);
    };
    return PointKey{
        canonical_bits(point.x),
        canonical_bits(point.y),
        canonical_bits(point.z),
    };
}

[[nodiscard]] EdgeKey edge_key(
    const Vec3& point_a,
    const Vec3& point_b) noexcept {
    auto first = point_key(point_a);
    auto second = point_key(point_b);
    if (second < first) {
        std::swap(first, second);
    }
    return EdgeKey{
        .first = first,
        .second = second,
    };
}

} // namespace

Session::Session(
    std::vector<std::byte> source_bytes,
    formats::hits::ScanResult source_scan,
    std::vector<Surface> surfaces,
    StableSurfaceId next_stable_id)
    : source_bytes_(std::move(source_bytes)),
      source_scan_(std::move(source_scan)),
      source_surfaces_(surfaces),
      source_next_stable_id_(next_stable_id),
      surfaces_(std::move(surfaces)),
      next_stable_id_(next_stable_id) {}

std::optional<Session> Session::open(
    std::span<const std::byte> source_bytes) {
    auto scan = formats::hits::RecordScanner::scan(source_bytes);
    if (!scan.ok()) {
        return std::nullopt;
    }

    auto surfaces = writer::SpatialWriter::surfaces_from_scan(scan, 1U);
    StableSurfaceId next_stable_id = 1U;
    if (!surfaces.empty()) {
        const auto last = surfaces.back().stable_id;
        next_stable_id =
            last == std::numeric_limits<StableSurfaceId>::max()
                ? StableSurfaceId{0U}
                : last + 1U;
    }

    return Session{
        std::vector<std::byte>{source_bytes.begin(), source_bytes.end()},
        std::move(scan),
        std::move(surfaces),
        next_stable_id,
    };
}

const formats::hits::ScanResult& Session::source_scan() const noexcept {
    return source_scan_;
}

std::span<const std::byte> Session::source_bytes() const noexcept {
    return std::span<const std::byte>{source_bytes_};
}

std::span<const Surface> Session::surfaces() const noexcept {
    return std::span<const Surface>{surfaces_};
}

std::span<const Mesh> Session::meshes() const noexcept {
    return std::span<const Mesh>{meshes_};
}

bool Session::dirty() const noexcept {
    return revision_ != 0U;
}

bool Session::can_undo() const noexcept {
    return !undo_.empty();
}

bool Session::can_redo() const noexcept {
    return !redo_.empty();
}

std::uint64_t Session::revision() const noexcept {
    return revision_;
}

std::optional<std::size_t> Session::index_of(
    StableSurfaceId stable_id) const noexcept {
    const auto found = std::find_if(
        surfaces_.begin(),
        surfaces_.end(),
        [stable_id](const Surface& surface) {
            return surface.stable_id == stable_id;
        });
    if (found == surfaces_.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(
        std::distance(surfaces_.begin(), found));
}

std::optional<std::size_t> Session::mesh_index_of(
    StableMeshId stable_id) const noexcept {
    const auto found = std::find_if(
        meshes_.begin(),
        meshes_.end(),
        [stable_id](const Mesh& mesh) {
            return mesh.stable_id == stable_id;
        });
    if (found == meshes_.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(
        std::distance(meshes_.begin(), found));
}

std::vector<StableSurfaceId> Session::connected_surface(
    StableSurfaceId seed,
    bool require_same_flags) const {
    const auto seed_index = index_of(seed);
    if (!seed_index) {
        return {};
    }

    std::unordered_map<
        EdgeKey,
        std::vector<std::size_t>,
        EdgeKeyHash> adjacency;
    adjacency.reserve(surfaces_.size() * 3U);

    const auto add_edge = [&adjacency](
                              const Vec3& point_a,
                              const Vec3& point_b,
                              std::size_t surface_index) {
        adjacency[edge_key(point_a, point_b)].push_back(surface_index);
    };

    for (std::size_t index = 0U; index < surfaces_.size(); ++index) {
        const auto& surface = surfaces_[index];
        add_edge(surface.point_a, surface.point_b, index);
        add_edge(surface.point_b, surface.point_c, index);
        add_edge(surface.point_c, surface.point_a, index);
    }

    const auto seed_flags = surfaces_[*seed_index].flags;
    std::vector<bool> visited(surfaces_.size(), false);
    std::vector<std::size_t> pending{*seed_index};
    std::vector<StableSurfaceId> result;

    while (!pending.empty()) {
        const auto current = pending.back();
        pending.pop_back();
        if (visited[current]) {
            continue;
        }
        visited[current] = true;

        const auto& surface = surfaces_[current];
        if (require_same_flags && surface.flags != seed_flags) {
            continue;
        }
        result.push_back(surface.stable_id);

        const std::array<EdgeKey, 3U> edges{
            edge_key(surface.point_a, surface.point_b),
            edge_key(surface.point_b, surface.point_c),
            edge_key(surface.point_c, surface.point_a),
        };
        for (const auto& edge : edges) {
            const auto found = adjacency.find(edge);
            if (found == adjacency.end()) {
                continue;
            }
            for (const auto neighbor : found->second) {
                if (!visited[neighbor] &&
                    (!require_same_flags ||
                     surfaces_[neighbor].flags == seed_flags)) {
                    pending.push_back(neighbor);
                }
            }
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::optional<std::vector<std::size_t>>
Session::resolve_surface_indices(
    std::span<const StableSurfaceId> stable_ids) const {
    std::vector<std::size_t> indices;
    indices.reserve(stable_ids.size());
    for (const auto stable_id : stable_ids) {
        const auto index = index_of(stable_id);
        if (!index) {
            return std::nullopt;
        }
        indices.push_back(*index);
    }
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    return indices;
}

Session::Snapshot Session::snapshot() const {
    return Snapshot{
        .surfaces = surfaces_,
        .next_stable_id = next_stable_id_,
        .meshes = meshes_,
        .next_mesh_id = next_mesh_id_,
        .revision = revision_,
    };
}

void Session::restore(Snapshot state) {
    surfaces_ = std::move(state.surfaces);
    next_stable_id_ = state.next_stable_id;
    meshes_ = std::move(state.meshes);
    next_mesh_id_ = state.next_mesh_id;
    revision_ = state.revision;
}

void Session::begin_mutation() {
    undo_.push_back(snapshot());
    redo_.clear();

    revision_ = next_revision_;
    if (next_revision_ != std::numeric_limits<std::uint64_t>::max()) {
        ++next_revision_;
    }
}

std::optional<StableSurfaceId> Session::allocate_stable_id() noexcept {
    if (next_stable_id_ == 0U) {
        return std::nullopt;
    }
    const auto allocated = next_stable_id_;
    if (next_stable_id_ == std::numeric_limits<StableSurfaceId>::max()) {
        next_stable_id_ = 0U;
    } else {
        ++next_stable_id_;
    }
    return allocated;
}

std::optional<StableMeshId> Session::allocate_mesh_id() noexcept {
    if (next_mesh_id_ == 0U) {
        return std::nullopt;
    }
    const auto allocated = next_mesh_id_;
    if (next_mesh_id_ == std::numeric_limits<StableMeshId>::max()) {
        next_mesh_id_ = 0U;
    } else {
        ++next_mesh_id_;
    }
    return allocated;
}

std::optional<StableMeshId> Session::append_triangle_mesh(
    std::span<const std::array<Vec3, 3U>> triangles,
    std::uint32_t flags) {
    if (triangles.empty() ||
        next_stable_id_ == 0U ||
        next_mesh_id_ == 0U) {
        return std::nullopt;
    }

    for (const auto& triangle : triangles) {
        if (!edit::recompute_geometry(
                triangle[0],
                triangle[1],
                triangle[2])) {
            return std::nullopt;
        }
    }

    const auto available_ids =
        std::numeric_limits<StableSurfaceId>::max() -
        next_stable_id_ +
        StableSurfaceId{1U};
    if (static_cast<std::uint64_t>(triangles.size()) > available_ids) {
        return std::nullopt;
    }

    begin_mutation();
    const auto mesh_id = allocate_mesh_id();
    if (!mesh_id) {
        return std::nullopt;
    }

    std::vector<StableSurfaceId> members;
    members.reserve(triangles.size());
    for (const auto& triangle : triangles) {
        const auto stable_id = allocate_stable_id();
        if (!stable_id) {
            return std::nullopt;
        }
        members.push_back(*stable_id);
        surfaces_.push_back(Surface{
            .stable_id = *stable_id,
            .flags = flags,
            .point_a = triangle[0],
            .point_b = triangle[1],
            .point_c = triangle[2],
        });
    }

    meshes_.push_back(Mesh{
        .stable_id = *mesh_id,
        .surface_ids = std::move(members),
    });
    return mesh_id;
}

std::optional<StableSurfaceId> Session::add_surface(
    std::uint32_t flags,
    const Vec3& point_a,
    const Vec3& point_b,
    const Vec3& point_c) {
    if (!edit::recompute_geometry(point_a, point_b, point_c) ||
        next_stable_id_ == 0U) {
        return std::nullopt;
    }

    begin_mutation();
    const auto stable_id = allocate_stable_id();
    if (!stable_id) {
        return std::nullopt;
    }

    surfaces_.push_back(Surface{
        .stable_id = *stable_id,
        .flags = flags,
        .point_a = point_a,
        .point_b = point_b,
        .point_c = point_c,
    });
    return stable_id;
}

std::optional<StableSurfaceId> Session::duplicate_surface(
    StableSurfaceId stable_id) {
    const auto index = index_of(stable_id);
    if (!index || next_stable_id_ == 0U) {
        return std::nullopt;
    }

    const auto original = surfaces_[*index];
    begin_mutation();
    const auto duplicate_id = allocate_stable_id();
    if (!duplicate_id) {
        return std::nullopt;
    }

    auto duplicate = original;
    duplicate.stable_id = *duplicate_id;
    surfaces_.push_back(duplicate);

    for (auto& mesh : meshes_) {
        if (std::find(
                mesh.surface_ids.begin(),
                mesh.surface_ids.end(),
                stable_id) != mesh.surface_ids.end()) {
            mesh.surface_ids.push_back(*duplicate_id);
        }
    }
    return duplicate_id;
}

bool Session::erase_surface(StableSurfaceId stable_id) {
    const auto index = index_of(stable_id);
    if (!index) {
        return false;
    }

    begin_mutation();
    surfaces_.erase(
        surfaces_.begin() + static_cast<std::ptrdiff_t>(*index));

    for (auto& mesh : meshes_) {
        mesh.surface_ids.erase(
            std::remove(
                mesh.surface_ids.begin(),
                mesh.surface_ids.end(),
                stable_id),
            mesh.surface_ids.end());
    }
    meshes_.erase(
        std::remove_if(
            meshes_.begin(),
            meshes_.end(),
            [](const Mesh& mesh) {
                return mesh.surface_ids.empty();
            }),
        meshes_.end());
    return true;
}

bool Session::set_flags(
    StableSurfaceId stable_id,
    std::uint32_t flags) {
    const auto index = index_of(stable_id);
    if (!index) {
        return false;
    }
    if (surfaces_[*index].flags == flags) {
        return true;
    }

    begin_mutation();
    surfaces_[*index].flags = flags;
    return true;
}

bool Session::set_flags(
    std::span<const StableSurfaceId> stable_ids,
    std::uint32_t flags) {
    const auto indices = resolve_surface_indices(stable_ids);
    if (!indices) {
        return false;
    }
    if (indices->empty()) {
        return true;
    }

    const auto changed = std::any_of(
        indices->begin(),
        indices->end(),
        [this, flags](std::size_t index) {
            return surfaces_[index].flags != flags;
        });
    if (!changed) {
        return true;
    }

    begin_mutation();
    for (const auto index : *indices) {
        surfaces_[index].flags = flags;
    }
    return true;
}

bool Session::set_collision_preset(
    StableSurfaceId stable_id,
    CollisionPreset preset) {
    return set_flags(
        stable_id,
        collision_preset_info(preset).raw_flags);
}

bool Session::set_collision_preset(
    std::span<const StableSurfaceId> stable_ids,
    CollisionPreset preset) {
    return set_flags(
        stable_ids,
        collision_preset_info(preset).raw_flags);
}

bool Session::set_geometry(
    StableSurfaceId stable_id,
    const Vec3& point_a,
    const Vec3& point_b,
    const Vec3& point_c) {
    const auto index = index_of(stable_id);
    if (!index || !edit::recompute_geometry(point_a, point_b, point_c)) {
        return false;
    }

    const auto& current = surfaces_[*index];
    if (current.point_a == point_a &&
        current.point_b == point_b &&
        current.point_c == point_c) {
        return true;
    }

    begin_mutation();
    auto& surface = surfaces_[*index];
    surface.point_a = point_a;
    surface.point_b = point_b;
    surface.point_c = point_c;
    return true;
}

bool Session::translate_surface(
    StableSurfaceId stable_id,
    const Vec3& delta) {
    const auto index = index_of(stable_id);
    if (!index || !edit::finite(delta)) {
        return false;
    }

    const auto surface = surfaces_[*index];
    return set_geometry(
        stable_id,
        add(surface.point_a, delta),
        add(surface.point_b, delta),
        add(surface.point_c, delta));
}

bool Session::translate_surfaces(
    std::span<const StableSurfaceId> stable_ids,
    const Vec3& delta) {
    if (!edit::finite(delta)) {
        return false;
    }
    if (stable_ids.empty()) {
        return true;
    }

    const auto indices = resolve_surface_indices(stable_ids);
    if (!indices) {
        return false;
    }

    std::vector<Surface> translated;
    translated.reserve(indices->size());
    for (const auto index : *indices) {
        auto surface = surfaces_[index];
        surface.point_a = add(surface.point_a, delta);
        surface.point_b = add(surface.point_b, delta);
        surface.point_c = add(surface.point_c, delta);
        if (!edit::recompute_geometry(
                surface.point_a,
                surface.point_b,
                surface.point_c)) {
            return false;
        }
        translated.push_back(surface);
    }

    bool changed = false;
    for (std::size_t i = 0U; i < indices->size(); ++i) {
        const auto& current = surfaces_[(*indices)[i]];
        const auto& candidate = translated[i];
        if (current.point_a != candidate.point_a ||
            current.point_b != candidate.point_b ||
            current.point_c != candidate.point_c) {
            changed = true;
            break;
        }
    }
    if (!changed) {
        return true;
    }

    begin_mutation();
    for (std::size_t i = 0U; i < indices->size(); ++i) {
        surfaces_[(*indices)[i]] = translated[i];
    }
    return true;
}

std::optional<StableMeshId> Session::create_mesh(
    std::span<const StableSurfaceId> stable_ids) {
    if (stable_ids.empty() || next_mesh_id_ == 0U) {
        return std::nullopt;
    }

    const auto indices = resolve_surface_indices(stable_ids);
    if (!indices || indices->empty()) {
        return std::nullopt;
    }

    std::vector<StableSurfaceId> members;
    members.reserve(indices->size());
    for (const auto index : *indices) {
        members.push_back(surfaces_[index].stable_id);
    }
    std::sort(members.begin(), members.end());

    for (const auto& mesh : meshes_) {
        for (const auto member : members) {
            if (std::find(
                    mesh.surface_ids.begin(),
                    mesh.surface_ids.end(),
                    member) != mesh.surface_ids.end()) {
                return std::nullopt;
            }
        }
    }

    begin_mutation();
    const auto mesh_id = allocate_mesh_id();
    if (!mesh_id) {
        return std::nullopt;
    }

    meshes_.push_back(Mesh{
        .stable_id = *mesh_id,
        .surface_ids = std::move(members),
    });
    return mesh_id;
}

std::optional<StableMeshId> Session::create_connected_mesh(
    StableSurfaceId seed,
    bool require_same_flags) {
    const auto members =
        connected_surface(seed, require_same_flags);
    if (members.empty()) {
        return std::nullopt;
    }
    return create_mesh(members);
}

std::optional<StableMeshId> Session::merge_meshes(
    std::span<const StableMeshId> mesh_ids) {
    if (mesh_ids.empty()) {
        return std::nullopt;
    }

    std::vector<std::size_t> indices;
    indices.reserve(mesh_ids.size());
    for (const auto mesh_id : mesh_ids) {
        const auto index = mesh_index_of(mesh_id);
        if (!index) {
            return std::nullopt;
        }
        indices.push_back(*index);
    }
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    if (indices.size() == 1U) {
        return meshes_[indices.front()].stable_id;
    }

    const auto destination_id = meshes_[indices.front()].stable_id;
    std::vector<StableMeshId> remove_ids;
    std::vector<StableSurfaceId> merged;
    for (const auto index : indices) {
        const auto& mesh = meshes_[index];
        if (mesh.stable_id != destination_id) {
            remove_ids.push_back(mesh.stable_id);
        }
        merged.insert(
            merged.end(),
            mesh.surface_ids.begin(),
            mesh.surface_ids.end());
    }
    std::sort(merged.begin(), merged.end());
    merged.erase(std::unique(merged.begin(), merged.end()), merged.end());

    begin_mutation();

    const auto destination = mesh_index_of(destination_id);
    if (!destination) {
        return std::nullopt;
    }
    meshes_[*destination].surface_ids = std::move(merged);

    meshes_.erase(
        std::remove_if(
            meshes_.begin(),
            meshes_.end(),
            [&remove_ids](const Mesh& mesh) {
                return std::find(
                    remove_ids.begin(),
                    remove_ids.end(),
                    mesh.stable_id) != remove_ids.end();
            }),
        meshes_.end());

    return destination_id;
}

bool Session::erase_mesh(StableMeshId mesh_id) {
    const auto index = mesh_index_of(mesh_id);
    if (!index) {
        return false;
    }

    begin_mutation();
    meshes_.erase(
        meshes_.begin() + static_cast<std::ptrdiff_t>(*index));
    return true;
}

bool Session::set_mesh_collision_preset(
    StableMeshId mesh_id,
    CollisionPreset preset) {
    const auto index = mesh_index_of(mesh_id);
    if (!index) {
        return false;
    }
    const auto members = meshes_[*index].surface_ids;
    return set_collision_preset(members, preset);
}

bool Session::translate_mesh(
    StableMeshId mesh_id,
    const Vec3& delta) {
    const auto index = mesh_index_of(mesh_id);
    if (!index) {
        return false;
    }
    const auto members = meshes_[*index].surface_ids;
    return translate_surfaces(members, delta);
}

std::optional<ScmImportResult> Session::import_scm_mesh(
    const formats::scm::Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    CollisionPreset preset) {
    const auto extracted =
        scm_import::extract_mesh(document, object_index, mesh_index);
    if (!extracted || extracted->triangles.empty()) {
        return std::nullopt;
    }

    std::vector<std::array<Vec3, 3U>> triangles;
    triangles.reserve(extracted->triangles.size());
    for (const auto& triangle : extracted->triangles) {
        triangles.push_back({
            triangle.point_a,
            triangle.point_b,
            triangle.point_c,
        });
    }

    const auto mesh_id = append_triangle_mesh(
        triangles,
        collision_preset_info(preset).raw_flags);
    if (!mesh_id) {
        return std::nullopt;
    }

    const auto mesh_index_in_editor = mesh_index_of(*mesh_id);
    if (!mesh_index_in_editor) {
        return std::nullopt;
    }

    return ScmImportResult{
        .mesh_id = *mesh_id,
        .surface_ids = meshes_[*mesh_index_in_editor].surface_ids,
        .source_object_index = extracted->object_index,
        .source_mesh_index = extracted->mesh_index,
        .source_node_index = extracted->node_index,
    };
}

std::optional<ScmObjectImportResult> Session::import_scm_object(
    const formats::scm::Document& document,
    std::size_t object_index,
    CollisionPreset preset) {
    const auto extracted =
        scm_import::extract_object(document, object_index);
    if (!extracted || extracted->triangles.empty()) {
        return std::nullopt;
    }

    std::vector<std::array<Vec3, 3U>> triangles;
    triangles.reserve(extracted->triangles.size());
    for (const auto& triangle : extracted->triangles) {
        triangles.push_back({
            triangle.point_a,
            triangle.point_b,
            triangle.point_c,
        });
    }

    const auto mesh_id = append_triangle_mesh(
        triangles,
        collision_preset_info(preset).raw_flags);
    if (!mesh_id) {
        return std::nullopt;
    }

    const auto editor_mesh_index = mesh_index_of(*mesh_id);
    if (!editor_mesh_index) {
        return std::nullopt;
    }

    return ScmObjectImportResult{
        .mesh_id = *mesh_id,
        .surface_ids = meshes_[*editor_mesh_index].surface_ids,
        .source_object_index = extracted->object_index,
        .source_node_index = extracted->node_index,
        .source_mesh_count = extracted->source_mesh_count,
    };
}

std::optional<StableMeshId> Session::add_quad(
    const Vec3& point_a,
    const Vec3& point_b,
    const Vec3& point_c,
    const Vec3& point_d,
    CollisionPreset preset) {
    const std::array<std::array<Vec3, 3U>, 2U> triangles{{
        {point_a, point_b, point_c},
        {point_a, point_c, point_d},
    }};
    return append_triangle_mesh(
        triangles,
        collision_preset_info(preset).raw_flags);
}

std::optional<StableMeshId> Session::create_rectangular_boundary(
    const Vec3& minimum,
    const Vec3& maximum,
    CollisionPreset preset) {
    if (!edit::finite(minimum) ||
        !edit::finite(maximum) ||
        minimum.x >= maximum.x ||
        minimum.y >= maximum.y ||
        minimum.z >= maximum.z) {
        return std::nullopt;
    }

    const auto x0 = minimum.x;
    const auto y0 = minimum.y;
    const auto z0 = minimum.z;
    const auto x1 = maximum.x;
    const auto y1 = maximum.y;
    const auto z1 = maximum.z;

    // Winding points normals toward the playable volume.
    const std::array<std::array<Vec3, 3U>, 8U> triangles{{
        {Vec3{x0, y0, z0}, Vec3{x1, y0, z0}, Vec3{x1, y1, z0}},
        {Vec3{x0, y0, z0}, Vec3{x1, y1, z0}, Vec3{x0, y1, z0}},

        {Vec3{x1, y0, z0}, Vec3{x1, y0, z1}, Vec3{x1, y1, z1}},
        {Vec3{x1, y0, z0}, Vec3{x1, y1, z1}, Vec3{x1, y1, z0}},

        {Vec3{x1, y0, z1}, Vec3{x0, y0, z1}, Vec3{x0, y1, z1}},
        {Vec3{x1, y0, z1}, Vec3{x0, y1, z1}, Vec3{x1, y1, z1}},

        {Vec3{x0, y0, z1}, Vec3{x0, y0, z0}, Vec3{x0, y1, z0}},
        {Vec3{x0, y0, z1}, Vec3{x0, y1, z0}, Vec3{x0, y1, z1}},
    }};

    return append_triangle_mesh(
        triangles,
        collision_preset_info(preset).raw_flags);
}

bool Session::undo() {
    if (undo_.empty()) {
        return false;
    }

    redo_.push_back(snapshot());
    auto state = std::move(undo_.back());
    undo_.pop_back();
    restore(std::move(state));
    return true;
}

bool Session::redo() {
    if (redo_.empty()) {
        return false;
    }

    undo_.push_back(snapshot());
    auto state = std::move(redo_.back());
    redo_.pop_back();
    restore(std::move(state));
    return true;
}

bool Session::reset_to_source() {
    if (!dirty()) {
        return true;
    }

    undo_.push_back(snapshot());
    redo_.clear();
    surfaces_ = source_surfaces_;
    next_stable_id_ = source_next_stable_id_;
    meshes_.clear();
    next_mesh_id_ = 1U;
    revision_ = 0U;
    return true;
}

writer::RebuildResult Session::rebuild() const {
    auto options = writer::RebuildOptions{};
    auto result = writer::SpatialWriter::rebuild(
        source_scan_,
        source_bytes_,
        surfaces_,
        options);
    if (result.ok() ||
        surfaces_.empty() ||
        !has_diagnostic(
            result,
            "hits.writer.surface_outside_preserved_grid")) {
        return result;
    }

    options.grid_policy =
        writer::GridPolicy::fit_bounds_preserve_cell_size;
    return writer::SpatialWriter::rebuild(
        source_scan_,
        source_bytes_,
        surfaces_,
        options);
}

writer::RebuildResult Session::rebuild(
    writer::RebuildOptions options) const {
    return writer::SpatialWriter::rebuild(
        source_scan_,
        source_bytes_,
        surfaces_,
        options);
}

} // namespace dmc::rengine::hits::editor
