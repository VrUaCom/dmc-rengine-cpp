#include "dmc_rengine/hits/viewport.hpp"

#include "dmc_rengine/hits/scm_import.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

namespace dmc::rengine::hits::viewport {
namespace {
std::uint32_t color(std::uint32_t flags) {
    switch (flags) {
    case 1:
        return 0x4285F4;
    case 9:
        return 0xFF9933;
    case 10:
        return 0x40CB78;
    case 0x18060001:
        return 0xEF5350;
    default:
        return 0xAB78CA;
    }
}
bool finite(formats::hits::Vec3 p) {
    return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
}
std::optional<float> depth_at(const Triangle& t, float x, float y) {
    const auto& a = t.points[0];
    const auto& b = t.points[1];
    const auto& c = t.points[2];
    const float determinant = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
    if (std::abs(determinant) < 1.0e-6F)
        return std::nullopt;
    const float u = ((b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y)) / determinant;
    const float v = ((c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y)) / determinant;
    const float w = 1 - u - v;
    if (u < -1.0e-5F || v < -1.0e-5F || w < -1.0e-5F)
        return std::nullopt;
    return u * a.depth + v * b.depth + w * c.depth;
}
} // namespace
bool Controller::open_hits(std::span<const std::byte> bytes) {
    auto opened = standalone::Session::open(bytes);
    if (!opened)
        return false;
    session_ = std::move(opened);
    selection_.clear();
    fit();
    return true;
}
bool Controller::open_scm(std::span<const std::byte> bytes) {
    auto parsed = formats::scm::Parser::parse(bytes);
    if (!parsed.ok())
        return false;
    std::vector<Overlay> triangles;
    for (std::size_t i = 0; i < parsed.document.objects.size(); ++i) {
        auto object = scm_import::extract_object(parsed.document, i);
        if (!object)
            return false; // Do not publish a partial scene.
        for (const auto& t : object->triangles)
            triangles.push_back({{t.point_a, t.point_b, t.point_c}, i});
    }
    scm_ = std::move(parsed.document);
    overlay_ = std::move(triangles);
    selected_object_.reset();
    fit();
    return true;
}
void Controller::fit() {
    formats::hits::Vec3 lo{INFINITY, INFINITY, INFINITY}, hi{-INFINITY, -INFINITY, -INFINITY};
    bool any = false;
    const auto add = [&](formats::hits::Vec3 p) {
        if (!finite(p))
            return;
        any = true;
        lo.x = std::min(lo.x, p.x);
        lo.y = std::min(lo.y, p.y);
        lo.z = std::min(lo.z, p.z);
        hi.x = std::max(hi.x, p.x);
        hi.y = std::max(hi.y, p.y);
        hi.z = std::max(hi.z, p.z);
    };
    if (session_)
        for (const auto& s : session_->editor().surfaces()) {
            add(s.point_a);
            add(s.point_b);
            add(s.point_c);
        }
    for (const auto& t : overlay_)
        for (auto p : t.points)
            add(p);
    if (any) {
        center_ = {lo.x / 2 + hi.x / 2, lo.y / 2 + hi.y / 2, lo.z / 2 + hi.z / 2};
        extent_ = std::max(0.001F, std::hypot(hi.x - lo.x, hi.y - lo.y, hi.z - lo.z) * 0.65F);
    } else {
        center_ = {};
        extent_ = 1;
    }
    pan_x_ = pan_y_ = 0;
}
void Controller::orbit(float y, float p) {
    if (!std::isfinite(y) || !std::isfinite(p))
        return;
    yaw_ = std::remainder(yaw_ + y, 6.2831853F);
    pitch_ = std::clamp(pitch_ + p, -1.55F, 1.55F);
}
void Controller::zoom(float factor) {
    if (std::isfinite(factor) && factor > 0)
        extent_ = std::clamp(extent_ * factor, 0.001F, 1.0e12F);
}
void Controller::pan(float x, float y) {
    if (std::isfinite(x) && std::isfinite(y)) {
        pan_x_ += x;
        pan_y_ += y;
    }
}
std::vector<Triangle> Controller::frame(float width, float height) const {
    std::vector<Triangle> output;
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0)
        return output;
    const float scale = std::min(width, height) / (2 * extent_);
    const float cy = std::cos(yaw_), sy = std::sin(yaw_), cp = std::cos(pitch_),
                sp = std::sin(pitch_);
    const auto project = [&](formats::hits::Vec3 p) {
        const float x = p.x - center_.x, y = p.y - center_.y, z = p.z - center_.z;
        const float rx = cy * x - sy * z, rz = sy * x + cy * z;
        return Point{width / 2 + (rx + pan_x_ * extent_) * scale,
                     height / 2 - (cp * y - sp * rz + pan_y_ * extent_) * scale, sp * y + cp * rz};
    };
    const auto append = [&](std::array<formats::hits::Vec3, 3> p, std::uint32_t rgb,
                            editor::StableSurfaceId id, std::optional<std::size_t> object,
                            bool selected) {
        if (!finite(p[0]) || !finite(p[1]) || !finite(p[2]))
            return;
        output.push_back(
            {{project(p[0]), project(p[1]), project(p[2])}, rgb, id, object, selected});
    };
    for (const auto& t : overlay_)
        append(t.points, 0x616E80, 0, t.object, selected_object_ == t.object);
    if (session_)
        for (const auto& s : session_->editor().surfaces())
            append({s.point_a, s.point_b, s.point_c}, color(s.flags), s.stable_id, std::nullopt,
                   std::find(selection_.begin(), selection_.end(), s.stable_id) !=
                       selection_.end());
    // Painter ordering is a preview approximation; picking uses interpolated
    // depth, not average triangle depth. Intersecting triangles need a z-buffer.
    std::stable_sort(output.begin(), output.end(), [](const auto& a, const auto& b) {
        return a.points[0].depth + a.points[1].depth + a.points[2].depth >
               b.points[0].depth + b.points[1].depth + b.points[2].depth;
    });
    return output;
}
bool Controller::pick(float x, float y, float w, float h, bool scm_mode) {
    if (!std::isfinite(x) || !std::isfinite(y))
        return false;
    auto triangles = frame(w, h);
    const Triangle* best = nullptr;
    float depth = INFINITY;
    for (const auto& t : triangles) {
        if (scm_mode != t.scm_object.has_value())
            continue;
        const auto d = depth_at(t, x, y);
        if (d && *d <= depth) {
            best = &t;
            depth = *d;
        }
    }
    if (scm_mode)
        selected_object_ = best ? best->scm_object : std::nullopt;
    else {
        selection_.clear();
        if (best)
            selection_.push_back(best->surface);
    }
    return best != nullptr;
}
bool Controller::select_connected() {
    if (!session_ || selection_.empty())
        return false;
    selection_ = session_->editor().connected_surface(selection_.front());
    return !selection_.empty();
}
bool Controller::paint(editor::CollisionPreset preset) {
    return session_ && !selection_.empty() &&
           session_->editor().set_collision_preset(selection_, preset);
}
bool Controller::translate(formats::hits::Vec3 delta) {
    return session_ && !selection_.empty() && finite(delta) &&
           session_->editor().translate_surfaces(selection_, delta);
}
bool Controller::import_selected_object(editor::CollisionPreset preset) {
    if (!session_ || !scm_ || !selected_object_)
        return false;
    auto imported = session_->editor().import_scm_object(*scm_, *selected_object_, preset);
    if (!imported)
        return false;
    selection_ = std::move(imported->surface_ids);
    return true;
}
bool Controller::boundary(formats::hits::Vec3 lo, formats::hits::Vec3 hi,
                          editor::CollisionPreset preset) {
    return session_ && finite(lo) && finite(hi) &&
           session_->editor().create_rectangular_boundary(lo, hi, preset).has_value();
}
void Controller::prune_selection() {
    if (!session_) {
        selection_.clear();
        return;
    }
    std::erase_if(selection_, [&](auto id) { return !session_->editor().index_of(id); });
}
bool Controller::undo() {
    if (!session_ || !session_->editor().undo())
        return false;
    prune_selection();
    return true;
}
bool Controller::redo() {
    if (!session_ || !session_->editor().redo())
        return false;
    prune_selection();
    return true;
}
standalone::SaveResult Controller::save() const {
    if (!session_)
        return {standalone::SaveStatus::rebuild_failed, {}, "Open HITS first."};
    return session_->save();
}
bool Controller::dirty() const {
    return session_ && session_->dirty();
}
std::string Controller::status() const {
    std::ostringstream s;
    s << "HITS " << (session_ ? session_->editor().surfaces().size() : 0) << " | Selected "
      << selection_.size();
    if (session_ && !selection_.empty())
        if (auto v = session_->editor().inspect_surface(selection_.front()))
            s << " | flags 0x" << std::hex << v->raw_flags << std::dec;
    s << " | SCM " << overlay_.size();
    if (selected_object_)
        s << " | Object " << *selected_object_;
    if (dirty())
        s << " | Modified";
    return s.str();
}
} // namespace dmc::rengine::hits::viewport
