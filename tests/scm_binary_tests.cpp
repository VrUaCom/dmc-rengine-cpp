#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_render.hpp"
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

    // Exact SCM compatibility selector -> VS-base table recovered from the
    // canonical 0x1400446C4 jump table. Selector 6 deliberately has no
    // shader-bind path. SCM source initialization itself produces 2 or 3.
    assert(runtime::scm_compatibility_object_selector(0U) == 3U);
    assert(runtime::scm_compatibility_object_selector(
               runtime::source_mask_00080000) == 2U);
    assert(runtime::scm_compatibility_vs_base_key(2U) == 13U);
    assert(runtime::scm_compatibility_vs_base_key(3U) == 13U);
    assert(runtime::scm_compatibility_vs_base_key(4U) == 5U);
    assert(runtime::scm_compatibility_vs_base_key(5U) == 8U);
    assert(runtime::scm_compatibility_vs_base_key(6U) ==
           runtime::scm_compat_invalid_vs_base_key);
    assert(runtime::scm_compatibility_vs_base_key(7U) == 10U);
    assert(runtime::scm_compatibility_vs_base_key(8U) == 11U);
    assert(runtime::scm_compatibility_vs_base_key(9U) == 9U);
    assert(runtime::scm_compatibility_vs_base_key(10U) == 7U);
    assert(runtime::scm_compatibility_vs_base_key(11U) == 12U);
    assert(runtime::scm_compatibility_vs_base_key(12U) == 6U);

    // Exact real SCM material GIF packet: one PACKED loop, four A+D writes.
    assert(scm_material_gif_tag_qword == 0x4000000000008001ULL);
    assert(scm_material_gif_regs_qword == 0x000000000020EEEEULL);
    assert(scm_material_gif_nreg == 4U);
    assert(scm_material_ad_registers[0] == legacy_gs_reg_tex0_1);
    assert(scm_material_ad_registers[1] == legacy_gs_reg_tex1_1);
    assert(scm_material_ad_registers[2] == legacy_gs_reg_clamp_1);
    assert(scm_material_ad_registers[3] == legacy_gs_reg_miptbp1_1);
    assert(std::find(scm_material_ad_registers.begin(),
                     scm_material_ad_registers.end(),
                     static_cast<std::uint8_t>(0x7DU)) ==
           scm_material_ad_registers.end());

    // Active TEX0 is the HD resource-registry bridge: TBP0 low14 selects the
    // companion-created texture object; bits 36..37 become the base PS key.
    constexpr std::uint64_t tex0_probe =
        0x1234ULL | (0x2ULL << legacy_gs_tex0_ps_base_key_shift);
    static_assert(legacy_gs_tex0_resource_key(tex0_probe) == 0x1234U);
    static_assert(legacy_gs_tex0_ps_base_key(tex0_probe) == 2U);

    // Normal compatibility sampler projection. Legacy REGION_REPEAT WMS/WMT=3
    // remains WRAP in the native D3D11 address-mode bridge; modes 1/2 map to
    // D3D11 CLAMP. The baseline SCM sampler filter resolves to anisotropic.
    constexpr auto region_repeat_qword =
        pack_legacy_gs_clamp_region_repeat({1U, 2U, 3U, 4U});
    constexpr auto region_repeat_flags =
        hd_sampler_flags_from_legacy_clamp(region_repeat_qword);
    static_assert(region_repeat_flags == hd_sampler_flag_filter);
    constexpr auto region_repeat_d3d =
        project_hd_sampler_flags_to_d3d11(region_repeat_flags);
    static_assert(region_repeat_d3d.filter == d3d11_filter_anisotropic);
    static_assert(region_repeat_d3d.address_u == d3d11_texture_address_wrap);
    static_assert(region_repeat_d3d.address_v == d3d11_texture_address_wrap);

    constexpr std::uint64_t clamp_modes_1_2 = 1ULL | (2ULL << 2U);
    constexpr auto clamp_flags =
        hd_sampler_flags_from_legacy_clamp(clamp_modes_1_2);
    static_assert((clamp_flags & hd_sampler_flag_address_u_clamp) != 0U);
    static_assert((clamp_flags & hd_sampler_flag_address_v_clamp) != 0U);
    constexpr auto clamp_d3d = project_hd_sampler_flags_to_d3d11(clamp_flags);
    static_assert(clamp_d3d.address_u == d3d11_texture_address_clamp);
    static_assert(clamp_d3d.address_v == d3d11_texture_address_clamp);

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

    const auto* texture_mirror =
        document.find_annotation("scm-prov-header-texture-mirror");
    assert(texture_mirror != nullptr);
    assert(has_tag(*texture_mirror, "external-companion"));

    const auto* alpha = document.find_annotation("scm-prov-object-000-alpha");
    assert(alpha != nullptr);
    assert(has_tag(*alpha, "shader-visible"));
    assert(alpha->text.find("COLOR0.a") != std::string::npos);

    const auto* selector =
        document.find_annotation("scm-prov-object-000-compat-selector");
    assert(selector != nullptr);
    assert(has_tag(*selector, "EXE_CONFIRMED"));
    assert(has_tag(*selector, "d3d11"));
    assert(selector->text.find("selector 3") != std::string::npos);
    assert(selector->text.find("VS base key 13") != std::string::npos);
    assert(selector->text.find("VSSetShader") != std::string::npos);
    assert(selector->text.find("PSSetShader") != std::string::npos);

    const auto* tex1 =
        document.find_annotation("scm-prov-object-000-tex1-filter");
    assert(tex1 != nullptr);
    assert(has_tag(*tex1, "TEX1_1"));
    assert(tex1->text.find("0x60 linear") != std::string::npos);
    assert(tex1->text.find("register 0x14") != std::string::npos);
    assert(tex1->text.find("no direct TEX1-to-D3D11") != std::string::npos);

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
    assert(texture->text.find("TEX0_1") != std::string::npos);
    assert(texture->text.find("TEX0.TBP0") != std::string::npos);
    assert(texture->text.find("0x140033350") != std::string::npos);
    assert(texture->text.find("PSSetShaderResources") != std::string::npos);
    assert(texture->text.find("PSSetSamplers") != std::string::npos);
    assert(has_tag(*texture, "resource-registry"));
    assert(has_tag(*texture, "d3d11"));
    assert(!has_tag(*texture, "SRV_SAMPLER_SOURCE_BINDING_OPEN"));

    const auto* clamp =
        document.find_annotation("scm-prov-mesh-000-000-gs-clamp");
    assert(clamp != nullptr);
    assert(has_tag(*clamp, "CLAMP_1"));
    assert(has_tag(*clamp, "D3D11_SAMPLER_DESC"));
    assert(has_tag(*clamp, "PSSetSamplers"));
    assert(clamp->text.find("0x1405D9210") != std::string::npos);
    assert(clamp->text.find("AddressU/V=CLAMP") != std::string::npos);
    assert(clamp->text.find("WRAP") != std::string::npos);

    const auto* scene_shell =
        document.find_annotation("scm-prov-scene-shell-negative");
    assert(scene_shell != nullptr);
    assert(has_tag(*scene_shell, "PRESERVED_UNDECODED"));

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

    const auto* carried = document.find_annotation("scm-prov-header-resource-code");
    assert(carried != nullptr);
    assert(has_tag(*carried, "EXE_AND_CORPUS_CONFIRMED"));
    assert(has_tag(*carried, "PRESERVED_UNDECODED_HIGH_LEVEL_ROLE"));
    assert(!has_tag(*carried, "BOUNDED_NEGATIVE_EVIDENCE"));
    assert(!has_tag(*carried, "DEEP_NEGATIVE_EVIDENCE"));

    const auto* lighting = document.find_annotation("scm-prov-header-13");
    assert(lighting != nullptr);
    assert(has_tag(*lighting, "EXE_CONFIRMED"));
    assert(has_tag(*lighting, "lighting-reference"));
    assert(!has_tag(*lighting, "PRESERVED_UNDECODED"));
    assert(lighting->text.find("manager+0xFA") != std::string::npos);
    assert(lighting->text.find("0x1402FD040") != std::string::npos);
    assert(lighting->text.find("MDL_LIGHT_MAT") != std::string::npos);

    const auto* rotation =
        document.find_annotation("scm-prov-transform-000-rotation");
    assert(rotation != nullptr);
    assert(rotation->text.find("0x1402F9700") != std::string::npos);

    assert(document.conflicts().empty());
    assert(document.ownership_conflicts().empty());

    return 0;
}
