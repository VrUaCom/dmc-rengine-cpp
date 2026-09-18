#pragma once

namespace rengine::lsg {

[[nodiscard]] float rayleigh_dolp_from_mu(float mu) noexcept;

[[nodiscard]] float polarized_attenuation(float dolp,
                                          float axis_alignment_sq,
                                          float strength) noexcept;

}  // namespace rengine::lsg
