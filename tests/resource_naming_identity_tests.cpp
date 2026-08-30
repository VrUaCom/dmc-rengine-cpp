#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"
#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"
#include "dmc_rengine/profiles/dmc3/legacy_extraction_naming.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
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

[[nodiscard]] std::uint32_t dds_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5) {
    std::uint32_t total = 0U;
    while (true) {
        total += std::max(1U, (width + 3U) / 4U) *
            std::max(1U, (height + 3U) / 4U) * (dxt5 ? 16U : 8U);
        if (width == 1U && height == 1U) {
            break;
        }
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    constexpr std::uint32_t width = 64U;
    constexpr std::uint32_t height = 64U;
    std::vector<std::byte> payload(
        dds_payload_size(width, height, true), std::byte{0x5A});
    const auto built = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dmc3::Dmc3DdsCompression::dxt5,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> make_texture_bundle() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto dds = make_dds();
    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(descriptor, 0x08U, 0x20000U | (doc.mip_map_count << 8U) | 0x88U);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * 4U);
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(descriptor, 0x44U, (doc.height << 16U) | doc.width);
    put_u32(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.width)));
    put_u32(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.height)));
    put_u32(descriptor, 0x60U, 4U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);

    const auto record_size = descriptor.size() + dds.size();
    const auto sectors = static_cast<std::uint32_t>(
        (record_size + dmc3::TextureSlotFramingParser::k_sector_size - 1U) /
        dmc3::TextureSlotFramingParser::k_sector_size);
    std::vector<std::byte> output(
        dmc3::TextureSlotFramingParser::k_bundle_header_size, std::byte{0});
    put_u32(output, 0U, 1U);
    put_u32(output, 4U, sectors);
    output.insert(output.end(), descriptor.begin(), descriptor.end());
    output.insert(output.end(), dds.begin(), dds.end());
    output.insert(
        output.end(),
        static_cast<std::size_t>(sectors) *
                dmc3::TextureSlotFramingParser::k_sector_size -
            record_size,
        std::byte{0});
    return output;
}

