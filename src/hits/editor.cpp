#include "dmc_rengine/hits/editor.hpp"

#include "dmc_rengine/hits/edit.hpp"

#include <algorithm>
#include <limits>
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

Session::Snapshot Session::snapshot() const {
    return Snapshot{
        .surfaces = surfaces_,
        .next_stable_id = next_stable_id_,
        .revision = revision_,
    };
}

void Session::restore(Snapshot state) {
    surfaces_ = std::move(state.surfaces);
    next_stable_id_ = state.next_stable_id;
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

    std::vector<std::size_t> indices;
    indices.reserve(stable_ids.size());
    for (const auto stable_id : stable_ids) {
        const auto index = index_of(stable_id);
        if (!index) {
            return false;
        }
        indices.push_back(*index);
    }
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    std::vector<Surface> translated;
    translated.reserve(indices.size());
    for (const auto index : indices) {
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
    for (std::size_t i = 0U; i < indices.size(); ++i) {
        const auto& current = surfaces_[indices[i]];
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
    for (std::size_t i = 0U; i < indices.size(); ++i) {
        surfaces_[indices[i]] = translated[i];
    }
    return true;
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
