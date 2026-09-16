#include "rengine/lsg/genome.hpp"
#include <iostream>
#include <string_view>
int main(int argc,char** argv){
  std::uint32_t profile=0; for(int i=1;i+1<argc;++i) if(std::string_view{argv[i]}=="--character") profile=static_cast<std::uint32_t>(std::stoul(argv[++i])); const auto bytes=rengine::lsg::encode_genome(rengine::lsg::builtin_profile(profile)); std::cout << "Rengine LSG Prototype bootstrap; character=" << (profile&1u) << "; genome=" << bytes.size() << " bytes\n"; std::cout << "Desktop surface creation is intentionally gated until the Win32 Vulkan viewer slice.\n"; return 0;
}
