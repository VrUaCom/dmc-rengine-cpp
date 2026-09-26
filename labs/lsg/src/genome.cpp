#include "rengine/lsg/genome.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <type_traits>
namespace rengine::lsg {
namespace {
constexpr std::array<char,4> kMagic{'L','S','G','0'};
constexpr std::size_t kHeaderSize=20;
std::uint32_t crc32(std::span<const std::byte> data) noexcept {
  std::uint32_t crc=0xFFFFFFFFu;
  for(auto b:data){ crc ^= std::to_integer<std::uint8_t>(b); for(int i=0;i<8;++i) crc=(crc>>1u) ^ (0xEDB88320u & (0u-(crc&1u))); }
  return ~crc;
}
template<class T> void put_le(std::vector<std::byte>& out,T v){
  using U=std::make_unsigned_t<T>; U u=static_cast<U>(v); for(std::size_t i=0;i<sizeof(T);++i) out.push_back(static_cast<std::byte>((u>>(i*8u))&0xFFu));
}
template<class T> bool get_le(std::span<const std::byte> in,std::size_t& off,T& v){
  if(off+sizeof(T)>in.size()) return false;
  using U=std::make_unsigned_t<T>; std::uint64_t accum=0;
  for(std::size_t i=0;i<sizeof(T);++i) accum |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(in[off+i])) << (i*8u);
  off+=sizeof(T); v=static_cast<T>(static_cast<U>(accum)); return true;
}
void put_geometry(std::vector<std::byte>& o,const GeometryGenome& g){
  put_le(o,g.height); put_le(o,g.shoulder_width); put_le(o,g.pelvis_width); put_le(o,g.chest_volume); put_le(o,g.waist); put_le(o,g.limb_length); put_le(o,g.muscle); put_le(o,g.body_fat); put_le(o,g.neck); put_le(o,g.head_scale); put_le(o,g.jaw); put_le(o,g.facial_softness);
}
bool get_geometry(std::span<const std::byte> b,std::size_t& o,GeometryGenome& g){
  return get_le(b,o,g.height) && get_le(b,o,g.shoulder_width) && get_le(b,o,g.pelvis_width) && get_le(b,o,g.chest_volume) && get_le(b,o,g.waist) && get_le(b,o,g.limb_length) && get_le(b,o,g.muscle) && get_le(b,o,g.body_fat) && get_le(b,o,g.neck) && get_le(b,o,g.head_scale) && get_le(b,o,g.jaw) && get_le(b,o,g.facial_softness);
}
void put_face(std::vector<std::byte>& o,const FaceGenomeV0& f){
  put_le(o,f.skull_width); put_le(o,f.skull_height); put_le(o,f.face_length); put_le(o,f.forehead_height);
  put_le(o,f.brow_depth); put_le(o,f.eye_spacing); put_le(o,f.eye_size); put_le(o,f.eye_tilt);
  put_le(o,f.nose_length); put_le(o,f.nose_width); put_le(o,f.nose_projection); put_le(o,f.cheekbone_width);
  put_le(o,f.cheek_fullness); put_le(o,f.jaw_width); put_le(o,f.chin_width); put_le(o,f.chin_projection);
  put_le(o,f.mouth_width); put_le(o,f.upper_lip_fullness); put_le(o,f.lower_lip_fullness); put_le(o,f.lip_projection);
}
bool get_face(std::span<const std::byte> b,std::size_t& o,FaceGenomeV0& f){
  return get_le(b,o,f.skull_width) && get_le(b,o,f.skull_height) && get_le(b,o,f.face_length) && get_le(b,o,f.forehead_height) &&
         get_le(b,o,f.brow_depth) && get_le(b,o,f.eye_spacing) && get_le(b,o,f.eye_size) && get_le(b,o,f.eye_tilt) &&
         get_le(b,o,f.nose_length) && get_le(b,o,f.nose_width) && get_le(b,o,f.nose_projection) && get_le(b,o,f.cheekbone_width) &&
         get_le(b,o,f.cheek_fullness) && get_le(b,o,f.jaw_width) && get_le(b,o,f.chin_width) && get_le(b,o,f.chin_projection) &&
         get_le(b,o,f.mouth_width) && get_le(b,o,f.upper_lip_fullness) && get_le(b,o,f.lower_lip_fullness) && get_le(b,o,f.lip_projection);
}
void put_bytes(std::vector<std::byte>& o,const void* p,std::size_t n){const auto* q=static_cast<const std::uint8_t*>(p); for(std::size_t i=0;i<n;++i)o.push_back(static_cast<std::byte>(q[i]));}
bool get_bytes(std::span<const std::byte> b,std::size_t& o,void* p,std::size_t n){if(o+n>b.size())return false; std::memcpy(p,b.data()+o,n);o+=n;return true;}
}

