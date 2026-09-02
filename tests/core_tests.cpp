#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

using dmc::rengine::evidence::Confidence;
using dmc::rengine::evidence::EvidenceLocation;
using dmc::rengine::evidence::EvidenceRecord;
using dmc::rengine::evidence::EvidenceRegistry;
using dmc::rengine::exe::PeKind;
using dmc::rengine::exe::PeMachine;
using dmc::rengine::exe::PeReader;
using dmc::rengine::gdspaces::LocalDirectorySource;
using dmc::rengine::gdspaces::OpenRequest;
using dmc::rengine::gdspaces::OpenRouter;
using dmc::rengine::gdspaces::ResourceGraph;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::gdspaces::ResourceRelation;
using dmc::rengine::gdspaces::SourceRegistry;
using dmc::rengine::gdspaces::StageBundle;
using dmc::rengine::gdspaces::StageIdentity;
using dmc::rengine::gdspaces::StageMember;
using dmc::rengine::gdspaces::StageResourceCategory;
using dmc::rengine::gdspaces::ToolTarget;

[[nodiscard]] ResourceRef make_resource(
    std::string path,
    std::string format,
    std::uint64_t size = 1) {
    return ResourceRef{
        .id = ResourceId{
            .source_id = "source",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = 0,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

void write_u16(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u64(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> make_synthetic_pe32_plus() {
    std::vector<std::byte> bytes(0x400U, std::byte{0});
    bytes[0] = static_cast<std::byte>('M');
    bytes[1] = static_cast<std::byte>('Z');
    write_u32(bytes, 0x3CU, 0x80U);

    write_u32(bytes, 0x80U, 0x00004550U);
    write_u16(bytes, 0x84U, 0x8664U);
    write_u16(bytes, 0x86U, 1U);
    write_u16(bytes, 0x94U, 0x00F0U);

    constexpr std::size_t optional = 0x98U;
    write_u16(bytes, optional, 0x020BU);
    write_u32(bytes, optional + 16U, 0x1000U);
    write_u64(bytes, optional + 24U, 0x140000000ULL);
    write_u32(bytes, optional + 56U, 0x2000U);
    write_u32(bytes, optional + 60U, 0x0200U);
    write_u16(bytes, optional + 68U, 2U);

    constexpr std::size_t section = 0x188U;
    bytes[section] = static_cast<std::byte>('.');
    bytes[section + 1U] = static_cast<std::byte>('t');
    bytes[section + 2U] = static_cast<std::byte>('e');
    bytes[section + 3U] = static_cast<std::byte>('x');
    bytes[section + 4U] = static_cast<std::byte>('t');
    write_u32(bytes, section + 8U, 0x0100U);
    write_u32(bytes, section + 12U, 0x1000U);
    write_u32(bytes, section + 16U, 0x0200U);
    write_u32(bytes, section + 20U, 0x0200U);
    write_u32(bytes, section + 36U, 0x60000020U);

    return bytes;
}

void test_resource_identity() {
    const ResourceId valid{
        .source_id = "source",
        .logical_path = "dmc3/st001",
        .container_chain = "NBZ/PAC",
        .offset = 16,
        .size = 32,
    };

    assert(valid.valid());
    assert(valid.canonical() == std::string{"rid2|6:source|10:dmc3/st001|7:NBZ/PAC|16|32"});
    assert(!ResourceId{}.valid());
}

void test_evidence_registry() {
    EvidenceRegistry registry;
    EvidenceRecord record{
        .id = "ev-stage-table",
        .claim_id = "claim-stage-table-layout",
        .title = "Stage table layout",
        .summary = "A synthetic test record for the evidence API.",
        .confidence = Confidence::confirmed,
        .locations = {EvidenceLocation{
            .artifact_id = "synthetic-exe",
            .file_offset = 16,
            .size = 32,
            .rva = 64,
            .va = std::nullopt,
            .symbol = "stage_table",
            .note = "Synthetic fixture only.",
        }},
        .tags = {"stage", "table"},
        .supersedes = {},
    };

    assert(registry.add(record));
    assert(!registry.add(record));
    assert(registry.size() == 1U);
    assert(registry.find("ev-stage-table") != nullptr);
    assert(registry.by_claim("claim-stage-table-layout").size() == 1U);
    assert(registry.by_confidence(Confidence::confirmed).size() == 1U);

    record.summary = "Corrected synthetic summary.";
    record.confidence = Confidence::corrected;
    assert(registry.replace(record));
    assert(registry.by_confidence(Confidence::corrected).size() == 1U);
}

void test_local_directory_source() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() /
        ("dmc-rengine-test-" + suffix);

    std::filesystem::create_directories(root / "nested");
    {
        std::ofstream file(root / "stage.pac", std::ios::binary);
        file << "PAC";
    }
    {
        std::ofstream file(root / "nested" / "config.txt", std::ios::binary);
        file << "#SET";
    }

    SourceRegistry registry;
    assert(registry.mount(std::make_unique<LocalDirectorySource>(
        "test-source", root, true)));
    assert(!registry.mount(std::make_unique<LocalDirectorySource>(
        "test-source", root, true)));

    const auto resources = registry.enumerate_all();
    assert(resources.size() == 2U);
    assert(resources[0].id.source_id == "test-source");

    const auto* pac = &resources[0];
    if (pac->format != "pac") {
        pac = &resources[1];
    }
    assert(pac->format == "pac");
    assert(pac->container);

    const auto payload = registry.read(pac->id);
    assert(payload.has_value());
    assert(payload->readable());
    assert(payload->bytes.size() == 3U);
    assert(std::to_integer<char>(payload->bytes[0]) == 'P');

    const ResourceId traversal{
        .source_id = "test-source",
        .logical_path = "../outside.bin",
        .container_chain = {},
        .offset = 0,
        .size = 1,
    };
    const auto rejected = registry.read(traversal);
    assert(rejected.has_value());
    assert(!rejected->readable());

    std::error_code error;
    std::filesystem::remove_all(root, error);
}

void test_graph_router_and_stage_bundle() {
    const auto parent = make_resource("st001.pac", "pac", 100);
    const auto child = make_resource("st001/config.txt", "txt", 20);

    ResourceGraph graph;
    assert(graph.add(parent));
    assert(graph.add(child));
    assert(!graph.add(child));
    assert(graph.connect(parent.id, child.id, ResourceRelation::contains));
    assert(!graph.connect(parent.id, child.id, ResourceRelation::contains));
    assert(graph.related(parent.id, ResourceRelation::contains).size() == 1U);

    const OpenRouter router;
    const OpenRequest item_request{
        .resource = make_resource("item.itm", "itm"),
        .preferred_target = std::nullopt,
        .stage_context = false,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(router.route(item_request) == ToolTarget::item_editor);

    auto menu_request = item_request;
    menu_request.menu_context = true;
    assert(router.route(menu_request) == ToolTarget::modviz_menu);

    StageBundle bundle(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "claim-stage-table-layout",
    });
    assert(bundle.add(StageMember{
        .category = StageResourceCategory::scripts,
        .resource = child,
        .role = "main-script",
    }));
    assert(!bundle.add(StageMember{
        .category = StageResourceCategory::scripts,
        .resource = child,
        .role = "duplicate",
    }));
    assert(bundle.valid());
    assert(bundle.members(StageResourceCategory::scripts).size() == 1U);
}

void test_pe_reader() {
    const auto bytes = make_synthetic_pe32_plus();
    const auto result = PeReader::read(std::span<const std::byte>{bytes});
    assert(result.ok());
    assert(result.errors.empty());
    assert(result.image->kind == PeKind::pe32_plus);
    assert(result.image->machine == PeMachine::amd64);
    assert(result.image->image_base == 0x140000000ULL);
    assert(result.image->entry_point_rva == 0x1000U);
    assert(result.image->sections.size() == 1U);
    assert(result.image->sections[0].name == ".text");
    assert(result.image->rva_to_file_offset(0x1000U) == 0x200U);
    assert(result.image->file_offset_to_rva(0x200U) == 0x1000U);
    assert(result.image->rva_to_va(0x1000U) == 0x140001000ULL);

    std::vector<std::byte> invalid(32U, std::byte{0});
    const auto invalid_result = PeReader::read(
        std::span<const std::byte>{invalid});
    assert(!invalid_result.ok());
    assert(!invalid_result.errors.empty());
}

} // namespace

int main() {
    static_assert(dmc::rengine::version() == "0.2.0");
    test_resource_identity();
    test_evidence_registry();
    test_local_directory_source();
    test_graph_router_and_stage_bundle();
    test_pe_reader();
    return 0;
}
