#pragma once

#include "rengine/lsg/camera.hpp"

#include <cstdint>
#include <memory>

namespace rengine::lsg {

enum class DiagnosticRenderMode : std::uint8_t {
  genome_perspective = 0,
  raw_perspective = 1,
  raw_orthographic = 2,
};

struct RendererDiagnostics {
  std::uint32_t swapchain_width{};
  std::uint32_t swapchain_height{};
  std::uint32_t logical_width{};
  std::uint32_t logical_height{};
  std::uint32_t surface_rotation{};
  float logical_aspect{1.0f};
  float fov_y_radians{};
  float camera_distance_m{};
  float near_plane_m{0.03f};
  float far_plane_m{30.0f};
  std::uint64_t estimated_gpu_bytes{};
  DiagnosticRenderMode mode{DiagnosticRenderMode::genome_perspective};
};

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

  void set_diagnostic_mode(DiagnosticRenderMode mode) noexcept;
  [[nodiscard]] DiagnosticRenderMode diagnostic_mode() const noexcept;
  [[nodiscard]] RendererDiagnostics diagnostics() const noexcept;

private:
  std::unique_ptr<Impl> impl_;
};

} // namespace rengine::lsg