std::vector<std::byte> encode_genome(const CharacterGenomeV0& g,std::uint32_t rev,std::uint32_t flags){
  std::vector<std::byte> payload; payload.reserve(160); put_geometry(payload,g.geometry);
  if(rev >= kGeneratorRevision) put_face(payload,g.face);
  put_bytes(payload,&g.skin,sizeof(g.skin)); put_bytes(payload,&g.eyes,sizeof(g.eyes)); put_bytes(payload,&g.micro,sizeof(g.micro)); put_bytes(payload,&g.physiology,sizeof(g.physiology)); put_le(payload,g.identity_seed); put_le(payload,g.surface_seed); put_le(payload,g.eye_seed);
  const std::size_t total=kHeaderSize+payload.size(); if(total>kGenomeHardLimit || total>std::numeric_limits<std::uint16_t>::max()) return {};
  std::vector<std::byte> out; out.reserve(total); for(char c:kMagic) out.push_back(static_cast<std::byte>(c)); put_le(out,kGenomeVersion); put_le(out,static_cast<std::uint16_t>(total)); put_le(out,rev); put_le(out,flags); put_le(out,crc32(payload)); out.insert(out.end(),payload.begin(),payload.end()); return out;
}

bool decode_genome(std::span<const std::byte> b,DecodedGenome& out,std::string& err){
  if(b.size()<kHeaderSize){err="genome shorter than header";return false;} if(b.size()>kGenomeHardLimit){err="genome exceeds 4096-byte hard limit";return false;}
  for(std::size_t i=0;i<4;++i) if(std::to_integer<char>(b[i])!=kMagic[i]){err="bad LSG magic";return false;}
  std::size_t o=4; std::uint16_t ver{},size{}; std::uint32_t rev{},flags{},sum{}; if(!get_le(b,o,ver)||!get_le(b,o,size)||!get_le(b,o,rev)||!get_le(b,o,flags)||!get_le(b,o,sum)){err="truncated header";return false;}
  if(ver!=kGenomeVersion){err="unsupported genome version";return false;} if(size!=b.size()){err="declared size mismatch";return false;} if(crc32(b.subspan(kHeaderSize))!=sum){err="checksum mismatch";return false;}
  if(rev!=kGeneratorRevision && rev!=kLegacyGeneratorRevision){err="unsupported generator revision; explicit migration required";return false;}
  CharacterGenomeV0 g{};
  if(!get_geometry(b,o,g.geometry)){err="truncated geometry payload";return false;}
  if(rev==kGeneratorRevision && !get_face(b,o,g.face)){err="truncated Face DNA payload";return false;}
  if(!get_bytes(b,o,&g.skin,sizeof(g.skin))||!get_bytes(b,o,&g.eyes,sizeof(g.eyes))||!get_bytes(b,o,&g.micro,sizeof(g.micro))||!get_bytes(b,o,&g.physiology,sizeof(g.physiology))||!get_le(b,o,g.identity_seed)||!get_le(b,o,g.surface_seed)||!get_le(b,o,g.eye_seed)){err="truncated payload";return false;} if(o!=b.size()){err="unexpected trailing payload";return false;}
  out={g,rev,flags}; err.clear(); return true;
}

} // namespace rengine::lsg
