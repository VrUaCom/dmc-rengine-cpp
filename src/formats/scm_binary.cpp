#include "dmc_rengine/formats/scm_binary.hpp"

#include "dmc_rengine/formats/scm_runtime_flags.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::scm {
namespace {

inline constexpr std::string_view evidence_id =
    "dmc3-scm-canonical-reverse-2026-09-13";

[[nodiscard]] std::string index_id(std::string_view prefix, std::size_t index) {
    std::ostringstream out;
    out << prefix << '-' << std::setfill('0') << std::setw(3) << index;
    return out.str();
}

[[nodiscard]] std::string nested_id(
    std::string_view prefix, std::size_t outer, std::size_t inner) {
    std::ostringstream out;
    out << prefix << '-' << std::setfill('0') << std::setw(3) << outer
        << '-' << std::setw(3) << inner;
    return out.str();
}

[[nodiscard]] std::string dec(std::uint64_t value) {
    return std::to_string(value);
}
[[nodiscard]] std::string signed_dec(std::int64_t value) {
    return std::to_string(value);
}
[[nodiscard]] std::string hex(std::uint64_t value, std::size_t width = 8U) {
    std::ostringstream out;
    out << "0x" << std::uppercase << std::hex << std::setfill('0')
        << std::setw(static_cast<int>(width)) << value;
    return out.str();
}
[[nodiscard]] std::string f32(float value) {
    std::ostringstream out;
    out << std::setprecision(9) << value;
    return out.str();
}

bool add_field(binary::Document& document, std::string id, std::string name,
    std::uint64_t offset, std::uint64_t size, binary::FieldKind kind,
    std::string type, std::string value, std::string parent = {}) {
    return document.add_field(binary::Field{
        .id = std::move(id),
        .name = std::move(name),
        .range = {.offset = offset, .size = size},
        .kind = kind,
        .type_name = std::move(type),
        .display_value = std::move(value),
        .parent_id = std::move(parent),
        .evidence_id = std::string(evidence_id),
    });
}

bool add_region(binary::Document& document, std::string id, std::string name,
    std::uint64_t offset, std::uint64_t size, binary::RegionKind kind,
    std::string type) {
    if (size == 0U) return true;
    return document.add_region(binary::Region{
        .id = std::move(id), .name = std::move(name),
        .range = {.offset = offset, .size = size}, .kind = kind,
        .type_name = std::move(type), .evidence_id = std::string(evidence_id)});
}

void add_owner(binary::Document& document, std::string owner,
    std::uint64_t offset, std::uint64_t size, std::string rationale) {
    if (size == 0U) return;
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = std::move(owner), .range = {.offset = offset, .size = size},
        .rationale = std::move(rationale)}));
}

void annotate(binary::Document& document, std::string id,
    std::uint64_t offset, std::uint64_t size, std::string text,
    std::vector<std::string> tags) {
    if (size == 0U) return;
    static_cast<void>(document.add_annotation(binary::Annotation{
        .id = std::move(id), .range = {.offset = offset, .size = size},
        .text = std::move(text), .evidence_id = std::string(evidence_id),
        .tags = std::move(tags)}));
}

bool add_vec3(binary::Document& document, std::string id, std::string name,
    std::uint64_t offset, const Vec3f& value, std::string parent) {
    const auto root = id;
    return add_field(document, root, std::move(name), offset, 12U,
               binary::FieldKind::structure, "Vec3f", {}, parent) &&
        add_field(document, root + "-x", "X", offset, 4U,
               binary::FieldKind::floating_point, "f32_le", f32(value.x), root) &&
        add_field(document, root + "-y", "Y", offset + 4U, 4U,
               binary::FieldKind::floating_point, "f32_le", f32(value.y), root) &&
        add_field(document, root + "-z", "Z", offset + 8U, 4U,
               binary::FieldKind::floating_point, "f32_le", f32(value.z), root);
}

