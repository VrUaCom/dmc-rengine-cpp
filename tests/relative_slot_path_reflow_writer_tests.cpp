#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_path_reflow_writer.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> inner_pnst() {
    std::vector<std::byte> bytes(0x50U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x30U);
    put_u32(bytes, 16U, 0U);
    const auto leaf = ascii("LEAF-ORIGINAL-01");
    assert(leaf.size() == 0x10U);
    std::copy(leaf.begin(), leaf.end(), bytes.begin() + 0x20U);
    const auto sibling = ascii("INNER-SIBLING-BYTE-EXACT-KEEP!!!!");
    assert(sibling.size() >= 0x20U);
    std::copy_n(sibling.begin(), 0x20U, bytes.begin() + 0x30U);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> outer_pac() {
    const auto nested = inner_pnst();
    std::vector<std::byte> bytes(0x90U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x70U);
    put_u32(bytes, 16U, 0U);
    std::copy(nested.begin(), nested.end(), bytes.begin() + 0x20U);
    const auto sibling = ascii("OUTER-SIBLING-BYTE-EXACT-KEEP!!!!");
    assert(sibling.size() >= 0x20U);
    std::copy_n(sibling.begin(), 0x20U, bytes.begin() + 0x70U);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload payload(
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "slot-path-test",
                .logical_path = "GData.afs/nested.pac",
                .container_chain = {},
                .offset = 0U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "nested.pac",
            .format = "PAC",
            .profile = "dmc3",
            .container = true,
        },
        .bytes = std::move(bytes),
    };
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto root = payload(outer_pac());
    const auto parsers = dmc3::make_container_parser_registry();
    const auto parsed_root = parsers.parse(root.bytes, root.resource.id.logical_path);
    assert(parsed_root.ok());
    const auto expanded_root = gdspaces::ContainerExpander::expand(root, parsed_root);
    assert(expanded_root.usable());
    const auto original_outer_sibling = expanded_root.children[1].payload.bytes;

    const auto parsed_inner = parsers.parse(
        expanded_root.children[0].payload.bytes,
        expanded_root.children[0].payload.resource.id.logical_path);
    assert(parsed_inner.ok());
    const auto expanded_inner = gdspaces::ContainerExpander::expand(
        expanded_root.children[0].payload, parsed_inner);
    assert(expanded_inner.usable());
    const auto original_inner_sibling = expanded_inner.children[1].payload.bytes;

    const auto replacement = ascii(
        "LEAF-REPLACEMENT-THAT-GROWS-THROUGH-TWO-CONTAINER-LEVELS");
    const std::vector<unsigned int> path{0U, 0U};
    const auto rebuilt = dmc3::RelativeSlotPathReflowWriter::rebuild(
        root, path, replacement);
    assert(rebuilt.ok());
    assert(rebuilt.receipt->levels.size() == 2U);
    assert(rebuilt.receipt->slot_path == path);
    assert(rebuilt.bytes.size() > root.bytes.size());

    auto reopened = root;
    reopened.bytes = rebuilt.bytes;
    reopened.resource.id.size = static_cast<std::uint64_t>(reopened.bytes.size());
    const auto reparsed_root = parsers.parse(
        reopened.bytes, reopened.resource.id.logical_path);
    assert(reparsed_root.ok());
    const auto reexpanded_root = gdspaces::ContainerExpander::expand(
        reopened, reparsed_root);
    assert(reexpanded_root.usable());
    assert(reexpanded_root.children[1].payload.bytes == original_outer_sibling);

    const auto reparsed_inner = parsers.parse(
        reexpanded_root.children[0].payload.bytes,
        reexpanded_root.children[0].payload.resource.id.logical_path);
    assert(reparsed_inner.ok());
    const auto reexpanded_inner = gdspaces::ContainerExpander::expand(
        reexpanded_root.children[0].payload, reparsed_inner);
    assert(reexpanded_inner.usable());
    assert(reexpanded_inner.children[0].payload.bytes == replacement);
    assert(reexpanded_inner.children[1].payload.bytes == original_inner_sibling);

    const std::vector<unsigned int> empty_path;
    const auto empty = dmc3::RelativeSlotPathReflowWriter::rebuild(
        root, empty_path, replacement);
    assert(empty.status == dmc3::RelativeSlotPathReflowStatus::empty_slot_path);

    const std::vector<unsigned int> too_deep_path(65U, 0U);
    const auto too_deep = dmc3::RelativeSlotPathReflowWriter::rebuild(
        root, too_deep_path, replacement);
    assert(too_deep.status == dmc3::RelativeSlotPathReflowStatus::path_too_deep);

    const std::vector<unsigned int> bad_path{2U};
    const auto empty_slot = dmc3::RelativeSlotPathReflowWriter::rebuild(
        root, bad_path, replacement);
    assert(empty_slot.status == dmc3::RelativeSlotPathReflowStatus::empty_slot);

    return 0;
}
