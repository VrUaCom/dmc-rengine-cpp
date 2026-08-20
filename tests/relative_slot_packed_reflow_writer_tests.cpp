#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
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

[[nodiscard]] std::uint32_t u32(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] std::string sha256_of(
    const std::vector<std::byte>& bytes) {
    return dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{bytes.data(), bytes.size()}).hex();
}

[[nodiscard]] std::vector<std::byte> nested_pnst() {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    bytes[0x10U] = std::byte{'D'};
    bytes[0x11U] = std::byte{'D'};
    bytes[0x12U] = std::byte{'S'};
    bytes[0x13U] = std::byte{' '};
    for (std::size_t index = 0x14U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(index & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> outer_pac() {
    const auto nested = nested_pnst();
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 4U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x20U);
    put_u32(bytes, 16U, 0x60U);
    put_u32(bytes, 20U, 0U);

    // Opaque pre-payload bytes after the offset table must survive reflow.
    for (std::size_t index = 0x18U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0xA0U + index - 0x18U);
    }

    std::copy(nested.begin(), nested.end(), bytes.begin() + 0x20U);
    bytes[0x60U] = std::byte{'D'};
    bytes[0x61U] = std::byte{'D'};
    bytes[0x62U] = std::byte{'S'};
    bytes[0x63U] = std::byte{' '};
    for (std::size_t index = 0x64U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0x55U ^ index);
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload parent_payload() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = outer_pac();
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "packed-reflow-source",
                .logical_path = "GData.afs/packed-reflow.pac",
                .container_chain = "nbz[7]",
                .offset = 0x1000U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "packed-reflow.pac",
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

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredChildImage authored(
    const dmc::rengine::gdspaces::ContainerChild& child,
    std::vector<std::byte> bytes) {
    return dmc::rengine::profiles::dmc3::AuthoredChildImage{
        .resource = child.payload.resource.id,
        .source_sha256 = sha256_of(child.payload.bytes),
        .output_sha256 = sha256_of(bytes),
        .revision = 1U,
        .writer_mode = "test-authored-complete-image",
        .bytes = std::move(bytes),
    };
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto parent = parent_payload();
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parsed.ok());
    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    assert(expansion.usable());
    assert(expansion.children.size() == 4U);
    assert(expansion.children[0].entry.offset == 0x20U);
    assert(expansion.children[1].entry.offset == 0x20U);
    assert(expansion.children[2].entry.offset == 0x60U);
    assert(!expansion.children[3].entry.populated);
    assert(expansion.children[0].payload.resource.container);

    auto grown_nested = expansion.children[0].payload.bytes;
    for (std::size_t index = 0U; index < 0x10U; ++index) {
        grown_nested.push_back(static_cast<std::byte>(0xE0U + index));
    }
    const auto grown = authored(expansion.children[0], grown_nested);
    const std::vector<dmc3::AuthoredChildImage> grow_set{grown};
    const auto grow_result = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, grow_set);
    assert(grow_result.ok());
    assert(grow_result.bytes.size() == 0x90U);
    assert(grow_result.receipt->source_topology.container_size == 0x80U);
    assert(grow_result.receipt->output_topology.container_size == 0x90U);
    assert(grow_result.receipt->source_topology.protected_prefix_size == 0x20U);
    assert(grow_result.receipt->output_topology.protected_prefix_size == 0x20U);
    assert(grow_result.receipt->spans.size() == 2U);
    assert(grow_result.receipt->spans[0].changed);
    assert(grow_result.receipt->spans[0].source_size == 0x40U);
    assert(grow_result.receipt->spans[0].output_size == 0x50U);
    assert(grow_result.receipt->spans[0].affected_aliases.size() == 2U);
    assert(grow_result.receipt->spans[0].authored_aliases.size() == 1U);
    assert(!grow_result.receipt->spans[1].changed);
    assert(grow_result.receipt->spans[1].source_offset == 0x60U);
    assert(grow_result.receipt->spans[1].output_offset == 0x70U);

    assert(u32(grow_result.bytes, 8U) == 0x20U);
    assert(u32(grow_result.bytes, 12U) == 0x20U);
    assert(u32(grow_result.bytes, 16U) == 0x70U);
    assert(u32(grow_result.bytes, 20U) == 0U);
    assert(std::equal(
        parent.bytes.begin() + 0x18,
        parent.bytes.begin() + 0x20,
        grow_result.bytes.begin() + 0x18));
    assert(std::equal(
        parent.bytes.begin() + 0x60,
        parent.bytes.end(),
        grow_result.bytes.begin() + 0x70));

    const auto reparsed_grow = registry.parse(
        std::span<const std::byte>{
            grow_result.bytes.data(), grow_result.bytes.size()},
        parent.resource.id.logical_path);
    assert(reparsed_grow.ok());
    assert(reparsed_grow.document.entries[0].offset == 0x20U);
    assert(reparsed_grow.document.entries[0].size == 0x50U);
    assert(reparsed_grow.document.entries[1].offset == 0x20U);
    assert(reparsed_grow.document.entries[1].size == 0x50U);
    assert(reparsed_grow.document.entries[2].offset == 0x70U);
    assert(reparsed_grow.document.entries[2].size == 0x20U);
    assert(!reparsed_grow.document.entries[3].populated);

    auto shrink_nested = expansion.children[0].payload.bytes;
    shrink_nested.resize(0x30U);
    const auto shrink = authored(expansion.children[0], shrink_nested);
    const std::vector<dmc3::AuthoredChildImage> shrink_set{shrink};
    const auto shrink_result = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, shrink_set);
    assert(shrink_result.ok());
    assert(shrink_result.bytes.size() == 0x70U);
    assert(u32(shrink_result.bytes, 16U) == 0x50U);
    assert(std::equal(
        parent.bytes.begin() + 0x60,
        parent.bytes.end(),
        shrink_result.bytes.begin() + 0x50));

    auto alias_same = grown;
    alias_same.resource = expansion.children[1].payload.resource.id;
    alias_same.source_sha256 = sha256_of(expansion.children[1].payload.bytes);
    const std::vector<dmc3::AuthoredChildImage> aliases_same{grown, alias_same};
    const auto alias_success = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, aliases_same);
    assert(alias_success.ok());
    assert(alias_success.receipt->spans[0].authored_aliases.size() == 2U);

    auto alias_different_bytes = expansion.children[1].payload.bytes;
    alias_different_bytes.resize(0x50U, std::byte{0x7A});
    const auto alias_different = authored(
        expansion.children[1], std::move(alias_different_bytes));
    const std::vector<dmc3::AuthoredChildImage> aliases_conflict{
        grown, alias_different};
    assert(
        dmc3::RelativeSlotPackedReflowWriter::rebuild(
            parent, expansion, aliases_conflict).status ==
        dmc3::RelativeSlotPackedReflowStatus::alias_conflict);

    auto stale = grown;
    stale.source_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredChildImage> stale_set{stale};
    assert(
        dmc3::RelativeSlotPackedReflowWriter::rebuild(
            parent, expansion, stale_set).status ==
        dmc3::RelativeSlotPackedReflowStatus::child_source_mismatch);

    auto invalid_topology_bytes = grown_nested;
    put_u32(invalid_topology_bytes, 4U, 2U);
    put_u32(invalid_topology_bytes, 8U, 0x10U);
    put_u32(invalid_topology_bytes, 12U, 0x20U);
    const auto invalid_topology = authored(
        expansion.children[0], std::move(invalid_topology_bytes));
    const std::vector<dmc3::AuthoredChildImage> invalid_topology_set{
        invalid_topology};
    assert(
        dmc3::RelativeSlotPackedReflowWriter::rebuild(
            parent, expansion, invalid_topology_set).status ==
        dmc3::RelativeSlotPackedReflowStatus::child_writer_validation_failed);

    const auto unchanged = authored(
        expansion.children[0], expansion.children[0].payload.bytes);
    const std::vector<dmc3::AuthoredChildImage> unchanged_set{unchanged};
    assert(
        dmc3::RelativeSlotPackedReflowWriter::rebuild(
            parent, expansion, unchanged_set).status ==
        dmc3::RelativeSlotPackedReflowStatus::no_changes);

    return 0;
}
