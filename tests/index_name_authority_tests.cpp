#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] std::uint32_t block_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    std::uint32_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        const auto blocks_w = std::max(1U, (width + 3U) / 4U);
        const auto blocks_h = std::max(1U, (height + 3U) / 4U);
        total += blocks_w * blocks_h * (dxt5 ? 16U : 8U);
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    const auto payload_size = block_payload_size(
        width, height, mip_count, dxt5);
    std::vector<std::byte> bytes(
        128U + static_cast<std::size_t>(payload_size), std::byte{0});
    bytes[0] = std::byte{'D'};
    bytes[1] = std::byte{'D'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{' '};
    put_u32(bytes, 4U, 124U);
    put_u32(bytes, 12U, height);
    put_u32(bytes, 16U, width);
    put_u32(bytes, 28U, mip_count);
    bytes[84U] = std::byte{'D'};
    bytes[85U] = std::byte{'X'};
    bytes[86U] = std::byte{'T'};
    bytes[87U] = dxt5 ? std::byte{'5'} : std::byte{'1'};
    for (std::size_t index = 128U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>((index * 17U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (mip_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(
        descriptor, 0x38U,
        static_cast<std::uint32_t>(dds.size() - 128U));
    put_u32(descriptor, 0x44U, (height << 16U) | width);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(width)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, static_cast<std::uint32_t>(dds.size()));
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] std::vector<std::byte> texture_bundle_fixture() {
    constexpr std::uint32_t width = 16U;
    constexpr std::uint32_t height = 16U;
    constexpr std::uint32_t mip_count = 5U;
    const auto dds = make_dds(width, height, mip_count, true);
    const auto descriptor = make_descriptor(
        dds, width, height, mip_count, true);

    std::vector<std::byte> bytes(0x1000U, std::byte{0});
    put_u32(bytes, 0U, 1U);
    put_u32(bytes, 4U, 1U);
    std::copy(
        descriptor.begin(), descriptor.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(0x800U));
    std::copy(
        dds.begin(), dds.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(0x870U));
    return bytes;
}

[[nodiscard]] std::vector<std::byte> valid_pnst_fixture() {
    std::vector<std::byte> bytes(0x20U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload index_payload(
    std::string_view path,
    std::string_view text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "index-corpus",
                .logical_path = std::string{path},
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = std::string{path},
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion st001_expansion() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "retail-nbz-source",
            .logical_path = "GData.afs/st001.pac",
            .container_chain = "nbz[12]",
            .offset = 0x1000U,
            .size = 0x10000U,
        },
        .display_name = "st001.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };

    std::vector<std::vector<std::byte>> slot_bytes(8U);
    slot_bytes[0] = {std::byte{0x11}, std::byte{0x22}};
    slot_bytes[1] = texture_bundle_fixture();
    slot_bytes[2] = {std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{0}};
    slot_bytes[3] = {std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}};
    slot_bytes[4] = bytes_from_text("#SET 1\n");
    slot_bytes[5] = valid_pnst_fixture();
    slot_bytes[6] = {std::byte{0x31}, std::byte{0x41}, std::byte{0x59}};
    slot_bytes[7] = {std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}};

    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
    };
    expansion.children.reserve(slot_bytes.size());
    std::uint64_t offset = 0x2000U;
    for (std::uint32_t slot = 0U; slot < slot_bytes.size(); ++slot) {
        const auto size = static_cast<std::uint64_t>(slot_bytes[slot].size());
        const auto logical = "GData.afs/st001.pac::PAC/slot-000" +
            std::to_string(slot);
        const auto chain = "nbz[12]/PAC[" + std::to_string(slot) + "]";
        expansion.children.push_back(gdspaces::ContainerChild{
            .entry = formats::ContainerEntry{
                .slot_index = slot,
                .offset = offset,
                .size = size,
                .logical_name = "slot_000" + std::to_string(slot) + ".bin",
                .populated = true,
                .synthetic_name = true,
            },
            .payload = gdspaces::ResourcePayload{
                .resource = gdspaces::ResourceRef{
                    .id = gdspaces::ResourceId{
                        .source_id = parent.id.source_id,
                        .logical_path = logical,
                        .container_chain = chain,
                        .offset = parent.id.offset + offset,
                        .size = size,
                    },
                    .display_name = "slot_000" + std::to_string(slot) + ".bin",
                    .format = "bin",
                    .profile = "dmc3-hd",
                    .synthetic_name = true,
                    .container = false,
                },
                .bytes = std::move(slot_bytes[slot]),
                .diagnostics = {},
                .byte_provenance = std::nullopt,
            },
        });
        offset += size + 0x10U;
    }
    return expansion;
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion sparse_pnst_expansion() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    gdspaces::ContainerExpansion expansion{
        .parent = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "pnst-source",
                .logical_path = "GData.afs/em000.pac::PNST/slot-0003",
                .container_chain = "nbz[4]/PNST[3]",
                .offset = 0x4000U,
                .size = 0x100U,
            },
            .display_name = "nested.pac",
            .format = "pnst",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .parser_format = "PNST",
        .children = {},
        .diagnostics = {},
    };

    for (std::uint32_t slot = 0U; slot < 5U; ++slot) {
        const bool populated = slot == 0U || slot == 2U || slot == 4U;
        const auto size = populated ? 4U : 0U;
        expansion.children.push_back(gdspaces::ContainerChild{
            .entry = formats::ContainerEntry{
                .slot_index = slot,
                .offset = populated ? 0x20U + slot * 4U : 0U,
                .size = size,
                .logical_name = "slot_000" + std::to_string(slot) + ".bin",
                .populated = populated,
                .synthetic_name = true,
            },
            .payload = gdspaces::ResourcePayload{
                .resource = gdspaces::ResourceRef{
                    .id = gdspaces::ResourceId{
                        .source_id = "pnst-source",
                        .logical_path = "GData.afs/em000.pac::PNST/slot-000" +
                            std::to_string(slot),
                        .container_chain = "nbz[4]/PNST[3]/PNST[" +
                            std::to_string(slot) + "]",
                        .offset = populated ? 0x4020U + slot * 4U : 0U,
                        .size = size,
                    },
                    .display_name = "slot_000" + std::to_string(slot) + ".bin",
                    .format = populated ? "bin" : "empty-slot",
                    .profile = "dmc3-hd",
                    .synthetic_name = true,
                    .container = false,
                },
                .bytes = populated
                    ? std::vector<std::byte>(4U, static_cast<std::byte>(slot + 1U))
                    : std::vector<std::byte>{},
                .diagnostics = {},
                .byte_provenance = std::nullopt,
            },
        });
    }
    return expansion;
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    constexpr std::string_view index_text =
        "st001_000.ukn\r\n"
        "st001_001 folder\r\n"
        "st001_002.scm\r\n"
        "st001_003.ukn\r\n"
        "st001_004.txt\r\n"
        "st001_005.pac\r\n"
        "st001_006.ukn\r\n"
        "st001_007.pac\r\n";

    const auto source_index = index_payload("st001.index", index_text);
    const auto parsed = gdspaces::IndexManifestParser::parse(source_index);
    assert(parsed.ok());
    assert(parsed.manifest->entries().size() == 8U);
    assert(parsed.manifest->entries()[1].raw == "st001_001 folder");
    assert(parsed.manifest->entries()[1].name == "st001_001");
    assert(parsed.manifest->entries()[1].is_folder);
    assert(parsed.manifest->entries()[2].extension == "scm");
    assert(parsed.manifest->entries()[2].line_number == 3U);
    assert(parsed.manifest->observed_sha256().size() == 64U);

    const auto changed_index = index_payload(
        "st001.index",
        "st001_999.ukn\n"
        "st001_001 folder\n"
        "st001_002.scm\n"
        "st001_003.ukn\n"
        "st001_004.txt\n"
        "st001_005.pac\n"
        "st001_006.ukn\n"
        "st001_007.pac\n");
    const auto changed_parsed = gdspaces::IndexManifestParser::parse(changed_index);
    assert(changed_parsed.ok());
    assert(
        parsed.manifest->observed_sha256() !=
        changed_parsed.manifest->observed_sha256());

    auto expansion = st001_expansion();
    assert(expansion.usable());
    std::vector<gdspaces::ResourceId> ids_before;
    std::vector<std::vector<std::byte>> bytes_before;
    for (const auto& child : expansion.children) {
        ids_before.push_back(child.payload.resource.id);
        bytes_before.push_back(child.payload.bytes);
    }

    const auto binding = gdspaces::IndexSlotNameBinder::bind(
        expansion, *parsed.manifest);
    assert(binding.ok());
    assert(binding.binding->mapping_mode() ==
           gdspaces::IndexSlotMappingMode::populated_slot_sequence);
    assert(binding.binding->authorities().size() == 8U);
    assert(binding.binding->authorities()[5].slot_index() == 5U);
    assert(binding.binding->authorities()[5].index_name() == "st001_005.pac");
    assert(binding.binding->authorities()[5].child_resource() == ids_before[5]);

    const auto overlay = gdspaces::IndexNameOverlayBuilder::build(
        expansion,
        *binding.binding,
        dmc3::resolve_index_display_semantic);
    assert(overlay.ok());
    assert(overlay.overlay->entries().size() == 8U);
    assert(overlay.overlay->manifest_resource() == source_index.resource.id);
    assert(overlay.overlay->entries()[6].semantic_format() == "unknown");
    assert(
        overlay.overlay->manifest_sha256() ==
        parsed.manifest->observed_sha256());

    const auto applied = gdspaces::IndexNameOverlayBuilder::apply(
        expansion, *overlay.overlay);
    assert(applied.ok());

    assert(expansion.children[1].payload.resource.display_name == "st001_001.ptx");
    assert(expansion.children[2].payload.resource.display_name == "st001_002.scm");
    assert(expansion.children[3].payload.resource.display_name == "st001_003.hits");
    assert(expansion.children[4].payload.resource.display_name == "st001_004.txt");
    assert(expansion.children[5].payload.resource.display_name == "st001_005.pnst");
    assert(expansion.children[6].payload.resource.display_name == "st001_006.ukn");
    assert(expansion.children[7].payload.resource.display_name == "st001_007.pac");

    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        assert(expansion.children[index].payload.resource.id == ids_before[index]);
        assert(expansion.children[index].payload.bytes == bytes_before[index]);
    }
    assert(
        overlay.overlay->entries()[1].evidence_kind() ==
        gdspaces::IndexDisplayEvidenceKind::profile_structural_format);
    assert(overlay.overlay->entries()[1].semantic_format() == "texture-bundle");
    assert(
        overlay.overlay->entries()[3].evidence_kind() ==
        gdspaces::IndexDisplayEvidenceKind::magic_confirmed_format);
    assert(overlay.overlay->entries()[3].semantic_format() == "hits");
    assert(
        overlay.overlay->entries()[6].evidence_kind() ==
        gdspaces::IndexDisplayEvidenceKind::index_source_extension);

    auto wrong_parent = st001_expansion();
    wrong_parent.parent.id.logical_path = "GData.afs/other.pac";
    const auto before_wrong_name =
        wrong_parent.children[0].payload.resource.display_name;
    const auto rejected = gdspaces::IndexNameOverlayBuilder::apply(
        wrong_parent, *overlay.overlay);
    assert(!rejected.ok());
    assert(
        wrong_parent.children[0].payload.resource.display_name ==
        before_wrong_name);

    const auto pnst_index = index_payload(
        "nested.index",
        "PNST\nchild_a.mod\nchild_b.pac\nchild_c.mod\n");
    const auto pnst_manifest = gdspaces::IndexManifestParser::parse(pnst_index);
    assert(pnst_manifest.ok());
    const auto sparse = sparse_pnst_expansion();
    const auto sparse_binding = gdspaces::IndexSlotNameBinder::bind(
        sparse, *pnst_manifest.manifest);
    assert(sparse_binding.ok());
    assert(
        sparse_binding.binding->mapping_mode() ==
        gdspaces::IndexSlotMappingMode::populated_slot_sequence);
    assert(sparse_binding.binding->authorities().size() == 3U);
    assert(sparse_binding.binding->authorities()[0].slot_index() == 0U);
    assert(sparse_binding.binding->authorities()[1].slot_index() == 2U);
    assert(sparse_binding.binding->authorities()[2].slot_index() == 4U);

    const auto duplicate_index = index_payload(
        "duplicate.index", "same.mod\nsame.mod\n");
    const auto duplicate_manifest =
        gdspaces::IndexManifestParser::parse(duplicate_index);
    assert(duplicate_manifest.ok());
    auto duplicate_expansion = sparse_pnst_expansion();
    duplicate_expansion.children[1] = duplicate_expansion.children[2];
    duplicate_expansion.children.resize(2U);
    duplicate_expansion.children[0].entry.slot_index = 0U;
    duplicate_expansion.children[1].entry.slot_index = 1U;
    duplicate_expansion.children[1].payload.resource.id.logical_path =
        "GData.afs/em000.pac::PNST/slot-0001";
    duplicate_expansion.children[1].payload.resource.id.container_chain =
        "nbz[4]/PNST[3]/PNST[1]";
    const auto duplicate_binding = gdspaces::IndexSlotNameBinder::bind(
        duplicate_expansion, *duplicate_manifest.manifest);
    assert(duplicate_binding.ok());
    assert(duplicate_binding.binding->authorities().size() == 2U);
    assert(
        duplicate_binding.binding->authorities()[0].index_name() ==
        duplicate_binding.binding->authorities()[1].index_name());
    assert(
        duplicate_binding.binding->authorities()[0].child_resource() !=
        duplicate_binding.binding->authorities()[1].child_resource());

    auto binary_index = index_payload("bad.index", "good.mod\n");
    binary_index.bytes.push_back(std::byte{0});
    binary_index.resource.id.size =
        static_cast<std::uint64_t>(binary_index.bytes.size());
    const auto binary_parse = gdspaces::IndexManifestParser::parse(binary_index);
    assert(!binary_parse.ok());
    assert(!binary_parse.manifest.has_value());

    auto truncated_index = source_index;
    truncated_index.resource.id.size += 1U;
    assert(!gdspaces::IndexManifestParser::parse(truncated_index).ok());

    const auto wrong_kind = index_payload("not-index.txt", "one.bin\n");
    assert(!gdspaces::IndexManifestParser::parse(wrong_kind).ok());

    const auto mismatch_index = index_payload(
        "mismatch.index", "one.bin\ntwo.bin\n");
    const auto mismatch_manifest = gdspaces::IndexManifestParser::parse(
        mismatch_index);
    assert(mismatch_manifest.ok());
    auto one_slot = sparse_pnst_expansion();
    one_slot.children.resize(1U);
    const auto mismatch_binding = gdspaces::IndexSlotNameBinder::bind(
        one_slot, *mismatch_manifest.manifest);
    assert(!mismatch_binding.ok());
    assert(mismatch_binding.binding.has_value());
    assert(!mismatch_binding.binding->valid());

    return 0;
}
