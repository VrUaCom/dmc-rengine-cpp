#include "dmc_rengine/formats/dmc3/pac_container_parser.hpp"
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

[[nodiscard]] std::vector<std::byte> make_valid_fixture() {
    std::vector<std::byte> bytes(80U, std::byte{0});
    write_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 4U, 5U);

    // Sparse runtime slot table. Slots 2 and 3 intentionally share one
    // physical offset; the parser preserves this without assigning semantics.
    write_u32(bytes, 8U, 32U);
    write_u32(bytes, 12U, 0U);
    write_u32(bytes, 16U, 48U);
    write_u32(bytes, 20U, 48U);
    write_u32(bytes, 24U, 64U);

    write_ascii(bytes, 32U, "DDS ");
    write_ascii(bytes, 48U, std::string_view{"SCM\0", 4U});
    write_ascii(bytes, 64U, std::string_view{"PAC\0", 4U});
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_parent(
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "dmc3-local",
                .logical_path = "DMC3/scr/st777.pac",
                .container_chain = {},
                .offset = 1000U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "st777.pac",
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
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::dmc3::PacContainerParser;
    using dmc::rengine::gdspaces::ContainerExpander;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    PacContainerParser parser;
    assert(parser.id() == "dmc3-pac-runtime-v1");
    assert(parser.format() == "pac");

    const auto bytes = make_valid_fixture();
    assert(parser.probe(std::span<const std::byte>{bytes}, "stage.pac") == 100);

    const auto parsed = parser.parse(
        std::span<const std::byte>{bytes}, "stage.pac");
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.valid());
    assert(parsed.document.declared_slot_count == 5U);
    assert(parsed.document.entries.size() == 5U);

    assert(parsed.document.entries[0].populated);
    assert(parsed.document.entries[0].offset == 32U);
    assert(parsed.document.entries[0].size == 16U);
    assert(parsed.document.entries[0].slot_index == 0U);
    assert(parsed.document.entries[0].synthetic_name);

    assert(!parsed.document.entries[1].populated);
    assert(parsed.document.entries[1].slot_index == 1U);

    assert(parsed.document.entries[2].populated);
    assert(parsed.document.entries[2].offset == 48U);
    assert(parsed.document.entries[2].size == 16U);
    assert(parsed.document.entries[3].populated);
    assert(parsed.document.entries[3].offset == 48U);
    assert(parsed.document.entries[3].size == 16U);

    assert(parsed.document.entries[4].populated);
    assert(parsed.document.entries[4].offset == 64U);
    assert(parsed.document.entries[4].size == 16U);

    bool found_shared_offset_warning = false;
    for (const auto& diagnostic : parsed.diagnostics) {
        if (diagnostic.severity == ParseSeverity::warning &&
            diagnostic.code == "dmc3.pac.duplicate_offset") {
            found_shared_offset_warning = true;
        }
    }
    assert(found_shared_offset_warning);

    // Production profile registration -> generic GDSpaces expansion. This is
    // deliberately independent of Stage Ops/Binary Inspector ownership.
    auto registry = make_container_parser_registry();
    assert(registry.size() == 1U);
    assert(registry.find_by_id("dmc3-pac-runtime-v1") != nullptr);
    const auto registered_parse = registry.parse(
        std::span<const std::byte>{bytes}, "DMC3/scr/st777.pac");
    assert(registered_parse.ok());

    const auto parent = make_parent(bytes);
    const auto expansion = ContainerExpander::expand(parent, registered_parse);
    assert(expansion.usable());
    assert(expansion.children.size() == 5U);
    assert(expansion.children[0].payload.resource.format == "dds");
    assert(expansion.children[0].payload.resource.id.offset == 1032U);
    assert(expansion.children[0].payload.resource.id.container_chain == "pac[0]");
    assert(expansion.children[1].payload.resource.format == "empty-slot");
    assert(expansion.children[1].payload.resource.id.container_chain == "pac[1]");
    assert(expansion.children[2].payload.resource.format == "scm");
    assert(expansion.children[3].payload.resource.format == "scm");
    assert(expansion.children[4].payload.resource.format == "pac");
    assert(expansion.children[4].payload.resource.container);

    auto malformed_bytes = make_valid_fixture();
    write_u32(malformed_bytes, 24U, 12U);
    const auto malformed = parser.parse(
        std::span<const std::byte>{malformed_bytes}, "malformed.pac");
    assert(malformed.recognized);
    assert(!malformed.ok());
    assert(malformed.document.entries.size() == 5U);
    assert(!malformed.document.entries[4].populated);

    bool found_range_error = false;
    for (const auto& diagnostic : malformed.diagnostics) {
        if (diagnostic.severity == ParseSeverity::error &&
            diagnostic.code == "dmc3.pac.offset_out_of_range") {
            found_range_error = true;
        }
    }
    assert(found_range_error);

    auto not_pac = bytes;
    not_pac[2U] = std::byte{'X'};
    const auto unrecognized = parser.parse(
        std::span<const std::byte>{not_pac}, "not-pac.bin");
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    return 0;
}
