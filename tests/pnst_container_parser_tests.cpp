#include "dmc_rengine/formats/dmc3/pnst_container_parser.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

[[nodiscard]] std::vector<std::byte> make_sparse_fixture() {
    std::vector<std::byte> bytes(112U, std::byte{0});
    write_ascii(bytes, 0U, "PNST");
    write_u32(bytes, 4U, 11U);

    // Preserve the confirmed sparse topology shape: original slots 0 and 10
    // are populated while slots 1..9 remain empty.
    write_u32(bytes, 8U, 64U);
    write_u32(bytes, 8U + 10U * 4U, 96U);

    write_ascii(bytes, 64U, std::string_view{"PAC\0", 4U});
    write_ascii(bytes, 96U, "DDS ");
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_parent(
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "pnst-test",
                .logical_path = "DMC3/em000.pac",
                .container_chain = "pac[5]",
                .offset = 500U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "em000.pac",
            .format = "pnst",
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
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::dmc3::PnstContainerParser;
    using dmc::rengine::gdspaces::ContainerExpander;
    using dmc::rengine::gdspaces::ResourceClassifier;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    PnstContainerParser parser;
    assert(parser.id() == "dmc3-pnst-runtime-v1");
    assert(parser.format() == "pnst");

    const auto bytes = make_sparse_fixture();
    assert(parser.probe(std::span<const std::byte>{bytes}, "em000.pac") == 100);

    const auto classified = ResourceClassifier::classify(
        "misleading.pac", std::span<const std::byte>{bytes});
    assert(classified.format == "pnst");
    assert(classified.magic_confirmed);
    assert(classified.container);

    const auto parsed = parser.parse(
        std::span<const std::byte>{bytes}, "em000.pac");
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.valid());
    assert(parsed.document.declared_slot_count == 11U);
    assert(parsed.document.entries.size() == 11U);

    assert(parsed.document.entries[0].populated);
    assert(parsed.document.entries[0].offset == 64U);
    assert(parsed.document.entries[0].size == 32U);
    for (std::size_t slot = 1U; slot < 10U; ++slot) {
        assert(!parsed.document.entries[slot].populated);
        assert(parsed.document.entries[slot].slot_index == slot);
    }
    assert(parsed.document.entries[10].populated);
    assert(parsed.document.entries[10].offset == 96U);
    assert(parsed.document.entries[10].size == 16U);

    auto registry = make_container_parser_registry();
    assert(registry.size() == 2U);
    assert(registry.find_by_id("dmc3-pnst-runtime-v1") != nullptr);
    const auto registered = registry.parse(
        std::span<const std::byte>{bytes}, "DMC3/em000.pac");
    assert(registered.ok());
    assert(registered.document.format == "pnst");

    const auto parent = make_parent(bytes);
    const auto expansion = ContainerExpander::expand(parent, registered);
    assert(expansion.usable());
    assert(expansion.children.size() == 11U);
    assert(expansion.children[0].payload.resource.format == "pac");
    assert(expansion.children[0].payload.resource.container);
    assert(expansion.children[0].payload.resource.id.offset == 564U);
    assert(expansion.children[0].payload.resource.id.container_chain == "pac[5]/pnst[0]");
    for (std::size_t slot = 1U; slot < 10U; ++slot) {
        assert(expansion.children[slot].payload.resource.format == "empty-slot");
    }
    assert(expansion.children[10].payload.resource.format == "dds");
    assert(expansion.children[10].payload.resource.id.offset == 596U);
    assert(expansion.children[10].payload.resource.id.container_chain == "pac[5]/pnst[10]");

    auto malformed = make_sparse_fixture();
    write_u32(malformed, 8U + 10U * 4U, 16U);
    const auto malformed_result = parser.parse(
        std::span<const std::byte>{malformed}, "bad.pac");
    assert(malformed_result.recognized);
    assert(!malformed_result.ok());

    bool found_range_error = false;
    for (const auto& diagnostic : malformed_result.diagnostics) {
        if (diagnostic.severity == ParseSeverity::error &&
            diagnostic.code == "dmc3.pnst.offset_out_of_range") {
            found_range_error = true;
        }
    }
    assert(found_range_error);

    return 0;
}
