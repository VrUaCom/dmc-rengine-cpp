#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rengine::lsg {
// Platform-independent capture clock. Long stalls cannot skip diagnostic views.
class ShadowProbe {
public:
  void start() noexcept { active_ = true; started_ = false; elapsed_ = 0.0f; }
  void cancel() noexcept { active_ = false; }
  void advance(float now) noexcept {
    if (!active_ || !std::isfinite(now)) return;
    if (!started_) { started_ = true; pose_ = previous_ = now; return; }
    elapsed_ += std::clamp(now - previous_, 0.0f, 0.25f);
    previous_ = now;
    if (elapsed_ >= 15.0f) active_ = false;
  }
  [[nodiscard]] bool active() const noexcept { return active_; }
  [[nodiscard]] std::uint32_t stage() const noexcept {
    return std::min(static_cast<std::uint32_t>(elapsed_ / 3.0f), 4u);
  }
  [[nodiscard]] float pose_time() const noexcept { return pose_; }
private:
  bool active_{}, started_{};
  float elapsed_{}, pose_{}, previous_{};
};
} // namespace rengine::lsg
