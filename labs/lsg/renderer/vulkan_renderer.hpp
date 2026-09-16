#pragma once
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

  [[nodiscard]] bool initialize(void* native_window);
  void shutdown() noexcept;
  [[nodiscard]] bool draw_frame(float time_seconds) noexcept;
  [[nodiscard]] bool ready() const noexcept;
  [[nodiscard]] std::uint64_t estimated_gpu_bytes() const noexcept;

private:
  std::unique_ptr<Impl> impl_;
};
}
