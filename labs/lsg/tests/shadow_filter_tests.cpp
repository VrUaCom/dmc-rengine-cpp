#include "rengine/lsg/shadow_quality.hpp"
#include "rengine/lsg/shadow_probe.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace rengine::lsg;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message); // Active in Release too.
}

// Analytic receiver z = 0.5 + 0.4*u - 0.2*v (within the 2 mm correction cap), sampled at shadow texel centers.
std::array<float, 25> plane(std::array<float, 2> fraction, float blocker) {
  std::array<float, 25> out{};
  for (int y = -2; y <= 2; ++y) for (int x = -2; x <= 2; ++x) {
    const auto i = static_cast<std::size_t>((y + 2) * 5 + x + 2);
    out[i] = 0.5f + (0.4f * (static_cast<float>(x) + 0.5f - fraction[0]) -
                     0.2f * (static_cast<float>(y) + 0.5f - fraction[1])) / 2048.0f - blocker;
  }
  return out;
}

float edge_visibility(float pixel) {
  const int center = static_cast<int>(std::floor(pixel));
  std::array<float, 25> depths{};
  for (int y = -2; y <= 2; ++y) for (int x = -2; x <= 2; ++x) {
    depths[static_cast<std::size_t>((y + 2) * 5 + x + 2)] =
        center + x >= 100 ? 0.8f : 0.2f;
  }
  return cinematic_shadow_visibility_reference(depths,
      {pixel - std::floor(pixel), 0.37f}, 0.5f, {}, 2048, 0.0001f, 0.001f);
}
}

int main() {
  ShadowProbe probe;
  require(!probe.active(), "probe must be opt-in");
  probe.start();
  probe.advance(100.0f);
  for (unsigned frame = 0; frame < 60; ++frame) {
    probe.advance(100.0f + static_cast<float>(frame) * 0.25f);
    require(probe.active() && probe.stage() == frame / 12u, "five ordered three-second views");
    require(probe.pose_time() == 100.0f, "capture animation must remain fixed");
  }
  probe.advance(115.0f);
  require(!probe.active(), "probe must finish after five views");
  probe.start();
  probe.advance(200.0f);
  probe.advance(900.0f);
  require(probe.active() && probe.stage() == 0u, "suspend must not skip views");
  probe.advance(std::numeric_limits<float>::quiet_NaN());
  probe.advance(800.0f);
  require(probe.stage() == 0u && probe.pose_time() == 200.0f, "invalid or backwards clock");
  probe.cancel();
  probe.advance(801.0f);
  require(!probe.active(), "cancel must remain cancelled");
  probe.start();
  probe.advance(1000.0f);
  require(probe.stage() == 0u && probe.pose_time() == 1000.0f, "restart captures new pose");
  std::cout << "SHADOW PROBE PASS: five views, fixed pose, completion, stall, cancellation, restart\n";

  // Derivatives of the same analytic plane in a rotated/skew screen basis.
  const auto gradient = receiver_depth_gradient(
      {0.003f, -0.001f, 0.0014f}, {0.002f, 0.004f, 0.0f});
  require(std::abs(gradient[0] - 0.4f) < 1e-5f &&
          std::abs(gradient[1] + 0.2f) < 1e-5f, "analytic plane gradient");
  // Camera derivative scale and mirrored UV orientation cannot change the field.
  const auto mirrored = receiver_depth_gradient(
      {-0.00003f, 0.00001f, -0.000014f}, {0.00002f, 0.00004f, 0.0f});
  require(std::abs(mirrored[0] - gradient[0]) < 1e-5f &&
          std::abs(mirrored[1] - gradient[1]) < 1e-5f, "derivative invariance");
  require(receiver_depth_gradient({1, 2, 3}, {2, 4, 6}) ==
          std::array<float, 2>{}, "singular Jacobian fallback");
  const float nan = std::numeric_limits<float>::quiet_NaN();
  require(receiver_depth_gradient({nan, 0, 0}, {0, 1, 0}) ==
          std::array<float, 2>{}, "nonfinite fallback");

  unsigned old_plane_failures = 0;
  for (int y = 0; y <= 10; ++y) for (int x = 0; x <= 10; ++x) {
    const std::array<float, 2> fraction{static_cast<float>(x) / 10.0f,
                                        static_cast<float>(y) / 10.0f};
    const auto depths = plane(fraction, 0.0f);
    const float corrected = cinematic_shadow_visibility_reference(
        depths, fraction, 0.5f, gradient, 2048, 0.00010f, 0.0020f / 2.2f);
    require(std::abs(corrected - 1.0f) < 1e-6f, "unoccluded slope self-shadowed");
    const float uncorrected = cinematic_shadow_visibility_reference(
        depths, fraction, 0.5f, {}, 2048, 0.00010f, 0.0020f / 2.2f);
    if (uncorrected < 0.99f) ++old_plane_failures;
    require(cinematic_shadow_visibility_reference(plane(fraction, 0.01f),
        fraction, 0.5f, gradient, 2048, 0.00010f, 0.0020f / 2.2f) == 0.0f,
        "real blocker leaked");
  }
  require(old_plane_failures > 100u, "regression fixture no longer exercises slope failure");
  // Continuous subtexel sweep across a fixed edge, including kernel recentering.
  float previous = edge_visibility(97.0f);
  float max_step = 0.0f;
  for (int i = 1; i <= 600; ++i) {
    const float current = edge_visibility(97.0f + static_cast<float>(i) * 0.01f);
    require(std::isfinite(current) && current >= 0.0f && current <= 1.0f,
            "invalid filtered visibility");
    require(current + 1e-6f >= previous, "edge response reversed");
    max_step = std::max(max_step, std::abs(current - previous));
    previous = current;
  }
  require(max_step < 0.005f, "texel-boundary pop");
  // Correction cap prevents arbitrarily distant receiver displacement at grazing angles.
  std::array<float, 25> blocker{};
  blocker.fill(0.49f);
  require(cinematic_shadow_visibility_reference(blocker, {0.5f, 0.5f},
      0.5f, {64.0f, -64.0f}, 2048, 0.0001f, 0.001f) == 0.0f, "correction cap leak");
  std::cout << "SHADOW FILTER PASS: analytic planes=121; uncorrected failures="
            << old_plane_failures << "; edge sweep=600; max visibility step="
            << max_step << "; blockers/degeneracy PASS\n";
}
