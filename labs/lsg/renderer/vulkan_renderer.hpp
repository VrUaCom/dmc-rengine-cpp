#pragma once

#include "rengine/lsg/camera.hpp"

#include <cstdint>
#include <memory>

namespace rengine::lsg {

class VulkanRenderer {
public:
  struct Impl;

  VulkanRenderer();
  ~VulkanRenderer();
  VulkanRenderer(const VulkanRenderer&) = delete;
  VulkanRenderer& operator=(const VulkanRenderer&) = delete;

  [[nodiscard]] bool initialize(void* native_window, void* asset_manager = nullptr);
  void shutdown() noexcept;
  [[nodiscard]] bool draw_frame(float time_seconds,
                                std::uint32_t character_index = 0,
                                bool detail_enabled = true) noexcept;
  [[nodiscard]] bool ready() const noexcept;
  [[nodiscard]] std::uint64_t estimated_gpu_bytes() const noexcept;

  void orbit_camera(float normalized_dx, float normalized_dy) noexcept;
  void zoom_camera(float scale) noexcept;
  void set_camera_preset(CameraPreset preset) noexcept;
  [[nodiscard]] CameraState camera_state() const noexcept;

private:
  std::unique_ptr<Impl> impl_;
};

} // namespace rengine::lsg
