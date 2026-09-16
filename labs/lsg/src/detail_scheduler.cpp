#include "rengine/lsg/detail_scheduler.hpp"
#include <cmath>
namespace rengine::lsg {
DetailBand select_detail_band(float mm, const DetailThresholds& t) {
  if(!std::isfinite(mm) || mm < 0.0f) return DetailBand::macro_only;
  if(mm < t.high_mm_per_px) return DetailBand::micro_high;
  if(mm < t.micro_mm_per_px) return DetailBand::micro;
  if(mm < t.meso_mm_per_px) return DetailBand::meso;
  return DetailBand::macro_only;
}
const char* detail_band_name(DetailBand b) noexcept {
  switch(b){case DetailBand::macro_only:return "MACRO";case DetailBand::meso:return "MESO";case DetailBand::micro:return "MICRO";case DetailBand::micro_high:return "MICRO_HIGH";}
  return "UNKNOWN";
}
}
