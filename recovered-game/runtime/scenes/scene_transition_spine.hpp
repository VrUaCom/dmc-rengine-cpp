#pragma once

#include <cstdint>
#include <memory>
#include <optional>

namespace dmc::recovered::dmc3::runtime::scenes {

enum class SceneId : std::uint32_t {
    boot = 0U,
    opening = 1U,
    start_menu = 2U,
    mission_select = 3U,
    game = 4U,
    game_main = 5U,
    demo = 6U,
    mission_start = 7U,
    result = 8U,
    ending = 9U,
};

class IScene {
public:
    virtual ~IScene() = default;
    virtual void enter() = 0;
    virtual void exit() = 0;
};

class ISceneFactory {
public:
    virtual ~ISceneFactory() = default;
    [[nodiscard]] virtual std::unique_ptr<IScene> create(SceneId id) = 0;
};

enum class SceneTransitionStatus {
    applied,
    replacement_unavailable,
};

// Executable reconstruction of the directly observed successful transition
// spine: exit current -> destroy current -> factory create -> enter replacement.
//
// This does NOT claim the exact original scene-manager object layout, update
// result codes, routing table, or failure recovery semantics. A null factory
// result is merely reported by this harness; no claim is made that the original
// game can or does take that path.
class SceneTransitionSpine final {
public:
    explicit SceneTransitionSpine(ISceneFactory& factory) noexcept
        : factory_{factory} {}

    [[nodiscard]] std::optional<SceneId> current_id() const noexcept {
        return current_id_;
    }

    [[nodiscard]] bool has_scene() const noexcept {
        return current_ != nullptr;
    }

    [[nodiscard]] SceneTransitionStatus transition_to(SceneId next) {
        if (current_ != nullptr) {
            current_->exit();
            current_.reset();
            current_id_.reset();
        }

        auto replacement = factory_.create(next);
        if (replacement == nullptr) {
            return SceneTransitionStatus::replacement_unavailable;
        }

        replacement->enter();
        current_ = std::move(replacement);
        current_id_ = next;
        return SceneTransitionStatus::applied;
    }

private:
    ISceneFactory& factory_;
    std::unique_ptr<IScene> current_;
    std::optional<SceneId> current_id_;
};

} // namespace dmc::recovered::dmc3::runtime::scenes
