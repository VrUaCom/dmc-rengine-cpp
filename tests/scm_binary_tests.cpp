#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_runtime_flags.hpp"
#include "dmc_rengine/formats/scm_runtime_provenance.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace {

template <typename T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

std::vector<std::byte> fixture() {
    using namespace dmc::rengine::formats::scm;

    ObjectShape shape;
    shape.mesh_vertex_counts = {3U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 1U);

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});
    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put<std::uint32_t>(bytes, 0x14U, 300100U);
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset + 0x00U] = std::byte{1};
    bytes[object_offset + 0x01U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(bytes, object_offset + 0x08U,
                       object_layout.mesh_table_offset);
    put<std::uint32_t>(bytes, object_offset + 0x10U,
                       dmc::rengine::formats::scm::runtime::source_mask_00200000);
    put<float>(bytes, object_offset + 0x3CU, 1.0F);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint16_t>(bytes, mesh_offset + 0x02U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x28U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U,
                       mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    const auto scene_offset = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};

    return bytes;
}

bool has_tag(
    const dmc::rengine::binary::Annotation& annotation,
    std::string_view tag) {
    return std::find(annotation.tags.begin(), annotation.tags.end(), tag) !=
        annotation.tags.end();
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using namespace dmc::rengine::formats::scm;

    const auto bytes = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{bytes});
    assert(parsed.recognized);
    assert(parsed.ok());

    gdspaces::ResourceRef resource{
        .id = gdspaces::ResourceId{
            .source_id = "scm-binary-test",
            .logical_path = "room/test.scm",
            .container_chain = {},
            .offset = 0U,
            .size = bytes.size(),
        },
        .display_name = "test.scm",
        .format = "SCM",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    const auto mapped = build_deep_binary_document(
        resource, std::span<const std::byte>{bytes}, parsed);
    assert(mapped.has_value());

    const auto& document = *mapped;
    assert(document.find_region("scm-header") != nullptr);
    assert(document.find_region("scm-object-000") != nullptr);
    assert(document.find_region("scm-mesh-000-000") != nullptr);
    assert(document.find_region("scm-mesh-000-000-positions-stream") != nullptr);
    assert(document.find_region("scm-scene-header") != nullptr);
    assert(document.find_region("scm-scene-transform-array") != nullptr);
    assert(document.find_region("scm-mesh-000-000-index-workspace") != nullptr);

    const auto* resource_code = document.find_field("scm-resource-code");
    assert(resource_code != nullptr);
    assert(resource_code->display_value.find("300100") != std::string::npos);

    const auto* object_flags = document.find_field("scm-object-000-flags");
    assert(object_flags != nullptr);
    assert(object_flags->display_value.find("0x00200000") != std::string::npos);

    const auto* texture_slot = document.find_field("scm-mesh-000-000-texture-index");
    assert(texture_slot != nullptr);
    assert(texture_slot->display_value == "0");

    assert(document.find_annotation("scm-object-000-flag-bit21") != nullptr);
    assert(document.find_annotation("scm-scene-preservation-10-1f") != nullptr);
    assert(document.find_annotation("scm-mesh-000-000-topology-contract") != nullptr);

    // Runtime provenance is now part of the same shared Binary Inspector
    // document instead of living only in research notes.
    const auto* texture_mirror =
        document.find_annotation("scm-prov-header-texture-mirror");
    assert(texture_mirror != nullptr);
    assert(has_tag(*texture_mirror, "external-companion"));

    const auto* alpha = document.find_annotation("scm-prov-object-000-alpha");
    assert(alpha != nullptr);
    assert(has_tag(*alpha, "shader-visible"));
    assert(alpha->text.find("COLOR0.a") != std::string::npos);

    const auto* bit21 =
        document.find_annotation("scm-prov-object-000-bit21-negative");
    assert(bit21 != nullptr);
    assert(has_tag(*bit21, "BOUNDED_NEGATIVE_EVIDENCE"));
    assert(bit21->text.find("0x1402F4C21") != std::string::npos);
    assert(bit21->text.find("0x140303F2F") != std::string::npos);

    const auto* texture =
        document.find_annotation("scm-prov-mesh-000-000-texture");
    assert(texture != nullptr);
    assert(texture->text.find("index*0x40") != std::string::npos);

    const auto* scene_shell =
        document.find_annotation("scm-prov-scene-shell-negative");
    assert(scene_shell != nullptr);
    assert(has_tag(*scene_shell, "PRESERVED_UNDECODED"));

    // Every reserved lane states its negative evidence, the header's three
    // included. "Preserved-undecoded" alone does not distinguish a lane a
    // census looked at and found dormant from one nobody has examined, and the
    // completion audit gives all three header lanes the first disposition.
    for (const auto* id : {
             "scm-prov-header-reserved08-negative",
             "scm-prov-header-reserved18-negative",
             "scm-prov-header-reserved28-negative",
         }) {
        const auto* lane = document.find_annotation(id);
        assert(lane != nullptr);
        assert(has_tag(*lane, "RESERVED_OBSERVED_ZERO"));
        assert(has_tag(*lane, "PRESERVED_UNDECODED"));
        assert(has_tag(*lane, "BOUNDED_NEGATIVE_EVIDENCE"));
    }

    // +0x13 is carried to manager +0xFA, so it is undecoded but not dormant.
    // Giving it the same negative evidence would claim nothing reads it.
    const auto* carried = document.find_annotation("scm-prov-header-13");
    assert(carried != nullptr);
    assert(has_tag(*carried, "runtime-carried"));
    assert(!has_tag(*carried, "BOUNDED_NEGATIVE_EVIDENCE"));

    const auto* rotation =
        document.find_annotation("scm-prov-transform-000-rotation");
    assert(rotation != nullptr);
    assert(rotation->text.find("0x1402F9700") != std::string::npos);

    // The deep reader must remain read-only and evidence aware. It may leave
    // canonical alignment padding uncovered, but it must map all owned semantic
    // payload domains and never overlap physical regions.
    assert(document.conflicts().empty());
    assert(document.ownership_conflicts().empty());

    return 0;
}
