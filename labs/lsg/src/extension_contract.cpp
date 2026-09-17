#include "rengine/lsg/extension_points.hpp"

#include <type_traits>

namespace rengine::lsg {
static_assert(std::is_trivially_copyable_v<SurfaceState>);
static_assert(std::is_trivially_copyable_v<DamageState>);
static_assert(std::is_trivially_copyable_v<HairGenome>);
static_assert(std::is_trivially_copyable_v<ClothGenome>);
static_assert(sizeof(HairGenome) <= 24);
static_assert(sizeof(ClothGenome) <= 16);
} // namespace rengine::lsg
