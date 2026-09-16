#include "rengine/lsg/physiology.hpp"
namespace rengine::lsg {
PhysiologyState physiology_for(PhysiologyPreset p) noexcept {
  switch(p) {
    case PhysiologyPreset::exercise: return {128.0f, 37.3f, 0.84f, 0.72f, 0.45f};
    case PhysiologyPreset::cold: return {68.0f, 35.9f, 0.22f, 0.03f, 0.08f};
    case PhysiologyPreset::hot: return {82.0f, 37.1f, 0.66f, 0.88f, 0.12f};
    case PhysiologyPreset::normal: default: return {};
  }
}
}
