#include "dmc_rengine/formats/scm_runtime_provenance.hpp"

#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/formats/scm_runtime_flags.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::scm {
namespace {

inline constexpr std::string_view evidence_id =
    "dmc3-scm-runtime-provenance-2026-09-13";

[[nodiscard]] std::string index_id(
    std::string_view prefix,
    std::size_t outer,
    std::size_t inner = static_cast<std::size_t>(-1)) {
    std::ostringstream out;
    out << prefix << '-' << std::setfill('0') << std::setw(3) << outer;
    if (inner != static_cast<std::size_t>(-1)) {
        out << '-' << std::setw(3) << inner;
    }
    return out.str();
}

[[nodiscard]] bool add(
    binary::Document& document,
    std::string id,
    std::uint64_t offset,
    std::uint64_t size,
    std::string text,
    std::vector<std::string> tags) {
    if (size == 0U) return true;
    return document.add_annotation(binary::Annotation{
        .id = std::move(id),
        .range = {.offset = offset, .size = size},
        .text = std::move(text),
        .evidence_id = std::string(evidence_id),
        .tags = std::move(tags),
    });
}

[[nodiscard]] bool add_header_provenance(binary::Document& document) {
    return
        add(document, "scm-prov-header-object-count", 0x10U, 1U,
            "L0 SCM+0x10 -> L1 0x1402F9570 -> L2 manager+0xE8 object count.",
            {"EXE_CONFIRMED", "runtime-provenance", "L0", "L1", "L2"}) &&
        add(document, "scm-prov-header-node-count", 0x11U, 1U,
            "L0 SCM+0x11 -> L1 0x1402F9570 -> L2 manager+0xEA scene-node count -> L3 0x1402F1DB0 node domain.",
            {"EXE_CONFIRMED", "runtime-provenance", "L0", "L1", "L2", "L3"}) &&
        add(document, "scm-prov-header-texture-mirror", 0x12U, 1U,
            "SCM+0x12 is a serialized texture-slot mirror, not live runtime authority. manager+0x110 owns the external companion; 0x1402F9570 reads companion+0x00 into manager+0xEC.",
            {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "external-companion", "mirror"}) &&
        add(document, "scm-prov-header-13", 0x13U, 1U,
            "L0 SCM+0x13 -> L1 0x1402F9570 -> L2 manager+0xFA. Runtime-carried but no stronger semantic consumer is proven.",
            {"PRESERVED_UNDECODED", "runtime-provenance", "runtime-carried"}) &&
        add(document, "scm-prov-header-resource-code", 0x14U, 4U,
            "L0 legacy decimal resource/provenance code -> L1 0x1402F9570 -> L2 manager+0xE4. Arithmetic decomposition is confirmed; official family labels are not.",
            {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "identity"}) &&
        add(document, "scm-prov-header-scene-offset", 0x20U, 8U,
            "L0 scene-node block offset -> SCM setup 0x140303C10 -> L2/L3 common node binder 0x1402F1DB0.",
            {"STRUCTURAL_CONFIRMED", "runtime-provenance", "scene"});
}

[[nodiscard]] bool add_object_provenance(
    binary::Document& document,
    const ParseResult& parsed) {
    for (std::size_t oi = 0U; oi < parsed.document.objects.size(); ++oi) {
        const auto& object = parsed.document.objects[oi];
        const auto prefix = index_id("scm-prov-object", oi);

        if (!add(document, prefix + "-alpha", object.record_offset + 0x01U, 1U,
                "L0 object+0x01 -> 0x14030303A/SCM initializer 0x140302F10 -> runtime+0x07 -> runtime+0x178/+0x17C -> 0x140304111..0x140304167 -> MDL_PARTS_COLOR_PKT.alpha.w -> DMC3_STG COLOR0.a.",
                {"EXE_CONFIRMED", "runtime-provenance", "alpha-control", "shader-visible", "L0", "L1", "L2", "L3", "L4"})) {
            return false;
        }

        if (!add(document, prefix + "-flags", object.record_offset + 0x10U, 4U,
                "L0 object+0x10 source flags -> SCM initializer 0x140302F10 -> runtime baseline/effective +0x10/+0x14 -> 0x140302640 render-state helper and 0x1402F9890 material helper. Only independently proven bits receive semantics.",
                {"EXE_CONFIRMED", "runtime-provenance", "source-flags", "L0", "L1", "L2", "L3"})) {
            return false;
        }

        if ((object.flags & runtime::source_mask_00200000) != 0U) {
            if (!add(document, prefix + "-bit21-negative", object.record_offset + 0x10U, 4U,
                    "Bit 0x00200000 is source-preserved but semantically undecoded. Rejected provenance: 0x1402F4C21 tests independent runtimeRecord+0x304; 0x140303F2F tests independently produced manager+0xE0; 0x140302CF9/0x140302D59 set manager bit21 from another control word. No semantic editor toggle is justified.",
                    {"PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance", "rejected-candidate", "bit21"})) {
                return false;
            }
        }

        for (std::size_t mi = 0U; mi < object.meshes.size(); ++mi) {
            const auto& mesh = object.meshes[mi];
            const auto mesh_prefix = index_id("scm-prov-mesh", oi, mi);

            if (!add(document, mesh_prefix + "-texture", mesh.record_offset + 0x02U, 2U,
                    "L0 mesh+0x02 texture index -> 0x1402F9890 -> runtimeTextureTable + index*0x40 -> texture record+0x20 -> runtime mesh texture. Table ownership comes from manager+0x110 external companion.",
                    {"EXE_CONFIRMED", "runtime-provenance", "external-companion", "texture-binding"}) ||
                !add(document, mesh_prefix + "-gs-clamp", mesh.record_offset + 0x04U, 8U,
                    "L0 mesh+0x04..+0x0B MINU/MAXU/MINV/MAXV -> 0x1402F9890 -> legacy GS CLAMP packing with WMS=WMT=REGION_REPEAT(3); MINU==0 selects disabled/sentinel packed zero.",
                    {"EXE_CONFIRMED", "runtime-provenance", "legacy-gs", "REGION_REPEAT"}) ||
                !add(document, mesh_prefix + "-reserved0c-negative", mesh.record_offset + 0x0CU, 4U,
                    "mesh+0x0C is zero in bounded corpus; 0x1402F9BB0 primary materialization has no promoted read and 0x1402F9890 consumes preceding CLAMP words then skips this lane. Preserve exactly; do not call global padding.",
                    {"RESERVED_OBSERVED_ZERO", "PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance"}) ||
                !add(document, mesh_prefix + "-position-offset", mesh.record_offset + 0x10U, 8U,
                    "L0 position stream offset -> primary runtime mesh materialization 0x1402F9BB0 -> float3 position stream.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "geometry"}) ||
                !add(document, mesh_prefix + "-normal-offset", mesh.record_offset + 0x18U, 8U,
                    "L0 normal stream offset -> primary runtime mesh materialization 0x1402F9BB0 -> float3 normal stream.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "geometry"}) ||
                !add(document, mesh_prefix + "-uv-offset", mesh.record_offset + 0x20U, 8U,
                    "L0 UV stream offset -> primary runtime mesh materialization 0x1402F9BB0 -> signed i16 UV components scaled by 1/4096.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "uv"}) ||
                !add(document, mesh_prefix + "-continuation", mesh.record_offset + 0x28U, 8U,
                    "L0 continuation span -> post-load normalizer 0x1403051B0 physical mesh-chain walk. Canonical non-final=0x50, final=0.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "layout"}) ||
                !add(document, mesh_prefix + "-reserved30-negative", mesh.record_offset + 0x30U, 8U,
                    "mesh+0x30 is bounded-corpus zero and has no promoted consumer in the recovered normalizer/materializer path. Preserve exact source bytes.",
                    {"RESERVED_OBSERVED_ZERO", "PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance"}) ||
                !add(document, mesh_prefix + "-topology-offset", mesh.record_offset + 0x38U, 8U,
                    "L0 RGB/topology stream offset -> 0x1403051B0 -> topology bit 0x02 breaks/skips a triangle-strip run -> generated u16 index workspace.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "topology"}) ||
                !add(document, mesh_prefix + "-workspace", mesh.record_offset + 0x40U, 12U,
                    "L0 mesh-relative workspace (+0x40) and serialized generated-count (+0x48) -> 0x1403051B0 generates u16 indices and publishes generated word count. Capacity is align16(6*(vertexCount-2)); retail workspace begins with 0x1212 sentinel.",
                    {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "generated-index-workspace"}) ||
                !add(document, mesh_prefix + "-reserved4c-negative", mesh.record_offset + 0x4CU, 4U,
                    "mesh+0x4C is bounded-corpus zero with no promoted semantic in recovered SCM normalizer/materializer paths. Preserve exactly and do not expose a semantic editor control.",
                    {"RESERVED_OBSERVED_ZERO", "PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance"})) {
                return false;
            }

            if (!mesh.colors_topology.empty()) {
                const auto stream_size = static_cast<std::uint64_t>(mesh.colors_topology.size()) * 4U;
                if (!add(document, mesh_prefix + "-topology-stream", mesh.color_flags_offset,
                        stream_size,
                        "Vertex RGB/topology payload consumed by 0x1403051B0. Only topology flag bit 0x02 is promoted; every other observed/unknown topology bit remains preservation-gated.",
                        {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "topology", "payload"})) {
                    return false;
                }
            }
        }
    }
    return true;
}

[[nodiscard]] bool add_scene_provenance(
    binary::Document& document,
    const ParseResult& parsed) {
    const auto& scene = parsed.document.scene_nodes;
    if (!add(document, "scm-prov-scene-arrays", scene.offset, 0x10U,
            "L0 four relative arrays (parent/order/object-binding/transform) -> SCM setup 0x140303C10 -> common node binder 0x1402F1DB0 -> runtime hierarchy/object-binding/transform domain.",
            {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "scene", "hierarchy"}) ||
        !add(document, "scm-prov-scene-shell-negative", scene.offset + 0x10U, 0x10U,
            "Scene header +0x10..+0x1F is bounded-corpus zero. Audited planner reads +0x10 once into a dead local and the shell pointer does not escape the recovered model path. Preserve exactly; dormant/no-effect is not global padding authority.",
            {"RESERVED_OBSERVED_ZERO", "PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance", "scene"})) {
        return false;
    }

    const auto transform_base =
        scene.offset + static_cast<std::uint64_t>(scene.transform_rel);
    for (std::size_t ni = 0U;
         ni < scene.transform_by_node_index.size(); ++ni) {
        const auto offset = transform_base +
            static_cast<std::uint64_t>(ni) * scene_transform_size;
        const auto prefix = index_id("scm-prov-transform", ni);

        if (!add(document, prefix + "-translation", offset + 0x00U, 12U,
                "L0 translation XYZ -> SCM local initializer 0x1402FA360 -> translation helper 0x140031200 -> world update 0x1402F9700.",
                {"EXE_CONFIRMED", "runtime-provenance", "transform", "translation"}) ||
            !add(document, prefix + "-magnitude", offset + 0x0CU, 4U,
                "L0 precomputed Euclidean translation magnitude. Confirmed SCM matrix construction does not treat this lane as homogeneous translation W.",
                {"EXE_AND_CORPUS_CONFIRMED", "runtime-provenance", "transform", "derived-scalar"}) ||
            !add(document, prefix + "-rotation", offset + 0x10U, 12U,
                "L0 XYZ radians -> SCM initializer 0x1402FA360 -> rotation helper 0x140330450 -> Rz*Ry*Rx local rotation -> 0x1402F9700 world = local * parentOrRootWorld.",
                {"EXE_CONFIRMED", "runtime-provenance", "transform", "rotation", "world"}) ||
            !add(document, prefix + "-reserved1c-negative", offset + 0x1CU, 4U,
                "Transform+0x1C is bounded-corpus zero and is outside confirmed SCM matrix construction in 0x1402FA360. Preserve exact; no transform semantic is promoted.",
                {"RESERVED_OBSERVED_ZERO", "PRESERVED_UNDECODED", "BOUNDED_NEGATIVE_EVIDENCE", "runtime-provenance", "transform"})) {
            return false;
        }
    }
    return true;
}

} // namespace

bool annotate_runtime_provenance(
    binary::Document& document,
    const ParseResult& parsed) {
    if (!parsed.recognized || !parsed.ok() ||
        document.byte_size() != parsed.document.source_bytes.size()) {
        return false;
    }

    return add_header_provenance(document) &&
        add_object_provenance(document, parsed) &&
        add_scene_provenance(document, parsed);
}

std::optional<binary::Document> build_deep_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ParseResult& parsed) {
    auto document = build_binary_document(std::move(resource), bytes, parsed);
    if (!document.has_value()) return std::nullopt;
    if (!annotate_runtime_provenance(*document, parsed)) return std::nullopt;
    return document;
}

} // namespace dmc::rengine::formats::scm
