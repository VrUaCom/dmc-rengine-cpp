#pragma once
#include <cstdint>
#include "skin_contract.inc"
namespace rengine::lsg {
enum class DetailBand : std::uint8_t { macro_only=0, meso=1, micro=2, micro_high=3 };
struct DetailThresholds {
  float meso_mm_per_px=RENGINE_SKIN_DETAIL_MESO_MM_PER_PX;
  float micro_mm_per_px=RENGINE_SKIN_DETAIL_MICRO_MM_PER_PX;
  float high_mm_per_px=RENGINE_SKIN_DETAIL_HIGH_MM_PER_PX;
};
[[nodiscard]] DetailBand select_detail_band(float mm_per_pixel, const DetailThresholds& thresholds = {});
[[nodiscard]] const char* detail_band_name(DetailBand band) noexcept;
}