[[nodiscard]] std::string resource_code_text(const LegacyResourceCode& code) {
    std::ostringstream out;
    out << code.raw << " (family=" << static_cast<unsigned>(code.family_class)
        << ", model_set=" << code.model_set
        << ", sub_index=" << static_cast<unsigned>(code.sub_index) << ')';
    return out.str();
}

[[nodiscard]] std::string flags_text(std::uint32_t flags) {
    const auto projection = runtime::project(flags);
    std::ostringstream out;
    out << hex(flags) << " -> runtime_set=" << hex(projection.runtime_flags_to_set)
        << ", helper_mode=" << static_cast<unsigned>(projection.helper_mode)
        << ", helper_selector=" << hex(projection.helper_state_selector);
    if (projection.preserved_undecoded_source_bits != 0U) {
        out << ", preserved_undecoded="
            << hex(projection.preserved_undecoded_source_bits);
    }
    return out.str();
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource, std::span<const std::byte> bytes,
    const ParseResult& parsed) {
    if (!resource.valid() || resource.id.size != bytes.size() ||
        !parsed.recognized || !parsed.ok() || bytes.size() < header_size) {
        return std::nullopt;
    }

    binary::Document document(std::move(resource), bytes.size());
    const auto& h = parsed.document.header;

    if (!add_region(document, "scm-header", "SCM header", 0U, header_size,
            binary::RegionKind::header, "ScmHeader") ||
        !add_field(document, "scm-header-struct", "SCM header structure", 0U,
            header_size, binary::FieldKind::structure, "ScmHeader", {}) ||
        !add_field(document, "scm-magic", "Magic", 0U, 4U,
            binary::FieldKind::string, "char[4]", "SCM ", "scm-header-struct") ||
        !add_field(document, "scm-version", "Version", 0x04U, 4U,
            binary::FieldKind::floating_point, "f32_le", f32(h.version),
            "scm-header-struct") ||
        !add_field(document, "scm-header-reserved08", "Preserved header +0x08",
            0x08U, 8U, binary::FieldKind::unknown, "u64_le preservation-only",
            hex(h.reserved08, 16U), "scm-header-struct") ||
        !add_field(document, "scm-object-count", "Object count", 0x10U, 1U,
            binary::FieldKind::unsigned_integer, "u8", dec(h.object_count),
            "scm-header-struct") ||
        !add_field(document, "scm-scene-node-count", "Scene-node count", 0x11U,
            1U, binary::FieldKind::unsigned_integer, "u8", dec(h.scene_node_count),
            "scm-header-struct") ||
        !add_field(document, "scm-texture-slot-count", "Texture-slot mirror count",
            0x12U, 1U, binary::FieldKind::unsigned_integer, "u8",
            dec(h.texture_slot_count), "scm-header-struct") ||
        // Keep the historical field id for manifest compatibility; semantics are
        // no longer preservation-only.
        !add_field(document, "scm-header-reserved13", "Lighting reference node index",
            0x13U, 1U, binary::FieldKind::unsigned_integer,
            "u8 scene-node index", dec(h.reserved13), "scm-header-struct") ||
        !add_field(document, "scm-resource-code", "Legacy resource code", 0x14U,
            4U, binary::FieldKind::unsigned_integer, "u32_le decimal structural code",
            resource_code_text(h.resource_code), "scm-header-struct") ||
        !add_field(document, "scm-header-reserved18", "Preserved header +0x18",
            0x18U, 8U, binary::FieldKind::unknown, "u64_le preservation-only",
            hex(h.reserved18, 16U), "scm-header-struct") ||
        !add_field(document, "scm-scene-block-offset", "Scene-node block offset",
            0x20U, 8U, binary::FieldKind::pointer, "u64_le file offset",
            hex(h.scene_node_block_offset, 16U), "scm-header-struct") ||
        !add_field(document, "scm-header-reserved28", "Preserved header +0x28",
            0x28U, 8U, binary::FieldKind::unknown, "u64_le preservation-only",
            hex(h.reserved28, 16U), "scm-header-struct") ||
        !add_field(document, "scm-header-reserved30", "Preserved header +0x30",
            0x30U, 8U, binary::FieldKind::unknown, "u64_le preservation-only",
            hex(h.reserved30, 16U), "scm-header-struct") ||
        !add_field(document, "scm-header-reserved38", "Preserved header +0x38",
            0x38U, 8U, binary::FieldKind::unknown, "u64_le preservation-only",
            hex(h.reserved38, 16U), "scm-header-struct")) return std::nullopt;

    add_owner(document, "formats.scm.header", 0U, header_size,
              "Canonical 0x40 SCM header; typed and preservation-only fields stay visible.");
    annotate(document, "scm-header-preservation-08", 0x08U, 8U,
             "PRESERVED_UNDECODED: corpus-zero header lane with no promoted typed runtime effect.",
             {"PRESERVED_UNDECODED", "header"});
    annotate(document, "scm-header-lighting-reference", 0x13U, 1U,
             "EXE_CONFIRMED: scene-node index selecting the world-space reference used to build MDL_LIGHT_MAT Lc/Lv for CDrawSCM lighting.",
             {"EXE_CONFIRMED", "lighting", "scene-node", "shader-constant"});
    annotate(document, "scm-header-preservation-18", 0x18U, 8U,
             "PRESERVED_UNDECODED: bounded source-pointer census found no promoted typed consumer.",
             {"PRESERVED_UNDECODED", "header"});
    annotate(document, "scm-header-preservation-28", 0x28U, 0x18U,
             "PRESERVED_UNDECODED: terminal header shell +0x28..+0x3F.",
             {"PRESERVED_UNDECODED", "header"});

    for (std::size_t oi = 0; oi < parsed.document.objects.size(); ++oi) {
        const auto& object = parsed.document.objects[oi];
        const auto object_id = index_id("scm-object", oi);
        const auto object_struct = object_id + "-struct";
        if (!add_region(document, object_id, "SCM object record",
                object.record_offset, object_record_size,
                binary::RegionKind::record, "ScmObject") ||
            !add_field(document, object_struct, "Object structure",
                object.record_offset, object_record_size,
                binary::FieldKind::structure, "ScmObject", {}) ||
            !add_field(document, object_id + "-mesh-count", "Mesh count",
                object.record_offset, 1U, binary::FieldKind::unsigned_integer,
                "u8", dec(object.mesh_count), object_struct) ||
            !add_field(document, object_id + "-alpha-control", "Alpha control",
                object.record_offset + 1U, 1U, binary::FieldKind::unsigned_integer,
                "u8 control", dec(object.alpha_control), object_struct) ||
            !add_field(document, object_id + "-vertex-count", "Total vertex count",
                object.record_offset + 2U, 2U, binary::FieldKind::unsigned_integer,
                "u16_le", dec(object.total_vertex_count), object_struct) ||
            !add_field(document, object_id + "-reserved04", "Preserved object +0x04",
                object.record_offset + 4U, 4U, binary::FieldKind::unknown,
                "u32_le preservation-only", hex(object.reserved04), object_struct) ||
            !add_field(document, object_id + "-mesh-table", "Mesh-table offset",
                object.record_offset + 8U, 8U, binary::FieldKind::pointer,
                "u64_le file offset", hex(object.mesh_table_offset, 16U), object_struct) ||
            !add_field(document, object_id + "-flags", "Source object flags",
                object.record_offset + 0x10U, 4U, binary::FieldKind::unsigned_integer,
                "u32_le bitfield", flags_text(object.flags), object_struct) ||
            !add_field(document, object_id + "-preserved14-2f",
                "Preserved object +0x14..+0x2F", object.record_offset + 0x14U,
                object.reserved14_2f.size(), binary::FieldKind::unknown,
                "byte[0x1C] preservation-only", {}, object_struct) ||
            !add_vec3(document, object_id + "-bounds-center", "Bounding center",
                object.record_offset + 0x30U, object.bounding_center, object_struct) ||
            !add_field(document, object_id + "-bounds-radius", "Bounding radius",
                object.record_offset + 0x3CU, 4U, binary::FieldKind::floating_point,
                "f32_le", f32(object.bounding_radius), object_struct)) return std::nullopt;

        add_owner(document, "formats.scm.objects", object.record_offset,
                  object_record_size, "Canonical fixed-stride 0x40 SCM object record.");
        annotate(document, object_id + "-preservation-04", object.record_offset + 4U,
                 4U, "PRESERVED_UNDECODED: corpus-zero object lane +0x04.",
                 {"PRESERVED_UNDECODED", "object"});
        annotate(document, object_id + "-preservation-14-2f",
                 object.record_offset + 0x14U, object.reserved14_2f.size(),
                 "PRESERVED_UNDECODED: object secondary shell +0x14..+0x2F.",
                 {"PRESERVED_UNDECODED", "object"});
        if ((object.flags & runtime::source_mask_00200000) != 0U) {
            annotate(document, object_id + "-flag-bit21", object.record_offset + 0x10U,
                     4U,
                     "PRESERVED_UNDECODED bit 0x00200000: real runtime-carried state with no provenance-clean terminal semantic consumer.",
                     {"PRESERVED_UNDECODED", "runtime-carried", "source-flags"});
        }

        for (std::size_t mi = 0; mi < object.meshes.size(); ++mi) {
            const auto& mesh = object.meshes[mi];
            const auto mesh_id = nested_id("scm-mesh", oi, mi);
            const auto mesh_struct = mesh_id + "-struct";
            if (!add_region(document, mesh_id, "SCM mesh record", mesh.record_offset,
                    mesh_record_size, binary::RegionKind::record, "ScmMesh") ||
                !add_field(document, mesh_struct, "Mesh structure", mesh.record_offset,
                    mesh_record_size, binary::FieldKind::structure, "ScmMesh", {}) ||
                !add_field(document, mesh_id + "-vertex-count", "Vertex count",
                    mesh.record_offset, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le", dec(mesh.vertex_count), mesh_struct) ||
                !add_field(document, mesh_id + "-texture-index", "Texture companion slot",
                    mesh.record_offset + 2U, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le slot", dec(mesh.texture_index), mesh_struct) ||
                !add_field(document, mesh_id + "-clamp-minu", "GS CLAMP MINU",
                    mesh.record_offset + 4U, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le", dec(mesh.gs_clamp_region_repeat.min_u), mesh_struct) ||
                !add_field(document, mesh_id + "-clamp-maxu", "GS CLAMP MAXU",
                    mesh.record_offset + 6U, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le", dec(mesh.gs_clamp_region_repeat.max_u), mesh_struct) ||
                !add_field(document, mesh_id + "-clamp-minv", "GS CLAMP MINV",
                    mesh.record_offset + 8U, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le", dec(mesh.gs_clamp_region_repeat.min_v), mesh_struct) ||
                !add_field(document, mesh_id + "-clamp-maxv", "GS CLAMP MAXV",
                    mesh.record_offset + 0x0AU, 2U, binary::FieldKind::unsigned_integer,
                    "u16_le", dec(mesh.gs_clamp_region_repeat.max_v), mesh_struct) ||
                !add_field(document, mesh_id + "-reserved0c", "Preserved mesh +0x0C",
                    mesh.record_offset + 0x0CU, 4U, binary::FieldKind::unknown,
                    "u32_le preservation-only", hex(mesh.reserved0c), mesh_struct) ||
                !add_field(document, mesh_id + "-positions", "Positions offset",
                    mesh.record_offset + 0x10U, 8U, binary::FieldKind::pointer,
                    "u64_le file offset", hex(mesh.positions_offset, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-normals", "Normals offset",
                    mesh.record_offset + 0x18U, 8U, binary::FieldKind::pointer,
                    "u64_le file offset", hex(mesh.normals_offset, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-uv", "UV offset",
                    mesh.record_offset + 0x20U, 8U, binary::FieldKind::pointer,
                    "u64_le file offset", hex(mesh.uv_offset, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-continuation", "Continuation span",
                    mesh.record_offset + 0x28U, 8U, binary::FieldKind::unsigned_integer,
                    "u64_le", hex(mesh.continuation_span, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-reserved30", "Preserved mesh +0x30",
                    mesh.record_offset + 0x30U, 8U, binary::FieldKind::unknown,
                    "u64_le preservation-only", hex(mesh.reserved30, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-color-topology", "Color/topology offset",
                    mesh.record_offset + 0x38U, 8U, binary::FieldKind::pointer,
                    "u64_le file offset", hex(mesh.color_flags_offset, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-workspace-rel", "Index workspace relative offset",
                    mesh.record_offset + 0x40U, 8U, binary::FieldKind::pointer,
                    "u64_le relative to mesh record",
                    hex(mesh.index_workspace_relative_offset, 16U), mesh_struct) ||
                !add_field(document, mesh_id + "-generated-index-count", "Generated index count",
                    mesh.record_offset + 0x48U, 4U, binary::FieldKind::unsigned_integer,
                    "u32_le runtime-normalized", dec(mesh.generated_index_count), mesh_struct) ||
                !add_field(document, mesh_id + "-reserved4c", "Preserved mesh +0x4C",
                    mesh.record_offset + 0x4CU, 4U, binary::FieldKind::unknown,
                    "u32_le preservation-only", hex(mesh.reserved4c), mesh_struct)) return std::nullopt;

            add_owner(document, "formats.scm.mesh-records", mesh.record_offset,
                      mesh_record_size, "Canonical fixed-stride 0x50 SCM mesh record.");
            annotate(document, mesh_id + "-preservation-0c", mesh.record_offset + 0x0CU,
                     4U, "PRESERVED_UNDECODED: mesh +0x0C remains unconsumed after deep canonical mesh-path census.",
                     {"PRESERVED_UNDECODED", "mesh"});
            annotate(document, mesh_id + "-preservation-30", mesh.record_offset + 0x30U,
                     8U, "PRESERVED_UNDECODED: mesh +0x30 remains unconsumed after deep canonical mesh-path census.",
                     {"PRESERVED_UNDECODED", "mesh"});
            annotate(document, mesh_id + "-preservation-4c", mesh.record_offset + 0x4CU,
                     4U, "PRESERVED_UNDECODED: serialized mesh +0x4C remains unconsumed; unrelated runtime +0x4C fields are rejected by provenance.",
                     {"PRESERVED_UNDECODED", "mesh"});

            const auto vertex_count = static_cast<std::uint64_t>(mesh.vertex_count);
            const auto pos_size = vertex_count * 12U;
            const auto uv_size = vertex_count * 4U;
            if (!add_region(document, mesh_id + "-positions-stream", "Position stream",
                    mesh.positions_offset, pos_size, binary::RegionKind::payload,
                    "f32x3[vertex_count]") ||
                !add_region(document, mesh_id + "-normals-stream", "Normal stream",
                    mesh.normals_offset, pos_size, binary::RegionKind::payload,
                    "f32x3[vertex_count]") ||
                !add_region(document, mesh_id + "-uv-stream", "UV stream",
                    mesh.uv_offset, uv_size, binary::RegionKind::payload,
                    "s16x2[vertex_count] / 4096") ||
                !add_region(document, mesh_id + "-color-topology-stream", "RGB/topology stream",
                    mesh.color_flags_offset, uv_size, binary::RegionKind::payload,
                    "u8x4[vertex_count]")) return std::nullopt;

            add_owner(document, "formats.scm.positions", mesh.positions_offset, pos_size,
                      "EXE-confirmed float3 position stream.");
            add_owner(document, "formats.scm.normals", mesh.normals_offset, pos_size,
                      "EXE-confirmed float3 normal stream.");
            add_owner(document, "formats.scm.uv", mesh.uv_offset, uv_size,
                      "EXE-confirmed signed fixed-point UV stream (1/4096).");
            add_owner(document, "formats.scm.color-topology", mesh.color_flags_offset, uv_size,
                      "RGB plus topology-control byte; bit 0x02 breaks/restarts triangle runs.");
            annotate(document, mesh_id + "-topology-contract", mesh.color_flags_offset,
                     uv_size,
                     "Fourth byte per vertex is topology control, not alpha; canonical bit 0x02 is the triangle-run break/skip condition.",
                     {"EXE_CONFIRMED", "topology", "triangle-strip"});

            if (mesh.index_workspace_capacity != 0U) {
                if (!add_region(document, mesh_id + "-index-workspace", "Index workspace",
                        mesh.index_workspace_offset, mesh.index_workspace_capacity,
                        binary::RegionKind::payload, "u16 workspace") ||
                    !add_field(document, mesh_id + "-workspace-sentinel",
                        "Serialized workspace sentinel", mesh.index_workspace_offset,
                        2U, binary::FieldKind::unsigned_integer, "u16_le",
                        hex(index_workspace_sentinel, 4U))) return std::nullopt;
                add_owner(document, "formats.scm.index-workspace",
                          mesh.index_workspace_offset, mesh.index_workspace_capacity,
                          "Reserved capacity for runtime-generated triangle indices; serialized source begins with 0x1212 in confirmed corpus.");
            }
        }
    }

    const auto& scene = parsed.document.scene_nodes;
    if (!add_region(document, "scm-scene-header", "SCM scene-node block header",
            scene.offset, scene_block_header_size, binary::RegionKind::header,
            "ScmSceneNodeBlockHeader") ||
        !add_field(document, "scm-scene-header-struct", "Scene-node block header",
            scene.offset, scene_block_header_size, binary::FieldKind::structure,
            "ScmSceneNodeBlockHeader", {}) ||
        !add_field(document, "scm-scene-parent-rel", "Parent array relative offset",
            scene.offset, 4U, binary::FieldKind::pointer,
            "u32_le relative to scene block", hex(scene.parent_rel), "scm-scene-header-struct") ||
        !add_field(document, "scm-scene-order-rel", "Order array relative offset",
            scene.offset + 4U, 4U, binary::FieldKind::pointer,
            "u32_le relative to scene block", hex(scene.order_rel), "scm-scene-header-struct") ||
        !add_field(document, "scm-scene-binding-rel", "Object-binding array relative offset",
            scene.offset + 8U, 4U, binary::FieldKind::pointer,
            "u32_le relative to scene block", hex(scene.object_binding_rel), "scm-scene-header-struct") ||
        !add_field(document, "scm-scene-transform-rel", "Transform array relative offset",
            scene.offset + 0x0CU, 4U, binary::FieldKind::pointer,
            "u32_le relative to scene block", hex(scene.transform_rel), "scm-scene-header-struct") ||
        !add_field(document, "scm-scene-preserved10-1f", "Preserved scene +0x10..+0x1F",
            scene.offset + 0x10U, scene.reserved10_1f.size(), binary::FieldKind::unknown,
            "byte[0x10] preservation-only", {}, "scm-scene-header-struct")) return std::nullopt;

    add_owner(document, "formats.scm.scene-header", scene.offset,
              scene_block_header_size, "Canonical 0x20 scene-node block header.");
    annotate(document, "scm-scene-preservation-10-1f", scene.offset + 0x10U,
             scene.reserved10_1f.size(),
             "PRESERVED_UNDECODED: scene shell +0x10..+0x1F; no promoted live model consumer.",
             {"PRESERVED_UNDECODED", "scene"});

    const auto n = static_cast<std::uint64_t>(h.scene_node_count);
    const auto parent_offset = scene.offset + scene.parent_rel;
    const auto order_offset = scene.offset + scene.order_rel;
    const auto binding_offset = scene.offset + scene.object_binding_rel;
    const auto transform_offset = scene.offset + scene.transform_rel;
    if (!add_region(document, "scm-scene-parent-array", "Scene parent array",
            parent_offset, n, binary::RegionKind::table, "s8[node_count]") ||
        !add_region(document, "scm-scene-order-array", "Scene evaluation-order array",
            order_offset, n, binary::RegionKind::table, "u8[node_count]") ||
        !add_region(document, "scm-scene-binding-array", "Scene object-binding array",
            binding_offset, n, binary::RegionKind::table, "s8[node_count]") ||
        !add_region(document, "scm-scene-transform-array", "Scene transform array",
            transform_offset, n * scene_transform_size, binary::RegionKind::table,
            "ScmSceneTransform[node_count]")) return std::nullopt;

    add_owner(document, "formats.scm.scene-parents", parent_offset, n,
              "Evaluation-position parent node indices; root uses -1.");
    add_owner(document, "formats.scm.scene-order", order_offset, n,
              "Permutation from evaluation position to scene-node index.");
    add_owner(document, "formats.scm.scene-bindings", binding_offset, n,
              "Object binding indexed by scene-node index; -1 denotes helper node.");
    add_owner(document, "formats.scm.scene-transforms", transform_offset,
              n * scene_transform_size,
              "SCM local transform records: translation, translation magnitude, XYZ radians, preserved +0x1C.");

    for (std::size_t i = 0; i < scene.parent_by_order_position.size(); ++i) {
        if (!add_field(document, index_id("scm-scene-parent", i), "Parent node",
                parent_offset + i, 1U, binary::FieldKind::signed_integer, "s8",
                signed_dec(scene.parent_by_order_position[i])) ||
            !add_field(document, index_id("scm-scene-order", i), "Node at evaluation position",
                order_offset + i, 1U, binary::FieldKind::unsigned_integer, "u8",
                dec(scene.node_at_order_position[i])) ||
            !add_field(document, index_id("scm-scene-binding", i), "Object binding by node",
                binding_offset + i, 1U, binary::FieldKind::signed_integer, "s8",
                signed_dec(scene.object_binding_by_node_index[i]))) return std::nullopt;
    }

    for (std::size_t i = 0; i < scene.transform_by_node_index.size(); ++i) {
        const auto& transform = scene.transform_by_node_index[i];
        const auto id = index_id("scm-scene-transform", i);
        const auto off = transform_offset + static_cast<std::uint64_t>(i) * scene_transform_size;
        if (!add_field(document, id, "Scene transform", off, scene_transform_size,
                binary::FieldKind::structure, "ScmSceneTransform", {}) ||
            !add_vec3(document, id + "-translation", "Translation", off,
                transform.translation, id) ||
            !add_field(document, id + "-translation-magnitude", "Translation magnitude",
                off + 0x0CU, 4U, binary::FieldKind::floating_point, "f32_le length(T)",
                f32(transform.translation_magnitude), id) ||
            !add_vec3(document, id + "-rotation", "Rotation XYZ radians", off + 0x10U,
                transform.rotation_xyz_radians, id) ||
            !add_field(document, id + "-reserved1c", "Preserved transform +0x1C",
                off + 0x1CU, 4U, binary::FieldKind::unknown,
                "f32_le preservation-only", f32(transform.reserved1c), id)) return std::nullopt;
        annotate(document, id + "-preservation-1c", off + 0x1CU, 4U,
                 "PRESERVED_UNDECODED: transform +0x1C is not consumed by confirmed SCM local-matrix construction.",
                 {"PRESERVED_UNDECODED", "transform"});
    }

    return document;
}

} // namespace dmc::rengine::formats::scm
