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

enum class SceneTransitionResult {
    applied,
    factory_failed,
};

// Recovered scene transition spine. The evidenced transition path exits the
// current scene, destroys it, creates the requested replacement and then enters
// the new scene. The same manager shape can be used for the root application
// flow and the nested gameplay scene flow; exact update-result code semantics
// remain a separate reverse target.
class SceneManager final {
public:
    explicit SceneManager(ISceneFactory& factory) noexcept
        : factory_{factory} {}

    [[nodiscard]] bool has_scene() const noexcept {
        return current_ != nullptr;
    }

    [[nodiscard]] std::optional<SceneId> current_id() const noexcept {
        return current_id_;
    }

    [[nodiscard]] SceneTransitionResult transition_to(SceneId next) {
        if (current_ != nullptr) {
            current_->exit();
            current_.reset();
            current_id_.reset();
        }

        auto replacement = factory_.create(next);
        if (replacement == nullptr) {
            return SceneTransitionResult::factory_failed;
        }

        replacement->enter();
        current_ = std::move(replacement);
        current_id_ = next;
        return SceneTransitionResult::applied;
    }

private:
    ISceneFactory& factory_;
    std::unique_ptr<IScene> current_;
    std::optional<SceneId> current_id_;
};

} // namespace dmc::recovered::dmc3::runtime::scenes
