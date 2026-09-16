#ifndef RENGINE_LSG_SURFACE_GLSL
#define RENGINE_LSG_SURFACE_GLSL
uint lsg_pcg_hash(uint inputValue){ uint state=inputValue*747796405u+2891336453u; uint word=((state>>((state>>28u)+4u))^state)*277803737u; return (word>>22u)^word; }
uint lsg_hash5(uvec2 seed,uint region,ivec3 cell){ uint h=lsg_pcg_hash(seed.x^seed.y); h=lsg_pcg_hash(h^region*0x9e3779b9u); h=lsg_pcg_hash(h^uint(cell.x)); h=lsg_pcg_hash(h^uint(cell.y)); h=lsg_pcg_hash(h^uint(cell.z)); return h; }
float lsg_hash01(uint h){ return float(h>>8u)*(1.0/16777216.0); }
int lsg_detail_band(float mmPerPixel){ if(mmPerPixel<0.1)return 3; if(mmPerPixel<1.0)return 2; if(mmPerPixel<3.0)return 1; return 0; }
#endif
