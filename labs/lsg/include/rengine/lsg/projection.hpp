#pragma once

#include <algorithm>
#include <cstdint>

namespace rengine::lsg {

struct Extent2u {
  std::uint32_t width{1};
  std::uint32_t height{1};
};

[[nodiscard]] constexpr Extent2u logical_extent_for_surface_rotation(
    Extent2u swapchain_extent, std::uint32_t rotation_code) noexcept {
  swapchain_extent.width = std::max(1u, swapchain_extent.width);
  swapchain_extent.height = std::max(1u, swapchain_extent.height);
  if (rotation_code == 1u || rotation_code == 3u) {
    return {swapchain_extent.height, swapchain_extent.width};
  }
  return swapchain_extent;
}

[[nodiscard]] constexpr float extent_aspect(Extent2u extent) noexcept {
  return static_cast<float>(std::max(1u, extent.width)) /
         static_cast<float>(std::max(1u, extent.height));
}

} // namespace rengine::lsg
