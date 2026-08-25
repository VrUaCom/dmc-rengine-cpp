#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_path_reflow_writer.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> ascii(std::string_view value) {
    std::vector<std::byte> bytes;
    bytes.reserve(value.size());
    for (const char ch : value) {
        bytes.push_back(static_cast<std::byte>(ch));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_inner_pnst() {
    std::vector<std::byte> bytes(0x50U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x30U);
    put_u32(bytes, 16U, 0U);
    for (std::size_t index = 0x14U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0x80U + index);
    }
    const auto leaf = ascii("LEAF-ORIGINAL-01");
    assert(leaf.size() == 0x10U);
    std::copy(leaf.begin(), leaf.end(), bytes.begin() + 0x20U);
    const auto sibling = ascii("INNER-SIBLING-BYTE-EXACT-KEEP!!!!");
    assert(sibling.size() >= 0x20U);
    std::copy_n(sibling.begin(), 0x20U, bytes.begin() + 0x30U);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_outer_pac() {
    const auto inner = make_inner_pnst();
    std::vector<std::byte> bytes(0x90U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x70U);
    put_u32(bytes, 16U, 0U);
    for (std::size_t index = 0x14U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0x40U + index);
    }
    std::copy(inner.begin(), inner.end(), bytes.begin() + 0x20U);
    const auto sibling = ascii("OUTER-SIBLING-BYTE-EXACT-KEEP!!!!");
    assert(sibling.size() >= 0x20U);
    std::copy_n(sibling.begin(), 0x20U, bytes.begin() + 0x70U);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload root_payload(
    std::vector<std::byte> bytes) {
    dmc::rengine::gdspaces::ResourceId id{
        .source_id = "nested-path-test",
        .logical_path = "GData.afs/nested.pac",
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = id,
            .display_name = "nested.pac",
            .format = "PAC",
            .profile = "DMC3",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion expand(
    const dmc::rengine::gdspaces::ResourcePayload& payload) {
    const auto registry = dmc::rengine::profiles::dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{payload.bytes.data(), payload.bytes.size()},
        payload.resource.id.logical_path);
    assert(parsed.ok());
    const auto expansion = dmc::rengine::gdspaces::ContainerExpander::expand(payload, parsed);
    assert(expansion.usable());
    return expansion;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto original = root_payload(make_outer_pac());
    const auto original_outer = expand(original);
    assert(original_outer.children.size() == 3U);
    const auto original_outer_sibling = original_outer.children[1].payload.bytes;
    const auto original_inner = expand(original_outer.children[0].payload);
    assert(original_inner.children.size() == 3U);
    const auto original_inner_sibling = original_inner.children[1].payload.bytes;

    const auto replacement = ascii("LEAF-EXPANDED-THROUGH-NESTED-SLOT-PATH-REFLOW");
    assert(replacement.size() > original_inner.children[0].payload.bytes.size());
    const std::array<unsigned int, 2U> path{0U, 0U};
    const auto rebuilt = dmc3::RelativeSlotPathReflowWriter::rebuild(
        original,
        path,
        std::span<const std::byte>{replacement.data(), replacement.size()});
    assert(rebuilt.ok());
    assert(rebuilt.receipt->levels.size() == 2U);
    assert(rebuilt.receipt->slot_path == std::vector<unsigned int>({0U, 0U}));
    assert(rebuilt.receipt->levels[0].parser_format == "PAC");
    assert(rebuilt.receipt->levels[1].parser_format == "PNST");
    assert(rebuilt.bytes.size() > original.bytes.size());

    auto reopened = original;
    reopened.bytes = rebuilt.bytes;
    reopened.resource.id.size = static_cast<std::uint64_t>(reopened.bytes.size());
    const auto rebuilt_outer = expand(reopened);
    assert(rebuilt_outer.children[1].payload.bytes == original_outer_sibling);
    const auto rebuilt_inner = expand(rebuilt_outer.children[0].payload);
    assert(rebuilt_inner.children[0].payload.bytes == replacement);
    assert(rebuilt_inner.children[1].payload.bytes == original_inner_sibling);

    const std::array<unsigned int, 0U> empty_path{};
    const auto empty = dmc3::RelativeSlotPathReflowWriter::rebuild(
        original,
        empty_path,
        std::span<const std::byte>{replacement.data(), replacement.size()});
    assert(empty.status == dmc3::RelativeSlotPathReflowStatus::empty_slot_path);

    const std::array<unsigned int, 1U> out_of_range{99U};
    const auto invalid = dmc3::RelativeSlotPathReflowWriter::rebuild(
        original,
        out_of_range,
        std::span<const std::byte>{replacement.data(), replacement.size()});
    assert(invalid.status == dmc3::RelativeSlotPathReflowStatus::slot_out_of_range);

    const std::array<unsigned int, 2U> descends_into_leaf{0U, 1U};
    const std::array<unsigned int, 3U> too_deep{0U, 1U, 0U};
    const auto non_container = dmc3::RelativeSlotPathReflowWriter::rebuild(
        original,
        too_deep,
        std::span<const std::byte>{replacement.data(), replacement.size()});
    assert(non_container.status == dmc3::RelativeSlotPathReflowStatus::nested_rebuild_failed);
    (void)descends_into_leaf;

    return 0;
}
