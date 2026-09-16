#pragma once
namespace rengine::lsg {
enum class PhysiologyPreset { normal, exercise, cold, hot };
struct PhysiologyState {
  float heart_rate=62.0f;
  float temperature=36.6f;
  float perfusion=0.45f;
  float sweat=0.08f;
  float fatigue=0.0f;
};
[[nodiscard]] PhysiologyState physiology_for(PhysiologyPreset preset) noexcept;
}
