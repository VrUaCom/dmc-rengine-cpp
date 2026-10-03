#pragma once

#include "dmc_rengine/hits/standalone_session.hpp"

#include <array>

namespace dmc::rengine::hits::viewport {

struct Point {
    float x{}, y{}, depth{};
};
struct Triangle {
    std::array<Point, 3> points;
    std::uint32_t color{}; // 0xRRGGBB
    editor::StableSurfaceId surface{};
    std::optional<std::size_t> scm_object;
    bool selected{};
};

// Platform-neutral product controller. All coordinates and SCM transforms are
// owned here; shells supply pointer positions and draw these triangle packets.
class Controller final {
  public:
    bool open_hits(std::span<const std::byte> bytes);
    bool open_scm(std::span<const std::byte> bytes);
    void fit();
    void orbit(float yaw_delta, float pitch_delta);
    void zoom(float factor);
    void pan(float horizontal, float vertical); // fraction of visible extent
    [[nodiscard]] std::vector<Triangle> frame(float width, float height) const;
    bool pick(float x, float y, float width, float height, bool scm_mode = false);
    bool select_connected();
    bool paint(editor::CollisionPreset preset);
    bool translate(formats::hits::Vec3 delta);
    bool import_selected_object(editor::CollisionPreset preset);
    bool boundary(formats::hits::Vec3 minimum, formats::hits::Vec3 maximum,
                  editor::CollisionPreset preset);
    bool undo();
    bool redo();
    [[nodiscard]] standalone::SaveResult save() const;
    [[nodiscard]] bool dirty() const;
    [[nodiscard]] std::string status() const;
    [[nodiscard]] std::span<const editor::StableSurfaceId> selection() const {
        return selection_;
    }
    [[nodiscard]] const standalone::Session* session() const {
        return session_ ? &*session_ : nullptr;
    }

  private:
    struct Overlay {
        std::array<formats::hits::Vec3, 3> points;
        std::size_t object{};
    };
    std::optional<standalone::Session> session_;
    std::optional<formats::scm::Document> scm_;
    std::vector<Overlay> overlay_;
    std::vector<editor::StableSurfaceId> selection_;
    std::optional<std::size_t> selected_object_;
    formats::hits::Vec3 center_{};
    float extent_{1.0F}, yaw_{0.5F}, pitch_{0.5F};
    float pan_x_{}, pan_y_{};
    void prune_selection();
};
} // namespace dmc::rengine::hits::viewport
