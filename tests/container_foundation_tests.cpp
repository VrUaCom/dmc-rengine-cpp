#include "dmc_rengine/formats/container_parser_registry.hpp"
#include "dmc_rengine/formats/synthetic/slot_container_parser.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace {

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

void write_ascii(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    const std::string& value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] std::vector<std::byte> make_valid_fixture() {
    std::vector<std::byte> bytes(160U, std::byte{0});
    write_ascii(bytes, 0U, "SLTC");
    write_u16(bytes, 4U, 1U);
    write_u16(bytes, 6U, 3U);
    write_u32(bytes, 8U, 16U);
    write_u32(bytes, 12U, 64U);

    // Slot 0: populated PAC-magic child with a misleading .bin name.
    write_u32(bytes, 16U, 96U);
    write_u32(bytes, 20U, 4U);
    write_u32(bytes, 24U, 1U);
    write_u32(bytes, 28U, 1U);

    // Slot 1: empty and structurally preserved.

    // Slot 2: populated DDS child.
    write_u32(bytes, 48U, 100U);
    write_u32(bytes, 52U, 4U);
    write_u32(bytes, 56U, 11U);
    write_u32(bytes, 60U, 1U);

    bytes[64U] = std::byte{0};
    write_ascii(bytes, 65U, std::string{"model.bin\0", 10U});
    write_ascii(bytes, 75U, std::string{"texture.bin\0", 12U});

    write_ascii(bytes, 96U, std::string{"PAC\0", 4U});
    write_ascii(bytes, 100U, "DDS ");
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_parent(
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic-source",
                .logical_path = "DMC3/archive.sltc",
                .container_chain = {},
                .offset = 1000U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "archive.sltc",
            .format = "synthetic-slot-container",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
    };
}

} // namespace

int main() {
    using dmc::rengine::formats::ContainerParserRegistry;
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::synthetic::SlotContainerParser;
    using dmc::rengine::gdspaces::ContainerExpander;
    using dmc::rengine::gdspaces::ResourceGraph;
    using dmc::rengine::gdspaces::ResourceRelation;

    ContainerParserRegistry empty_registry;
    const auto no_parser = empty_registry.parse({}, "unknown.bin");
    assert(!no_parser.recognized);
    assert(!no_parser.ok());
    assert(no_parser.diagnostics.size() == 1U);

    ContainerParserRegistry registry;
    assert(registry.register_parser(std::make_unique<SlotContainerParser>()));
    assert(!registry.register_parser(std::make_unique<SlotContainerParser>()));
    assert(registry.size() == 1U);

    const auto bytes = make_valid_fixture();
    const auto* parser = registry.select(
        std::span<const std::byte>{bytes}, "archive.sltc");
    assert(parser != nullptr);
    assert(parser->id() == "synthetic-slot-container-v1");

    const auto parsed = registry.parse(
        std::span<const std::byte>{bytes}, "archive.sltc");
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.valid());
    assert(parsed.document.declared_slot_count == 3U);
    assert(parsed.document.entries.size() == 3U);
    assert(parsed.document.entries[0].populated);
    assert(!parsed.document.entries[1].populated);
    assert(parsed.document.entries[2].logical_name == "texture.bin");

    const auto parent = make_parent(bytes);
    const auto expansion = ContainerExpander::expand(parent, parsed);
    assert(expansion.usable());
    assert(expansion.children.size() == 3U);
    assert(expansion.children[0].payload.resource.format == "pac");
    assert(expansion.children[0].payload.resource.container);
    assert(expansion.children[0].payload.resource.id.offset == 1096U);
    assert(expansion.children[1].payload.resource.format == "empty-slot");
    assert(expansion.children[1].payload.bytes.empty());
    assert(expansion.children[2].payload.resource.format == "dds");
    assert(expansion.children[2].payload.resource.profile == "dmc3-hd");
    assert(expansion.children[2].payload.resource.id.container_chain ==
        "synthetic-slot-container[2]");

    ResourceGraph graph;
    ContainerExpander::connect_graph(expansion, graph);
    assert(graph.resource_count() == 4U);
    assert(graph.edge_count() == 3U);
    assert(graph.related(parent.resource.id, ResourceRelation::contains).size() == 3U);

    auto malformed_bytes = make_valid_fixture();
    write_u32(malformed_bytes, 32U, 200U);
    write_u32(malformed_bytes, 36U, 4U);
    write_u32(malformed_bytes, 40U, 0U);
    write_u32(malformed_bytes, 44U, 1U);

    const auto malformed = registry.parse(
        std::span<const std::byte>{malformed_bytes}, "malformed.sltc");
    assert(malformed.recognized);
    assert(!malformed.ok());
    assert(malformed.document.valid());
    assert(!malformed.document.entries[1].populated);
    assert(malformed.document.entries[1].synthetic_name);

    bool found_range_error = false;
    for (const auto& diagnostic : malformed.diagnostics) {
        if (diagnostic.severity == ParseSeverity::error &&
            diagnostic.code == "synthetic_slot.entry_out_of_range") {
            found_range_error = true;
        }
    }
    assert(found_range_error);

    const auto partial_expansion = ContainerExpander::expand(
        make_parent(malformed_bytes), malformed);
    assert(!partial_expansion.usable());
    assert(partial_expansion.children.size() == 3U);

    return 0;
}
