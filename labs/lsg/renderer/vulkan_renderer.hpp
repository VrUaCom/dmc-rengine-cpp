#pragma once

#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/physiology.hpp"
#include "rengine/lsg/eye_runtime.hpp"
#include "rengine/lsg/lighting_runtime.hpp"

#include <cstdint>
#include <memory>

namespace rengine::lsg {

enum class DiagnosticRenderMode : std::uint8_t {
  genome_perspective = 0,
  raw_perspective = 1,
  raw_orthographic = 2,
  genome_joint_debug = 3,
};

enum class EyeDiagnosticMode : std::uint8_t {
  normal = 0,
  components = 1,
  iris_only = 2,
  cornea_only = 3,
};

struct RendererDiagnostics {
  std::uint32_t window_width{};
  std::uint32_t window_height{};
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
  LightingPreset lighting_preset{LightingPreset::noon};
  OpticalFilterPreset optical_filter{OpticalFilterPreset::clear};
  float scene_luminance{};
  float effective_eye_luminance{};
  float pupil_target_radius{};
  float pupil_current_radius{};
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
  void reset_camera_view() noexcept;
  [[nodiscard]] CameraState camera_state() const noexcept;

  void set_diagnostic_mode(DiagnosticRenderMode mode) noexcept;
  [[nodiscard]] DiagnosticRenderMode diagnostic_mode() const noexcept;

  void set_ui_tooltip_row(int row) noexcept;
  [[nodiscard]] int ui_tooltip_row() const noexcept;

  void set_physiology_preset(PhysiologyPreset preset) noexcept;
  [[nodiscard]] PhysiologyPreset physiology_preset() const noexcept;

  void set_eye_diagnostic_mode(EyeDiagnosticMode mode) noexcept;
  [[nodiscard]] EyeDiagnosticMode eye_diagnostic_mode() const noexcept;

  void set_lighting_preset(LightingPreset preset) noexcept;
  [[nodiscard]] LightingPreset lighting_preset() const noexcept;
  void set_optical_filter_preset(OpticalFilterPreset preset) noexcept;
  [[nodiscard]] OpticalFilterPreset optical_filter_preset() const noexcept;
  [[nodiscard]] LightingRuntimeState lighting_state() const noexcept;

  [[nodiscard]] RendererDiagnostics diagnostics() const noexcept;

private:
  std::unique_ptr<Impl> impl_;
};

} // namespace rengine::lsg
