#include "dmc_rengine/gdspaces/container_tree_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace {
void u32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) {
    for (std::size_t i = 0; i < 4; ++i) {
        b[o + i] = static_cast<std::byte>((v >> (i * 8U)) & 0xFFU);
    }
}
void text(std::vector<std::byte>& b, std::size_t o, std::string_view v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        b[o + i] = static_cast<std::byte>(v[i]);
    }
}
std::vector<std::byte> fixture() {
    std::vector<std::byte> b(0x60U, std::byte{0});
    text(b, 0, std::string_view{"PAC\0", 4});
    u32(b, 4, 3); u32(b, 8, 0x20); u32(b, 12, 0x20); u32(b, 16, 0x50);
    text(b, 0x20, std::string_view{"PAC\0", 4});
    u32(b, 0x24, 1); u32(b, 0x28, 0x10); text(b, 0x30, "DDS ");
    text(b, 0x50, "SCM");
    return b;
}
dmc::rengine::gdspaces::ResourcePayload root() {
    using namespace dmc::rengine::gdspaces;
    auto bytes = fixture();
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return ResourcePayload{
        .resource = ResourceRef{
            .id = ResourceId{"tree-test", "DMC3/root.pac", {}, 0U, size},
            .display_name = "root.pac", .format = "pac", .profile = "dmc3-hd",
            .synthetic_name = false, .container = true},
        .bytes = std::move(bytes), .diagnostics = {},
        .byte_provenance = ByteProvenance{
            .kind = ByteOriginKind::direct_source_span, .authority_id = "fixture",
            .offset = 1000U, .stored_size = size, .materialized_size = size,
            .transform = ByteTransform::none, .crc32 = std::nullopt}}
    ;
}
} // namespace

int main() {
    using namespace dmc::rengine::gdspaces;
    const auto registry = dmc::rengine::profiles::dmc3::make_container_parser_registry();
    const auto source = root();
    const auto tree = ContainerTreeExpander::expand(source, registry);
    assert(tree.complete());
    assert(tree.expanded_container_count == 3U);
    assert(tree.parser_invocation_count == 2U);
    assert(tree.parse_cache_hits == 1U);
    assert(tree.parsed_container_bytes == 0x90U);
    assert(tree.graph.resource_count() == 6U);
    assert(tree.graph.edge_count() == 5U);

    const auto children = tree.graph.related(source.resource.id, ResourceRelation::contains);
    assert(children.size() == 3U);
    const ResourceRef* a = nullptr; const ResourceRef* b = nullptr;
    for (const auto* child : children) {
        if (child->id.container_chain == "PAC[0]") a = child;
        if (child->id.container_chain == "PAC[1]") b = child;
    }
    assert(a != nullptr && b != nullptr);
    assert(a->id.canonical() != b->id.canonical());
    assert(a->id.offset == b->id.offset && a->id.size == b->id.size);
    const auto ac = tree.graph.related(a->id, ResourceRelation::contains);
    const auto bc = tree.graph.related(b->id, ResourceRelation::contains);
    assert(ac.size() == 1U && bc.size() == 1U);
    assert(ac.front()->format == "dds" && bc.front()->format == "dds");
    assert(ac.front()->id.canonical() != bc.front()->id.canonical());

    ContainerTreeExpansionLimits depth_limited;
    depth_limited.max_nested_depth = 0U;
    const auto shallow = ContainerTreeExpander::expand(root(), registry, depth_limited);
    assert(!shallow.complete());
    assert(shallow.expanded_container_count == 1U);

    ContainerTreeExpansionLimits byte_limited;
    byte_limited.max_parsed_container_bytes = 0x5FU;
    const auto byte_blocked = ContainerTreeExpander::expand(root(), registry, byte_limited);
    assert(!byte_blocked.complete());
    assert(byte_blocked.parser_invocation_count == 0U);
    assert(byte_blocked.expanded_container_count == 0U);

    ContainerTreeExpansionLimits container_limited;
    container_limited.max_expanded_containers = 1U;
    const auto count_blocked = ContainerTreeExpander::expand(root(), registry, container_limited);
    assert(!count_blocked.complete());
    assert(count_blocked.expanded_container_count == 1U);
    assert(count_blocked.parser_invocation_count == 1U);

    return 0;
}
