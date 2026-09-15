// Canonical-image structural census. No instruction semantics are inferred.
// MSVC RTTI layout: clang/lib/CodeGen/MicrosoftCXXABI.cpp (LLVM).
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include <algorithm>
#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

using namespace dmc::rengine;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
constexpr auto canonical = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

struct Image {
    std::vector<std::byte> bytes;
    exe::PeImage pe;
    std::optional<std::size_t> offset(U64 rva, std::size_t size) const {
        if (rva > UINT32_MAX) return {};
        for (const auto& s : pe.sections) {
            if (rva < s.virtual_address) continue;
            const auto delta = rva - s.virtual_address;
            if (delta > s.raw_size || size > s.raw_size - delta) continue;
            const auto off = s.raw_offset + delta;
            if (off > bytes.size() || size > bytes.size() - off) return {};
            return static_cast<std::size_t>(off);
        }
        return {};
    }
    U64 read(U64 rva, std::size_t size=4) const {
        const auto off=offset(rva,size);
        if (!off || size>8) throw std::runtime_error("unbacked RVA");
        U64 v=0;
        for(std::size_t i=0;i<size;++i)
            v |= U64(std::to_integer<unsigned char>(bytes[*off+i])) << (8*i);
        return v;
    }
    U32 u32(U64 rva) const { return static_cast<U32>(read(rva)); }
    bool code(U64 va) const {
        if(va<pe.image_base) return false;
        const auto rva=va-pe.image_base;
        for(const auto& s:pe.sections)
            if((s.characteristics & 0x20000000U) && rva>=s.virtual_address &&
               rva-s.virtual_address<s.virtual_size && offset(rva,1)) return true;
        return false;
    }
    std::string type(U32 rva) const {
        if(!offset(rva,20)) return {};
        std::string n;
        for(U64 i=U64(rva)+16;i<U64(rva)+16+1024;++i) {
            if(!offset(i,1)) return {};
            auto c=static_cast<char>(read(i,1));
            if(c==0) return n.starts_with(".?AV") || n.starts_with(".?AU") ? n : "";
            if(c<32 || c>126) return {};
            n+=c;
        }
        return {};
    }
};
std::string hex(U64 v) { std::ostringstream s; s<<"0x"<<std::hex<<v; return s.str(); }
std::string name(std::string n) {
    // Only simplify unqualified non-template names. Preserve all other mangling.
    if(n.size()>6 && n.ends_with("@@") && n.find('@',4)==n.size()-2 && n[4]!='$')
        return n.substr(4,n.size()-6);
    return n;
}
struct Base { U32 descriptor{},type{},contained{},mdisp{},pdisp{},vdisp{},flags{}; };
struct Hierarchy { U32 rva{},flags{}; std::vector<Base> bases; };
struct Locator { U32 rva{},type{},hierarchy{},offset{},cd_offset{}; };
struct Census {
    std::map<U32,std::string> types;
    std::map<U32,Hierarchy> hierarchies;
    std::map<U32,Locator> locators;
    std::vector<std::tuple<U32,U32,U64>> anchors;
    std::set<std::pair<U32,U32>> edges;
};
// Flattened preorder contains descendant counts, not just direct bases.
std::vector<std::pair<std::size_t,std::size_t>> parents(const std::vector<Base>& b) {
    if(b.empty() || b[0].contained!=b.size()-1) throw std::runtime_error("bad RTTI root span");
    std::vector<std::size_t> stack;
    std::vector<std::pair<std::size_t,std::size_t>> out;
    for(std::size_t i=0;i<b.size();++i) {
        while(!stack.empty() && i>stack.back()+b[stack.back()].contained) stack.pop_back();
        if(b[i].contained>b.size()-1-i) throw std::runtime_error("RTTI span overflow");
        if(i) {
            if(stack.empty() || i+b[i].contained>stack.back()+b[stack.back()].contained)
                throw std::runtime_error("RTTI crossing spans");
            out.emplace_back(stack.back(),i);
        }
        stack.push_back(i);
    }
    return out;
}
Census census(const Image& image) {
    Census out;
    for(const auto& s:image.pe.sections) {
        if(s.characteristics & 0x20000000U) continue;
        for(U64 r=s.virtual_address;r+24<=U64(s.virtual_address)+s.raw_size;r+=4) {
            if(!image.offset(r,24) || image.read(r)!=1 || image.read(r+20)!=r) continue;
            const auto td=image.u32(r+12), ch=image.u32(r+16);
            const auto type=image.type(td);
            if(type.empty() || !image.offset(ch,16) || image.read(ch)!=0) continue;
            const auto count=image.u32(U64(ch)+8), array=image.u32(U64(ch)+12);
            if(!count || count>4096 || !image.offset(array,std::size_t(count)*4)) continue;
            Hierarchy h{ch,image.u32(U64(ch)+4),{}};
            bool valid=true;
            for(U32 i=0;i<count;++i) {
                const auto br=image.u32(U64(array)+U64(i)*4);
                if(!image.offset(br,24)) { valid=false; break; }
                Base b{br,image.u32(br),image.u32(U64(br)+4),image.u32(U64(br)+8),
                       image.u32(U64(br)+12),image.u32(U64(br)+16),image.u32(U64(br)+20)};
                if(image.type(b.type).empty()) { valid=false; break; }
                h.bases.push_back(b);
            }
            if(!valid || h.bases.front().type!=td) continue;
            try {
                const auto links=parents(h.bases);
                for(const auto& [p,c]:links) out.edges.emplace(h.bases[p].type,h.bases[c].type);
            } catch(const std::runtime_error&) { continue; }
            for(const auto& b:h.bases) out.types.emplace(b.type,image.type(b.type));
            out.hierarchies.emplace(ch,std::move(h));
            const auto rv=static_cast<U32>(r);
            out.locators.emplace(rv,Locator{rv,td,ch,image.u32(r+4),image.u32(r+8)});
        }
    }
    for(const auto& s:image.pe.sections) {
        if(s.characteristics & 0x20000000U) continue;
        for(U64 r=s.virtual_address;r+16<=U64(s.virtual_address)+s.raw_size;r+=8) {
            if(!image.offset(r,16)) continue;
            const auto ptr=image.read(r,8);
            if(ptr<image.pe.image_base || ptr-image.pe.image_base>UINT32_MAX) continue;
            auto it=out.locators.find(static_cast<U32>(ptr-image.pe.image_base));
            if(it!=out.locators.end() && image.code(image.read(r+8,8)))
                out.anchors.emplace_back(static_cast<U32>(r+8),it->first,image.read(r+8,8));
        }
    }
    return out;
}
std::ofstream output(const std::filesystem::path& dir,const char* n) {
    std::ofstream out(dir/n);
    out.exceptions(std::ios::failbit|std::ios::badbit);
    return out;
}
void self_test() {
    auto require=[](bool v){if(!v) throw std::runtime_error("self-test failed");};
    // D : B : A, C. D->A is transitive and must not be emitted.
    std::vector<Base> b(4); b[0].contained=3; b[1].contained=1;
    require(parents(b)==std::vector<std::pair<std::size_t,std::size_t>>{{0,1},{1,2},{0,3}});
    b[1].contained=4;
    bool rejected=false; try{(void)parents(b);}catch(const std::runtime_error&){rejected=true;}
    require(rejected);
    Image im; im.bytes.resize(64); im.pe.sections.push_back({"test",128,0x1000,64,0,0});
    require(im.offset(0x103f,1).has_value()); require(!im.offset(0x103f,2));
    require(!im.offset(0x1040,1)); require(!im.offset(0xffffffffULL,4));
    // A plausible decorated string without a valid COL/CHD is not a class.
    const std::string fake=".?AVFake@@";
    for(std::size_t i=0;i<fake.size();++i) im.bytes[16+i]=std::byte(fake[i]);
    require(im.type(0x1000)==fake); require(census(im).types.empty());
    // Positive x64 COL -> CHD -> BCA -> BCD -> TypeDescriptor -> vftable.
    im.bytes.assign(1024,std::byte{}); im.pe.image_base=0x140000000;
    im.pe.sections={{".rdata",768,0x1000,768,0,0}, {".text",256,0x2000,256,768,0x20000000}};
    auto put=[&](U32 r,U64 v,std::size_t size=4){auto o=im.offset(r,size).value(); for(std::size_t i=0;i<size;++i) im.bytes[o+i]=std::byte((v>>(i*8))&255);};
    for(std::size_t i=0;i<fake.size();++i) im.bytes[16+i]=std::byte(fake[i]);
    put(0x1100,1);put(0x110c,0x1000);put(0x1110,0x1140);put(0x1114,0x1100);
    put(0x1148,1);put(0x114c,0x1180);put(0x1180,0x11a0);
    put(0x11a0,0x1000);put(0x11ac,UINT32_MAX);
    put(0x1200,im.pe.image_base+0x1100,8);put(0x1208,im.pe.image_base+0x2000,8);
    auto positive=census(im);
    require(positive.types.size()==1 && positive.locators.size()==1 && positive.anchors.size()==1);
    put(0x1114,0x1104);require(census(im).locators.empty());put(0x1114,0x1100);
    put(0x1148,4097);require(census(im).locators.empty());put(0x1148,1);
    put(0x1208,im.pe.image_base+0x1000,8);require(census(im).anchors.empty());
    std::cout<<"self-test PASS: preorder, malformed span, file-backed bounds, unanchored type, valid RTTI chain, bad self RVA, oversized array, noncode vtable entry\n";
}
int main(int argc,char** argv) try {
    if(argc==2 && std::string(argv[1])=="--self-test") {self_test();return 0;}
    if(argc!=3) {std::cerr<<"usage: dmc3-runtime-tree canonical.exe output-directory\n";return 2;}
    Image im;
    std::ifstream in(argv[1],std::ios::binary|std::ios::ate);
    if(!in || in.tellg()!=6356432) throw std::runtime_error("canonical image size mismatch");
    im.bytes.resize(6356432); in.seekg(0);
    if(!in.read(reinterpret_cast<char*>(im.bytes.data()),static_cast<std::streamsize>(im.bytes.size())))
        throw std::runtime_error("image read failed");
    if(core::Sha256::compute(im.bytes).hex()!=canonical) throw std::runtime_error("canonical SHA-256 mismatch");
    auto parsed=exe::PeReader::read(im.bytes);
    if(!parsed.ok()) throw std::runtime_error("invalid PE");
    im.pe=*parsed.image;
    if(im.pe.kind!=exe::PeKind::pe32_plus || im.pe.machine!=exe::PeMachine::amd64)
        throw std::runtime_error("unsupported ABI");
    const auto c=census(im);
    const std::filesystem::path dir(argv[2]);
    std::filesystem::create_directories(dir);
    // Instruction boundaries reviewed with objdump for this exact hash.
    // This is a bounded callsite receipt, never a byte-pattern callgraph scan.
    auto calls=output(dir,"entry_calls.tsv");
    calls<<"caller_va\tcallsite_va\tcallee_va\trelation\n";
    constexpr U32 sites[]={0x2c5e03,0x2c5e1f,0x2c5e2b,0x2c5e30,0x2c5e3c,
        0x2c5e48,0x2c5e4d,0x2c5e59,0x2c5e69,0x2c5f39,0x2c5f5c,0x2c5f6b,
        0x2c5f75,0x2c5f85,0x2c5f91,0x2c5f9d,0x2c5fa9,0x2c5fb5,0x2c5fd3};
    for(const auto r:sites) {
        if(im.read(r,1)!=0xe8) throw std::runtime_error("reviewed direct call opcode mismatch");
        const auto target=std::int64_t(r)+5+std::bit_cast<std::int32_t>(im.u32(U64(r)+1));
        if(target<0 || !im.code(im.pe.image_base+static_cast<U64>(target))) throw std::runtime_error("invalid direct call target");
        calls<<"0x1402c5df0\t"<<hex(im.pe.image_base+r)<<'\t'<<hex(im.pe.image_base+static_cast<U64>(target))<<"\tdirect_call_not_ownership\n";
    }
    auto scenes=output(dir,"scene_dispatch.tsv");
    scenes<<"dispatch_va\tselector\tjump_entry_va\tcase_va\n";
    for(U32 i=0;i<10;++i) {
        const U32 slot=0x2402b8+i*4,target=im.u32(slot);
        if(!im.code(im.pe.image_base+target)) throw std::runtime_error("invalid scene case");
        scenes<<"0x140240090\t"<<i<<'\t'<<hex(im.pe.image_base+slot)<<'\t'<<hex(im.pe.image_base+target)<<'\n';
    }
    auto candidates=output(dir,"type_candidates.tsv");
    candidates<<"type_va\tdecorated_name\treferenced_by_validated_hierarchy\n";
    std::size_t candidate_count=0;
    for(const auto& s:im.pe.sections) if(!(s.characteristics&0x20000000U))
        for(U64 r=s.virtual_address;r+20<=U64(s.virtual_address)+s.raw_size;r+=8) {
            const auto n=im.type(static_cast<U32>(r));
            if(n.empty()) continue;
            ++candidate_count;
            candidates<<hex(im.pe.image_base+r)<<'\t'<<n<<'\t'<<(c.types.contains(static_cast<U32>(r))?"true":"false")<<'\n';
        }
    auto types=output(dir,"types.tsv"); types<<"type_va\tname\tdecorated_name\n";
    for(const auto& [r,n]:c.types) types<<hex(im.pe.image_base+r)<<'\t'<<name(n)<<'\t'<<n<<'\n';
    auto edges=output(dir,"inheritance.tsv"); edges<<"derived_type_va\tbase_type_va\tderived\tbase\n";
    for(const auto& [d,b]:c.edges) edges<<hex(im.pe.image_base+d)<<'\t'<<hex(im.pe.image_base+b)<<'\t'<<name(c.types.at(d))<<'\t'<<name(c.types.at(b))<<'\n';
    auto proof=output(dir,"inheritance_evidence.tsv");
    proof<<"hierarchy_va\tparent_preorder_index\tchild_preorder_index\tderived_type_va\tbase_type_va\tbase_descriptor_va\n";
    for(const auto& [r,h]:c.hierarchies) for(const auto& [p,b]:parents(h.bases))
        proof<<hex(im.pe.image_base+r)<<'\t'<<p<<'\t'<<b<<'\t'<<hex(im.pe.image_base+h.bases[p].type)<<'\t'<<hex(im.pe.image_base+h.bases[b].type)<<'\t'<<hex(im.pe.image_base+h.bases[b].descriptor)<<'\n';
    auto tree=output(dir,"class_tree.md");
    tree<<"# DMC3 internal class graph\n\nGenerated from the SHA-256-bound executable. Each row is a class node; direct bases are outgoing inheritance links. Multiple inheritance is retained. These are recovered runtime types, not original source folders or complete class definitions. Decorated template/namespace names remain exact.\n\n| Class | Type descriptor VA | Direct bases |\n| --- | --- | --- |\n";
    for(const auto& [r,n]:c.types) {
        tree<<"| `"<<name(n)<<"` | `"<<hex(im.pe.image_base+r)<<"` | ";
        bool first=true;
        for(const auto& [d,b]:c.edges) if(d==r) {if(!first) tree<<", ";tree<<'`'<<name(c.types.at(b))<<'`';first=false;}
        tree<<(first?"No base recorded in validated RTTI":"")<<" |\n";
    }
    auto bases=output(dir,"subobjects.tsv");
    bases<<"hierarchy_va\tmost_derived_type_va\tpreorder_index\tbase_descriptor_va\ttype_va\tdescendant_count\tmdisp\tpdisp\tvdisp\tattributes\n";
    std::size_t virtual_bases=0;
    for(const auto& [r,h]:c.hierarchies) for(std::size_t i=0;i<h.bases.size();++i) {
        const auto& b=h.bases[i]; if(b.pdisp!=UINT32_MAX) ++virtual_bases;
        bases<<hex(im.pe.image_base+r)<<'\t'<<hex(im.pe.image_base+h.bases[0].type)<<'\t'<<i<<'\t'
             <<hex(im.pe.image_base+b.descriptor)<<'\t'<<hex(im.pe.image_base+b.type)<<'\t'<<b.contained<<'\t'
             <<std::bit_cast<std::int32_t>(b.mdisp)<<'\t'<<std::bit_cast<std::int32_t>(b.pdisp)<<'\t'
             <<std::bit_cast<std::int32_t>(b.vdisp)<<'\t'<<hex(b.flags)<<'\n';
    }
    auto loc=output(dir,"locators.tsv"); loc<<"locator_va\ttype_va\thierarchy_va\tsubobject_offset\tconstructor_displacement_offset\n";
    for(const auto& [r,l]:c.locators) loc<<hex(im.pe.image_base+r)<<'\t'<<hex(im.pe.image_base+l.type)<<'\t'<<hex(im.pe.image_base+l.hierarchy)<<'\t'<<l.offset<<'\t'<<l.cd_offset<<'\n';
    auto vt=output(dir,"vtable_anchors.tsv"); vt<<"vftable_va\tlocator_va\ttype_va\tsubobject_offset\tfirst_entry_va\tclass\n";
    for(const auto& [r,l,f]:c.anchors) {const auto& col=c.locators.at(l); vt<<hex(im.pe.image_base+r)<<'\t'<<hex(im.pe.image_base+l)<<'\t'<<hex(im.pe.image_base+col.type)<<'\t'<<col.offset<<'\t'<<hex(f)<<'\t'<<name(c.types.at(col.type))<<'\n';}
    auto ranges=output(dir,"runtime_ranges.tsv");
    ranges<<"begin_va\tend_va_exclusive\tunwind_va\tversion\tflags\tchain_begin_va\tchain_end_va\tchain_unwind_va\n";
    std::size_t count=0, chained=0, handlers=0;
    for(const auto& s:im.pe.sections) if(s.name==".pdata") {
        if(s.virtual_size%12) throw std::runtime_error("unaligned pdata extent");
        for(U64 r=s.virtual_address;r<U64(s.virtual_address)+s.virtual_size;r+=12) {
            const U32 begin=im.u32(r),end=im.u32(r+4),unwind=im.u32(r+8);
            if(begin>=end || !im.code(im.pe.image_base+begin) || !im.code(im.pe.image_base+end-1)) throw std::runtime_error("bad runtime range");
            const auto v=im.read(unwind,1),flags=v>>3,codes=im.read(U64(unwind)+2,1);
            if((v&7)!=1 || ((flags&4) && (flags&3))) throw std::runtime_error("unsupported unwind header");
            const U64 tail=U64(unwind)+4+((codes+1)&~U64(1))*2;
            if(!im.offset(unwind,static_cast<std::size_t>(tail-unwind))) throw std::runtime_error("truncated unwind codes");
            ranges<<hex(im.pe.image_base+begin)<<'\t'<<hex(im.pe.image_base+end)<<'\t'<<hex(im.pe.image_base+unwind)<<'\t'<<(v&7)<<'\t'<<flags;
            if(flags&4) {++chained;ranges<<'\t'<<hex(im.pe.image_base+im.read(tail))<<'\t'<<hex(im.pe.image_base+im.read(tail+4))<<'\t'<<hex(im.pe.image_base+im.read(tail+8));}
            else {ranges<<"\t\t\t"; if(flags&3) {++handlers;(void)im.read(tail);}}
            ranges<<'\n';++count;
        }
    }
    auto summary=output(dir,"summary.json");
    summary<<"{\n  \"schema\": \"dmc-rengine.runtime-structure-census.v1\",\n  \"sha256\": \""<<canonical<<"\",\n  \"image_base\": \""<<hex(im.pe.image_base)<<"\",\n  \"types\": "<<c.types.size()<<",\n  \"locators\": "<<c.locators.size()<<",\n  \"vtable_anchors\": "<<c.anchors.size()<<",\n  \"direct_inheritance_edges\": "<<c.edges.size()<<",\n  \"hierarchies\": "<<c.hierarchies.size()<<",\n  \"virtual_base_occurrences\": "<<virtual_bases<<",\n  \"runtime_ranges\": "<<count<<",\n  \"chained_ranges\": "<<chained<<",\n  \"handler_ranges\": "<<handlers<<",\n  \"logical_function_count\": null,\n  \"full_game_decompiled\": false\n}\n";
    std::cout<<candidate_count<<" decorated type candidates, "<<c.types.size()<<" hierarchy-linked types, "<<c.locators.size()<<" COLs, "<<c.anchors.size()<<" vtable anchors, "<<c.edges.size()<<" direct inheritance edges; "<<count<<" runtime ranges, "<<chained<<" chained, "<<handlers<<" handlers\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
