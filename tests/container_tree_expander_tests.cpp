#include "dmc_rengine/gdspaces/container_tree_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace {

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
    std::string_view value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] std::vector<std::byte> make_nested_fixture() {
    std::vector<std::byte> bytes(80U, std::byte{0});

    // Outer PAC: slot 0 is a nested PAC [32,64), slot 1 is SCM [64,80).
    write_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 4U, 2U);
    write_u32(bytes, 8U, 32U);
    write_u32(bytes, 12U, 64U);

    // Nested PAC at outer +32: one DDS-like child at nested +16.
    write_ascii(bytes, 32U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 36U, 1U);
    write_u32(bytes, 40U, 16U);
    write_ascii(bytes, 48U, "DDS ");

    write_ascii(bytes, 64U, std::string_view{"SCM\0", 4U});
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_root(
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "recursive-test",
                .logical_path = "DMC3/root.pac",
                .container_chain = {},
                .offset = 1000U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "root.pac",
            .format = "pac",
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
    using dmc::rengine::gdspaces::ContainerTreeExpander;
    using dmc::rengine::gdspaces::ContainerTreeExpansionLimits;
    using dmc::rengine::gdspaces::DiagnosticSeverity;
    using dmc::rengine::gdspaces::ResourceRelation;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    auto registry = make_container_parser_registry();
    const auto root = make_root(make_nested_fixture());

    const auto tree = ContainerTreeExpander::expand(root, registry);
    assert(tree.complete());
    assert(tree.fully_expanded);
    assert(tree.expanded_container_count == 2U);
    assert(tree.expansions.size() == 2U);
    assert(tree.graph.resource_count() == 4U);
    assert(tree.graph.edge_count() == 3U);

    const auto root_children = tree.graph.related(
        root.resource.id, ResourceRelation::contains);
    assert(root_children.size() == 2U);

    const auto* nested = root_children[0];
    assert(nested != nullptr);
    assert(nested->format == "pac");
    assert(nested->container);
    assert(nested->id.offset == 1032U);
    assert(nested->id.container_chain == "pac[0]");

    const auto nested_children = tree.graph.related(
        nested->id, ResourceRelation::contains);
    assert(nested_children.size() == 1U);
    assert(nested_children[0]->format == "dds");
    assert(nested_children[0]->id.offset == 1048U);
    assert(nested_children[0]->id.container_chain == "pac[0]/pac[0]");

    // Depth 0 still expands the root but preserves the nested container as an
    // unexpanded node with an explicit diagnostic.
    const auto shallow = ContainerTreeExpander::expand(
        root,
        registry,
        ContainerTreeExpansionLimits{
            .max_nested_depth = 0U,
            .max_expanded_containers = 16U,
            .max_graph_nodes = 64U,
            .max_parsed_container_bytes = 4096U,
        });
    assert(!shallow.complete());
    assert(!shallow.fully_expanded);
    assert(shallow.expanded_container_count == 1U);
    assert(shallow.graph.resource_count() == 3U);

    bool found_depth_warning = false;
    for (const auto& diagnostic : shallow.diagnostics) {
        if (diagnostic.severity == DiagnosticSeverity::warning &&
            diagnostic.code == "gdspaces.container-tree.depth-budget") {
            found_depth_warning = true;
        }
    }
    assert(found_depth_warning);

    // A byte budget smaller than the root blocks parsing deterministically.
    const auto byte_limited = ContainerTreeExpander::expand(
        root,
        registry,
        ContainerTreeExpansionLimits{
            .max_nested_depth = 4U,
            .max_expanded_containers = 16U,
            .max_graph_nodes = 64U,
            .max_parsed_container_bytes = 32U,
        });
    assert(!byte_limited.complete());
    assert(byte_limited.expanded_container_count == 0U);
    assert(byte_limited.graph.resource_count() == 1U);

    return 0;
}
