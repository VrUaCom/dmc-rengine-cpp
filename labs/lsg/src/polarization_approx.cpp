#include "rengine/lsg/polarization_approx.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {

float rayleigh_dolp_from_mu(float mu) noexcept {
  if (!std::isfinite(mu)) return 0.0f;
  mu = std::clamp(mu, -1.0f, 1.0f);
  const float mu2 = mu * mu;
  const float denom = 1.0f + mu2;
  return denom > 0.0f ? std::clamp((1.0f - mu2) / denom, 0.0f, 1.0f) : 0.0f;
}

float polarized_attenuation(float dolp,
                            float axis_alignment_sq,
                            float strength) noexcept {
  if (!std::isfinite(dolp) || !std::isfinite(axis_alignment_sq) ||
      !std::isfinite(strength)) {
    return 1.0f;
  }

  dolp = std::clamp(dolp, 0.0f, 1.0f);
  axis_alignment_sq = std::clamp(axis_alignment_sq, 0.0f, 1.0f);
  strength = std::clamp(strength, 0.0f, 1.0f);

  return std::clamp(
      1.0f - 0.55f * strength * dolp * (1.0f - axis_alignment_sq),
      0.45f, 1.0f);
}

}  // namespace rengine::lsg
