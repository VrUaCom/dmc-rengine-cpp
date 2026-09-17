#pragma once

#include <algorithm>
#include <cstdint>

namespace rengine::lsg {

struct Extent2u {
  std::uint32_t width{1};
  std::uint32_t height{1};
};

// Camera projection and Vulkan surface pre-rotation are separate concerns.
//
// Android's NativeWindow / surface extent used by this runtime already reflects the current
// user-visible viewport after Activity recreation on orientation changes. currentTransform is
// still required to pre-rotate clip-space for presentation, but applying a second width/height
// swap to the projection aspect in ROTATE_90/ROTATE_270 inverts a landscape viewport
// (e.g. 2340x1080 -> 1080x2340) and stretches geometry horizontally by ~4.69x.
//
// Keep the historical function name for the v0 renderer ABI, but deliberately preserve the
// supplied extent for camera projection. rotation_code is accepted only to make the contract
// explicit and to prevent callers from coupling projection dimensions to Vulkan pre-rotation.
[[nodiscard]] constexpr Extent2u logical_extent_for_surface_rotation(
    Extent2u viewport_extent, std::uint32_t rotation_code) noexcept {
  (void)rotation_code;
  viewport_extent.width = std::max(1u, viewport_extent.width);
  viewport_extent.height = std::max(1u, viewport_extent.height);
  return viewport_extent;
}

[[nodiscard]] constexpr float extent_aspect(Extent2u extent) noexcept {
  return static_cast<float>(std::max(1u, extent.width)) /
         static_cast<float>(std::max(1u, extent.height));
}

} // namespace rengine::lsg
