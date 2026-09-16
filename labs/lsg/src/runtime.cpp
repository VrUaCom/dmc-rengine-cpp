#include "rengine/lsg/runtime.hpp"
namespace rengine::lsg {
CharacterRuntime::CharacterRuntime(CharacterGenomeV0 genome) : genome_(genome), physiology_(physiology_for(PhysiologyPreset::normal)) {}
void CharacterRuntime::set_physiology(PhysiologyPreset preset) noexcept { physiology_ = physiology_for(preset); }
}
