#include "rengine/lsg/deterministic_hash.hpp"
#include "rengine/lsg/detail_scheduler.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/physiology.hpp"
#include "rengine/lsg/surface.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace rengine::lsg;
int main(){
  const auto g0=builtin_profile(0), g1=builtin_profile(1);
  const auto bytes=encode_genome(g0); assert(!bytes.empty()); assert(bytes.size()<512); assert(bytes.size()<=kGenomeHardLimit);
  DecodedGenome d{}; std::string err; assert(decode_genome(bytes,d,err)); assert(d.generator_revision==kGeneratorRevision); assert(d.value.surface_seed==g0.surface_seed);
  auto corrupt=bytes; corrupt.back() ^= std::byte{1}; assert(!decode_genome(corrupt,d,err));
  constexpr auto h1=hash5(123,2,10,20,30); constexpr auto h2=hash5(123,2,10,20,30); constexpr auto h3=hash5(124,2,10,20,30); static_assert(h1==h2); static_assert(h1!=h3);
  assert(select_detail_band(4.0f)==DetailBand::macro_only); assert(select_detail_band(2.0f)==DetailBand::meso); assert(select_detail_band(0.5f)==DetailBand::micro); assert(select_detail_band(0.05f)==DetailBand::micro_high);
  const auto a=sample_surface(g0,BodyRegion::head,{0.1f,1.7f,0.05f},0.05f,physiology_for(PhysiologyPreset::normal)); const auto b=sample_surface(g0,BodyRegion::head,{0.1f,1.7f,0.05f},0.05f,physiology_for(PhysiologyPreset::normal)); assert(a.pore_height==b.pore_height); assert(std::isfinite(a.roughness));
  const auto ex=sample_surface(g1,BodyRegion::head,{0.2f,1.6f,0.03f},0.05f,physiology_for(PhysiologyPreset::exercise)); const auto normal=sample_surface(g1,BodyRegion::head,{0.2f,1.6f,0.03f},0.05f,physiology_for(PhysiologyPreset::normal)); assert(ex.redness>normal.redness); assert(ex.roughness<normal.roughness);
  std::cout << "LSG tests PASS; genome bytes=" << bytes.size() << "\n"; return 0;
}