[[nodiscard]] std::vector<std::byte> st001_name_list() {
    std::vector<std::byte> bytes(48U, std::byte{0});
    const auto copy = [&](std::size_t offset, std::string_view value) {
        for (std::size_t index = 0U; index < value.size(); ++index) {
            bytes[offset + index] = static_cast<std::byte>(
                static_cast<unsigned char>(value[index]));
        }
    };
    copy(0U, "st001.ptx");
    copy(11U, "st001.scm");
    copy(22U, "st001.sch");
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_index(
    std::string_view path,
    std::string_view text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-source",
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
        .name_evidence = {},
        .semantic_evidence = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerChild make_child(
    const dmc::rengine::gdspaces::ResourceRef& parent,
    std::uint32_t slot,
    bool populated,
    std::vector<std::byte> bytes) {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto size = static_cast<std::uint64_t>(bytes.size());
    const auto offset = populated
        ? static_cast<std::uint64_t>(0x100U + slot * 0x1000U)
        : 0U;
    return gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = slot,
            .offset = offset,
            .size = size,
            .logical_name = "slot_" + std::to_string(slot) + ".bin",
            .populated = populated,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = gdspaces::ResourceId{
                    .source_id = parent.id.source_id,
                    .logical_path = parent.id.logical_path + "::PAC/slot-" +
                        std::to_string(slot),
                    .container_chain = parent.id.container_chain + "/PAC[" +
                        std::to_string(slot) + "]",
                    .offset = populated ? parent.id.offset + offset : 0U,
                    .size = size,
                },
                .display_name = "slot_" + std::to_string(slot) + ".bin",
                .format = populated ? "unknown" : "empty-slot",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = std::move(bytes),
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
            .semantic_evidence = {},
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion st001_expansion() {
    namespace gdspaces = dmc::rengine::gdspaces;
    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "retail-source",
            .logical_path = "GData.afs/scr/st001.pac",
            .container_chain = "NBZ[12]",
            .offset = 0x10000U,
            .size = 0x20000U,
        },
        .display_name = "st001.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };

    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
        .external_index_evidence = std::nullopt,
    };
    expansion.children.push_back(make_child(parent, 0U, true, st001_name_list()));
    expansion.children.push_back(make_child(parent, 1U, true, make_texture_bundle()));
    expansion.children.push_back(make_child(
        parent, 2U, true,
        {std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{0}}));
    expansion.children.push_back(make_child(
        parent, 3U, true,
        {std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}, std::byte{'$'}}));
    return expansion;
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion em035_037_sparse() {
    namespace gdspaces = dmc::rengine::gdspaces;
    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "retail-source",
            .logical_path = "GData.afs/em035_037.pac",
            .container_chain = "NBZ[7]",
            .offset = 0x80000U,
            .size = 0x8000U,
        },
        .display_name = "em035_037.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };
    constexpr std::uint32_t populated_slots[]{5U, 6U, 7U, 21U, 50U, 80U, 89U, 94U};
    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
        .external_index_evidence = std::nullopt,
    };
    expansion.children.reserve(95U);
    for (std::uint32_t slot = 0U; slot < 95U; ++slot) {
        const bool populated = std::find(
            std::begin(populated_slots), std::end(populated_slots), slot) !=
            std::end(populated_slots);
        expansion.children.push_back(make_child(
            parent,
            slot,
            populated,
            populated ? bytes_from_text("payload") : std::vector<std::byte>{}));
    }
    return expansion;
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    auto expansion = st001_expansion();
    std::vector<gdspaces::ResourceId> original_ids;
    for (const auto& child : expansion.children) {
        original_ids.push_back(child.payload.resource.id);
    }

    const auto index = make_index(
        "GData.afs/scr/st001.index",
        "st001_000.ukn\r\n"
        "st001_001 folder\r\n"
        "st001_002.scm\r\n"
        "st001_003.ukn\r\n");
    const auto reconciled = gdspaces::ContainerNamingReconciler::reconcile(
        expansion,
        &index,
        dmc3::resolve_index_display_semantic);
    assert(reconciled.ok());
    assert(expansion.external_index_evidence.has_value());
    assert(expansion.external_index_evidence->directive.empty());
    assert(expansion.external_index_evidence->entry_count == 4U);

    for (std::size_t i = 0U; i < expansion.children.size(); ++i) {
        assert(expansion.children[i].payload.resource.id == original_ids[i]);
    }

    const auto snapshot = gdspaces::ResourceNamingIdentityBuilder::build(expansion);
    assert(snapshot.ok());
    assert(snapshot.children.size() == 4U);

    const auto& slot0 = snapshot.children[0];
    assert(slot0.physical_slot_index == 0U);
    assert(slot0.extracted_ordinal == 0U);
    assert(slot0.external_index_raw_label == "st001_000.ukn");
    assert(slot0.external_index_name == "st001_000.ukn");
    assert(!slot0.external_index_folder);
    assert(!slot0.embedded_alias.has_value());
    assert(slot0.semantic_format == "name-list");
    assert(slot0.canonical_extension == "index");
    assert(slot0.canonical_display_name == "st001_000.index");

    const auto& slot1 = snapshot.children[1];
    assert(slot1.physical_slot_index == 1U);
    assert(slot1.extracted_ordinal == 1U);
    assert(slot1.external_index_raw_label == "st001_001 folder");
    assert(slot1.external_index_name == "st001_001");
    assert(slot1.external_index_folder);
    assert(slot1.embedded_alias == "st001.ptx");
    assert(slot1.semantic_format == "texture-bundle");
    assert(slot1.canonical_extension == "ptx");
    assert(slot1.canonical_display_name == "st001_001.ptx");

    const auto legacy1 = dmc3::LegacyExtractionNamingPlanner::build(slot1);
    assert(legacy1.valid());
    assert(legacy1.exact_from_external_index);
    assert(
        legacy1.representation ==
        dmc3::LegacyExtractionRepresentation::expanded_directory);
    assert(legacy1.manifest_entry_raw == "st001_001 folder");
    assert(legacy1.extraction_name == "st001_001");
    assert(legacy1.nested_index_name == "st001_001.index");
    assert(legacy1.embedded_semantic_alias == "st001.ptx");
    assert(legacy1.canonical_display_name == "st001_001.ptx");

    const auto& slot2 = snapshot.children[2];
    assert(slot2.extracted_ordinal == 2U);
    assert(slot2.external_index_name == "st001_002.scm");
    assert(slot2.embedded_alias == "st001.scm");
    assert(slot2.semantic_format == "scm");
    assert(slot2.canonical_display_name == "st001_002.scm");

    const auto& slot3 = snapshot.children[3];
    assert(slot3.extracted_ordinal == 3U);
    assert(slot3.external_index_name == "st001_003.ukn");
    assert(slot3.embedded_alias == "st001.sch");
    assert(slot3.semantic_format == "hits");
    assert(slot3.canonical_extension == "hits");
    assert(slot3.canonical_display_name == "st001_003.hits");

    // Real sparse-PAC topology recovered from em035_037.
    auto sparse = em035_037_sparse();
    const auto sparse_index = make_index(
        "GData.afs/em035_037.index",
        "em035_037_000.txt\n"
        "em035_037_001.txt\n"
        "em035_037_002.txt\n"
        "em035_037_003.txt\n"
        "em035_037_004.txt\n"
        "em035_037_005.txt\n"
        "em035_037_006.txt\n"
        "em035_037_007.txt\n");
    const auto sparse_reconciled = gdspaces::ContainerNamingReconciler::reconcile(
        sparse, &sparse_index);
    assert(sparse_reconciled.ok());
    const auto sparse_snapshot = gdspaces::ResourceNamingIdentityBuilder::build(sparse);
    assert(sparse_snapshot.ok());

    constexpr std::uint32_t expected_slots[]{5U, 6U, 7U, 21U, 50U, 80U, 89U, 94U};
    for (std::size_t ordinal = 0U; ordinal < std::size(expected_slots); ++ordinal) {
        const auto slot = expected_slots[ordinal];
        const auto& identity = sparse_snapshot.children[slot];
        assert(identity.physical_slot_index == slot);
        assert(identity.extracted_ordinal == ordinal);
        assert(identity.external_index_name.has_value());
    }
    assert(!sparse_snapshot.children[0].extracted_ordinal.has_value());
    assert(!sparse_snapshot.children[1].external_index_name.has_value());

    // Topology defines extracted ordinal even when historical .index is absent.
    auto topology_only = em035_037_sparse();
    const auto topology_snapshot =
        gdspaces::ResourceNamingIdentityBuilder::build(topology_only);
    assert(topology_snapshot.ok());
    assert(!topology_snapshot.external_index_evidence.has_value());
    for (std::size_t ordinal = 0U; ordinal < std::size(expected_slots); ++ordinal) {
        const auto& identity = topology_snapshot.children[expected_slots[ordinal]];
        assert(identity.extracted_ordinal == ordinal);
        assert(!identity.external_index_name.has_value());
    }

    // Duplicate physical slots make extracted-ordinal binding ambiguous.
    auto duplicate = st001_expansion();
    duplicate.children[1].entry.slot_index = 0U;
    const auto duplicate_result = gdspaces::ContainerNamingReconciler::reconcile(
        duplicate, &index, dmc3::resolve_index_display_semantic);
    assert(!duplicate_result.ok());

    return 0;
}
