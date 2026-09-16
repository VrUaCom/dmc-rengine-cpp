#pragma once
#include <cstdint>
namespace rengine::lsg {
enum class DetailBand : std::uint8_t { macro_only=0, meso=1, micro=2, micro_high=3 };
struct DetailThresholds { float meso_mm_per_px=3.0f; float micro_mm_per_px=1.0f; float high_mm_per_px=0.1f; };
[[nodiscard]] DetailBand select_detail_band(float mm_per_pixel, const DetailThresholds& thresholds = {});
[[nodiscard]] const char* detail_band_name(DetailBand band) noexcept;
}
