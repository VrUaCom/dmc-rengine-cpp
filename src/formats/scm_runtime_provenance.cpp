#include "dmc_rengine/formats/scm_runtime_provenance.hpp"

#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/formats/scm_render.hpp"
#include "dmc_rengine/formats/scm_runtime_flags.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::scm {
namespace {

inline constexpr std::string_view evidence_id =
    "dmc3-scm-canonical-exe-deep-reader-2026-09-13";

[[nodiscard]] std::string index_id(
    std::string_view prefix,
    std::size_t outer,
    std::size_t inner = static_cast<std::size_t>(-1)) {
    std::ostringstream out;
    out << prefix << '-' << std::setfill('0') << std::setw(3) << outer;
    if (inner != static_cast<std::size_t>(-1)) out << '-' << std::setw(3) << inner;
    return out.str();
}

[[nodiscard]] bool add(binary::Document& document, std::string id,
    std::uint64_t offset, std::uint64_t size, std::string text,
    std::vector<std::string> tags) {
    if (size == 0U) return true;
    return document.add_annotation(binary::Annotation{
        .id = std::move(id), .range = {.offset = offset, .size = size},
        .text = std::move(text), .evidence_id = std::string(evidence_id),
        .tags = std::move(tags)});
}

[[nodiscard]] bool add_header_provenance(binary::Document& d) {
    return add(d, "scm-prov-header-object-count", 0x10U, 1U,
        "L0 SCM+0x10 -> 0x1402F9570 -> manager+0xE8 object count.",
        {"EXE_CONFIRMED","runtime-provenance"}) &&
    add(d, "scm-prov-header-node-count", 0x11U, 1U,
        "L0 SCM+0x11 -> 0x1402F9570 -> manager+0xEA scene-node count -> 0x1402F1DB0 node domain.",
        {"EXE_CONFIRMED","runtime-provenance","scene"}) &&
    add(d, "scm-prov-header-texture-mirror", 0x12U, 1U,
        "SCM+0x12 is a serialized texture-slot mirror. Live count comes from manager+0x110 external companion into manager+0xEC.",
        {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","external-companion","mirror"}) &&
    add(d, "scm-prov-header-13", 0x13U, 1U,
        "SCM+0x13 lighting_reference_node_index -> manager+0xFA. 0x1402F1DB0 installs scene-node matrices at manager+0x188 with 0x40 stride; CDrawSCM 0x1402FD040 indexes that array, takes the selected world-position lane +0x30 and feeds 0x1402EE560 -> MDL_LIGHT_MAT {Lc,Lv} consumed by the stage vertex shader.",
        {"EXE_CONFIRMED","runtime-provenance","scene-node","lighting-reference","matrix-selector","shader-visible"}) &&
    add(d, "scm-prov-header-resource-code", 0x14U, 4U,
        "SCM+0x14 decimal structural code -> 0x1402F9570 -> manager+0xE4. Expanded retail authority observes family classes 3/4/7/8; no type-proven downstream manager+0xE4 role is promoted.",
        {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","structural-code","PRESERVED_UNDECODED_HIGH_LEVEL_ROLE"}) &&
    add(d, "scm-prov-header-scene-offset", 0x20U, 8U,
        "Scene-node block offset -> SCM setup 0x140303C10 -> node binder 0x1402F1DB0.",
        {"STRUCTURAL_CONFIRMED","runtime-provenance","scene"}) &&
    add(d, "scm-prov-header-reserved08-negative", 0x08U, 8U,
        "SCM+0x08..+0x0F is corpus-zero. The shared model-manager whole-image source-pointer census exposes no typed runtime effect for this lane. Preserve exactly; corpus zero is not padding authority.",
        {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","header"}) &&
    add(d, "scm-prov-header-reserved18-negative", 0x18U, 8U,
        "SCM+0x18..+0x1F is corpus-zero with whole-image model-source dormancy. No promoted typed consumer is established. Preserve exactly.",
        {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","header"}) &&
    add(d, "scm-prov-header-reserved28-negative", 0x28U, 0x18U,
        "SCM+0x28..+0x3F is the terminal header shell: corpus-zero with whole-image model-source dormancy and no promoted typed consumer. Preserve exactly.",
        {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","header"});
}

[[nodiscard]] bool add_object_provenance(binary::Document& d,
    const ParseResult& parsed) {
    for (std::size_t oi=0; oi<parsed.document.objects.size(); ++oi) {
        const auto& o=parsed.document.objects[oi];
        const auto p=index_id("scm-prov-object",oi);
        if (!add(d,p+"-alpha",o.record_offset+1U,1U,
            "object+0x01 -> 0x140302F10 -> runtime alpha control -> 0x140304111..0x140304167 -> MDL_PARTS_COLOR_PKT.alpha.w -> DMC3_STG COLOR0.a.",
            {"EXE_CONFIRMED","runtime-provenance","alpha-control","shader-visible"}) ||
            !add(d,p+"-flags",o.record_offset+0x10U,4U,
            "object+0x10 source flags -> 0x140302F10 -> runtime baseline/effective +0x10/+0x14 -> 0x140302640 and 0x1402F9890. Only independently proven bits receive semantics.",
            {"EXE_CONFIRMED","runtime-provenance","source-flags"})) return false;

        const auto selector = runtime::scm_compatibility_object_selector(o.flags);
        const auto vs_base_key = runtime::scm_compatibility_vs_base_key(selector);
        std::ostringstream selector_text;
        selector_text
            << "object+0x10 flag 0x00080000 -> 0x1403033E3..0x1403033FC "
               "runtimeObject+0x00 bit 0x20 -> 0x14030DB50 runtimeObject+0x05 selector "
            << static_cast<unsigned>(selector)
            << " -> 0x14030DBA0 emits 0x5C00000"
            << static_cast<unsigned>(selector)
            << " -> 0x1400331E6 -> 0x140044310. Selector "
            << static_cast<unsigned>(selector)
            << " maps to VS base key " << static_cast<unsigned>(vs_base_key)
            << "; selectors 2 and 3 both map to base key 13. The path continues "
               "through 0x140045FE0 to 0x14004C340 VSSetShader and 0x14004C140 "
               "PSSetShader. Pixel-shader key variants remain mesh/aux-state dependent.";
        if (!add(d,p+"-compat-selector",o.record_offset+0x10U,4U,
            selector_text.str(),
            {"EXE_CONFIRMED","runtime-provenance","compatibility-selector","vertex-shader","pixel-shader","d3d11"})) return false;

        const auto tex1 = legacy_gs_tex1_filter_from_object_flags(o.flags);
        const bool nearest = tex1 == legacy_gs_tex1_nearest_filter;
        std::ostringstream tex1_text;
        tex1_text
            << "object+0x10 flag 0x00004000 -> 0x1402F9890 legacy GS TEX1 state "
            << (nearest ? "0x00 nearest" : "0x60 linear")
            << " -> SCM material PACKED A+D packet register 0x14 (TEX1_1) -> common compatibility GS-state store. The live D3D11 sampler bridge is separately driven from active CLAMP/context state; no direct TEX1-to-D3D11 filter projection is promoted.";
        if (!add(d,p+"-tex1-filter",o.record_offset+0x10U,4U,
            tex1_text.str(),
            {"EXE_CONFIRMED","runtime-provenance","legacy-gs","TEX1_1","texture-filter","material-packet","compatibility-state"})) return false;

        if ((o.flags & runtime::source_mask_00200000)!=0U &&
            !add(d,p+"-bit21-negative",o.record_offset+0x10U,4U,
            "Bit 0x00200000 remains semantically undecoded. Rejected provenance includes 0x1402F4C21 runtimeRecord+0x304, 0x140303F2F manager+0xE0, 0x140302CF9/0x140302D59 manager setters, fresh 0x1402F2CDD on 0x380-stride records, and fresh 0x140212B08 on unrelated +0x3FF8 actor/global state. Preserve exactly.",
            {"PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","bit21"})) return false;

        for (std::size_t mi=0; mi<o.meshes.size(); ++mi) {
            const auto& m=o.meshes[mi];
            const auto q=index_id("scm-prov-mesh",oi,mi);
            if (!add(d,q+"-texture",m.record_offset+2U,2U,
                "mesh+0x02 -> 0x1402F9890 selects external companion record index*0x40; record+0x20 supplies TEX0 to the SCM PACKED A+D TEX0_1 packet. 0x14002CD60 stores TEX0_1 in active compatibility GS state 0x1405D9200; 0x14002BC30 selects TEX0_1/TEX0_2 and 0x14002CC10 extracts TEX0.TBP0 low14 as the resource key. Companion texture resources are registered in the same 0x1405E1830 table by record+0x06 gs_base_pointer_units/TBP0 via 0x140331970 -> 0x140033370. Lookup 0x140033350 returns the texture object; virtual slot +0x08 is 0x140046900 -> 0x140046910 -> 0x14004C290 PSSetShaderResources, followed by 0x1400469C0 -> 0x14004C1D0 PSSetSamplers. Missing lookup clears PS SRV slot 0.",
                {"EXE_CONFIRMED","runtime-provenance","texture-binding","external-companion","TEX0_1","TEX0_TBP0","resource-registry","PSSetShaderResources","PSSetSamplers","d3d11"}) ||
                !add(d,q+"-gs-clamp",m.record_offset+4U,8U,
                "mesh+0x04..+0x0B -> 0x1402F9890 -> legacy GS CLAMP REGION_REPEAT -> SCM PACKED A+D CLAMP_1 -> common compatibility GS-state slot 0x1405D9210. 0x14002BC30 reads active CLAMP_1/2 WMS/WMT and projects modes 1/2 to D3D11 AddressU/V=CLAMP (3), while modes 0/3 retain WRAP (1); 0x1400469C0 writes D3D11_SAMPLER_DESC and 0x14004C1D0 calls PSSetSamplers. REGION_REPEAT mode 3 therefore has no direct native D3D11 address-mode equivalent in this bridge; serialized MIN/MAX remain preserved and no global-unused claim is made.",
                {"EXE_CONFIRMED","runtime-provenance","legacy-gs","REGION_REPEAT","CLAMP_1","material-packet","D3D11_SAMPLER_DESC","PSSetSamplers","bounded-projection"}) ||
                !add(d,q+"-reserved0c-negative",m.record_offset+0x0CU,4U,
                "mesh+0x0C is corpus-zero. Deep canonical chain 0x1402F9BB0 -> 0x140308C00 -> 0x1402F9890 -> 0x1402F9A80 plus SCM-specific 0x1402F9F20 exposes no provenance-clean read. Preserve exactly; not global padding.",
                {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","DEEP_NEGATIVE_EVIDENCE","runtime-provenance"}) ||
                !add(d,q+"-position-offset",m.record_offset+0x10U,8U,
                "position stream -> 0x1402F9BB0 -> float3 runtime position stream.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","geometry"}) ||
                !add(d,q+"-normal-offset",m.record_offset+0x18U,8U,
                "normal stream -> 0x1402F9BB0 -> float3 runtime normal stream.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","geometry"}) ||
                !add(d,q+"-uv-offset",m.record_offset+0x20U,8U,
                "UV stream -> 0x1402F9BB0 -> signed i16 components scaled by 1/4096.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","uv"}) ||
                !add(d,q+"-continuation",m.record_offset+0x28U,8U,
                "continuation span -> 0x1403051B0 physical mesh-chain walk; non-final=0x50, final=0.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","layout"}) ||
                !add(d,q+"-reserved30-negative",m.record_offset+0x30U,8U,
                "mesh+0x30 is corpus-zero and has no provenance-clean read across primary materialization, post-init, material, follow-up and SCM-specific allocation paths.",
                {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","DEEP_NEGATIVE_EVIDENCE","runtime-provenance"}) ||
                !add(d,q+"-topology-offset",m.record_offset+0x38U,8U,
                "RGB/topology stream -> 0x1403051B0; topology bit 0x02 breaks/skips triangle-strip run -> generated u16 indices.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","topology"}) ||
                !add(d,q+"-workspace",m.record_offset+0x40U,12U,
                "mesh-relative workspace/count -> 0x1403051B0 generated u16 indices; capacity align16(6*(vertexCount-2)).",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","generated-index-workspace"}) ||
                !add(d,q+"-reserved4c-negative",m.record_offset+0x4CU,4U,
                "serialized mesh+0x4C is corpus-zero with no provenance-clean read across the deep canonical mesh chain. Runtime mesh +0x4C exists but is produced from runtime-object state and is not serialized +0x4C evidence.",
                {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","DEEP_NEGATIVE_EVIDENCE","runtime-provenance"})) return false;
            if (!m.colors_topology.empty() && !add(d,q+"-topology-stream",m.color_flags_offset,
                static_cast<std::uint64_t>(m.colors_topology.size())*4U,
                "Vertex RGB/topology payload consumed by 0x1403051B0. Retail census observes only topology byte values 0 and 2; only bit 0x02 is promoted.",
                {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","topology","payload"})) return false;
        }
    }
    return true;
}

[[nodiscard]] bool add_scene_provenance(binary::Document& d,
    const ParseResult& parsed) {
    const auto& s=parsed.document.scene_nodes;
    if (!add(d,"scm-prov-scene-arrays",s.offset,0x10U,
        "four relative arrays -> 0x140303C10 -> 0x1402F1DB0 -> runtime hierarchy/object-binding/transform domain.",
        {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","scene","hierarchy"}) ||
        !add(d,"scm-prov-scene-shell-negative",s.offset+0x10U,0x10U,
        "scene+0x10..+0x1F is corpus-zero; recovered path has no live promoted consumer. Preserve exactly.",
        {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","scene"})) return false;
    const auto base=s.offset+static_cast<std::uint64_t>(s.transform_rel);
    for (std::size_t i=0;i<s.transform_by_node_index.size();++i) {
        const auto off=base+static_cast<std::uint64_t>(i)*scene_transform_size;
        const auto p=index_id("scm-prov-transform",i);
        if (!add(d,p+"-translation",off,12U,
            "translation XYZ -> 0x1402FA360 -> 0x140031200 -> world update 0x1402F9700.",
            {"EXE_CONFIRMED","runtime-provenance","transform","translation"}) ||
            !add(d,p+"-magnitude",off+0x0CU,4U,
            "precomputed translation length; excluded from homogeneous W construction.",
            {"EXE_AND_CORPUS_CONFIRMED","runtime-provenance","transform","derived-scalar"}) ||
            !add(d,p+"-rotation",off+0x10U,12U,
            "XYZ radians -> 0x1402FA360 -> 0x140330450 -> Rz*Ry*Rx -> 0x1402F9700 world composition.",
            {"EXE_CONFIRMED","runtime-provenance","transform","rotation","world"}) ||
            !add(d,p+"-reserved1c-negative",off+0x1CU,4U,
            "transform+0x1C is corpus-zero and outside confirmed SCM matrix construction. Preserve exact.",
            {"RESERVED_OBSERVED_ZERO","PRESERVED_UNDECODED","BOUNDED_NEGATIVE_EVIDENCE","runtime-provenance","transform"})) return false;
    }
    return true;
}

} // namespace

bool annotate_runtime_provenance(binary::Document& document,
    const ParseResult& parsed) {
    if (!parsed.recognized || !parsed.ok() ||
        document.byte_size()!=parsed.document.source_bytes.size()) return false;
    return add_header_provenance(document) &&
        add_object_provenance(document,parsed) &&
        add_scene_provenance(document,parsed);
}

std::optional<binary::Document> build_deep_binary_document(
    gdspaces::ResourceRef resource, std::span<const std::byte> bytes,
    const ParseResult& parsed) {
    auto document=build_binary_document(std::move(resource),bytes,parsed);
    if (!document.has_value()) return std::nullopt;
    if (!annotate_runtime_provenance(*document,parsed)) return std::nullopt;
    return document;
}

} // namespace dmc::rengine::formats::scm
