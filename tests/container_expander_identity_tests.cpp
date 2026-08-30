#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

dmc::rengine::gdspaces::ResourcePayload parent_payload() {
    using namespace dmc::rengine::gdspaces;
    std::vector<std::byte> bytes(0x30U, std::byte{0});
    return ResourcePayload{
        .resource = ResourceRef{
            .id = ResourceId{
                .source_id = "identity-test",
                .logical_path = "DMC3/root.pac",
                .container_chain = {},
                .offset = 0U,
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
        .byte_provenance = std::nullopt,
    };
}

dmc::rengine::formats::ContainerParseResult parsed_with_name(std::string name) {
    using namespace dmc::rengine::formats;
    return ContainerParseResult{
        .document = ContainerDocument{
            .format = "PAC",
            .schema_version = 1U,
            .declared_slot_count = 1U,
            .container_size = 0x30U,
            .entries = {
                ContainerEntry{
                    .slot_index = 0U,
                    .offset = 0x20U,
                    .size = 4U,
                    .logical_name = std::move(name),
                    .populated = true,
                    .synthetic_name = true,
                },
            },
        },
        .diagnostics = {},
        .recognized = true,
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ContainerExpander;

    const auto parent = parent_payload();
    const auto first = ContainerExpander::expand(
        parent, parsed_with_name("slot_0000.bin"));
    const auto renamed = ContainerExpander::expand(
        parent, parsed_with_name("st001_000.ukn"));

    assert(first.usable());
    assert(renamed.usable());
    assert(first.children.size() == 1U);
    assert(renamed.children.size() == 1U);

    const auto& before = first.children.front().payload.resource;
    const auto& after = renamed.children.front().payload.resource;

    assert(before.display_name != after.display_name);
    assert(before.id == after.id);
    assert(before.id.canonical() == after.id.canonical());
    assert(before.id.logical_path == "DMC3/root.pac::PAC/slot-0000");
    assert(before.id.container_chain == "PAC[0]");
    assert(before.id.offset == 0x20U);
    assert(before.id.size == 4U);

    return 0;
}
