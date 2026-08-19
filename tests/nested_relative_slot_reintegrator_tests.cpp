#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

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

std::vector<std::byte> nested_pnst() {
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
    return bytes;
}

std::vector<std::byte> outer_pac() {
    const auto nested = nested_pnst();
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x20U);
    put_u32(bytes, 16U, 0x60U);
    std::copy(nested.begin(), nested.end(), bytes.begin() + 0x20);
    bytes[0x60U] = std::byte{'D'};
    bytes[0x61U] = std::byte{'D'};
    bytes[0x62U] = std::byte{'S'};
    bytes[0x63U] = std::byte{' '};
    return bytes;
}

dmc::rengine::gdspaces::ResourcePayload parent_payload() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = outer_pac();
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-nbz-source",
                .logical_path = "GData.afs/outer.pac",
                .container_chain = "nbz[42]",
                .offset = 0x1000U,
                .size = size,
            },
            .display_name = "outer.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = gdspaces::ByteProvenance{
            .kind = gdspaces::ByteOriginKind::transformed_source_span,
            .authority_id = "retail-nbz-source",
            .offset = 0x777U,
            .stored_size = 0x50U,
            .materialized_size = size,
            .transform = gdspaces::ByteTransform::zip_deflate,
            .crc32 = 0x12345678U,
        },
    };
}

dmc::rengine::gdspaces::EditOperation edit_byte(
    std::string id,
    std::uint64_t revision,
    std::uint64_t offset,
    std::byte expected,
    std::byte replacement) {
    return dmc::rengine::gdspaces::EditOperation{
        .id = std::move(id),
        .base_revision = revision,
        .offset = offset,
        .expected = {expected},
        .replacement = {replacement},
        .description = "Nested reintegration regression edit.",
    };
}

dmc::rengine::profiles::dmc3::AuthoredChildImage author_pnst_child(
    const dmc::rengine::gdspaces::ResourcePayload& child,
    std::byte replacement) {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    gdspaces::WorkingCopy working{child};
    const auto edited = working.apply(edit_byte(
        "nested-pnst-payload",
        0U,
        0x20U,
        std::byte{0},
        replacement));
    assert(edited.applied);
    const auto rebuilt = dmc3::RelativeSlotLayoutWriter::rebuild(child, working);
    assert(rebuilt.ok());
    return dmc3::AuthoredChildImage{
        .resource = child.resource.id,
        .source_sha256 = rebuilt.receipt->source_sha256,
        .output_sha256 = rebuilt.receipt->output_sha256,
        .revision = rebuilt.receipt->working_revision,
        .writer_mode = "layout-preserving-packed",
        .bytes = rebuilt.bytes,
    };
}

dmc::rengine::profiles::dmc3::AuthoredChildImage clean_pnst_child(
    const dmc::rengine::gdspaces::ResourcePayload& child) {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    gdspaces::WorkingCopy working{child};
    const auto rebuilt = dmc3::RelativeSlotLayoutWriter::rebuild(child, working);
    assert(rebuilt.ok());
    return dmc3::AuthoredChildImage{
        .resource = child.resource.id,
        .source_sha256 = rebuilt.receipt->source_sha256,
        .output_sha256 = rebuilt.receipt->output_sha256,
        .revision = rebuilt.receipt->working_revision,
        .writer_mode = "layout-preserving-packed",
        .bytes = rebuilt.bytes,
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
    assert(expansion.children.size() == 3U);
    assert(expansion.children[0].entry.offset == 0x20U);
    assert(expansion.children[1].entry.offset == 0x20U);
    assert(expansion.children[0].entry.size == 0x40U);
    assert(expansion.children[1].entry.size == 0x40U);
    assert(expansion.children[0].payload.resource.id !=
           expansion.children[1].payload.resource.id);
    assert(expansion.children[0].payload.resource.format == "pnst");
    assert(expansion.children[1].payload.resource.format == "pnst");
    assert(expansion.children[0].payload.byte_provenance.has_value());
    assert(
        expansion.children[0].payload.byte_provenance->kind ==
        gdspaces::ByteOriginKind::materialized_parent_span);
    assert(expansion.children[0].payload.byte_provenance->offset == 0x20U);
    assert(expansion.children[0].payload.byte_provenance->authority_id ==
           parent.resource.id.canonical());
    assert(parent.byte_provenance->offset == 0x777U);

    const auto authored0 = author_pnst_child(
        expansion.children[0].payload, std::byte{0x5A});
    const std::vector<dmc3::AuthoredChildImage> one_alias{authored0};
    const auto reintegrated = dmc3::NestedRelativeSlotReintegrator::reintegrate(
        parent, expansion, one_alias);
    assert(reintegrated.ok());
    assert(reintegrated.bytes.size() == parent.bytes.size());
    assert(reintegrated.bytes[0x40U] == std::byte{0x5A});
    assert(parent.bytes[0x40U] == std::byte{0});
    assert(reintegrated.receipt->spans.size() == 1U);
    assert(reintegrated.receipt->spans[0].offset == 0x20U);
    assert(reintegrated.receipt->spans[0].size == 0x40U);
    assert(reintegrated.receipt->spans[0].affected_aliases.size() == 2U);
    assert(reintegrated.receipt->spans[0].authored_aliases.size() == 1U);
    assert(reintegrated.receipt->parent_revision == 1U);

    const auto reparsed = registry.parse(
        std::span<const std::byte>{
            reintegrated.bytes.data(), reintegrated.bytes.size()},
        parent.resource.id.logical_path);
    assert(reparsed.ok());
    assert(reparsed.document.entries[0].offset == 0x20U);
    assert(reparsed.document.entries[1].offset == 0x20U);
    assert(reparsed.document.entries[0].size == 0x40U);
    assert(reparsed.document.entries[1].size == 0x40U);

    const auto authored1_same = author_pnst_child(
        expansion.children[1].payload, std::byte{0x5A});
    const std::vector<dmc3::AuthoredChildImage> same_aliases{
        authored0, authored1_same};
    const auto identical = dmc3::NestedRelativeSlotReintegrator::reintegrate(
        parent, expansion, same_aliases);
    assert(identical.ok());
    assert(identical.receipt->spans.size() == 1U);
    assert(identical.receipt->spans[0].affected_aliases.size() == 2U);
    assert(identical.receipt->spans[0].authored_aliases.size() == 2U);
    assert(identical.receipt->parent_revision == 1U);

    const auto authored1_different = author_pnst_child(
        expansion.children[1].payload, std::byte{0x66});
    const std::vector<dmc3::AuthoredChildImage> divergent{
        authored0, authored1_different};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, divergent).status ==
        dmc3::NestedReintegrationStatus::alias_conflict);

    auto stale_source = authored0;
    stale_source.source_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredChildImage> stale_source_set{stale_source};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, stale_source_set).status ==
        dmc3::NestedReintegrationStatus::child_source_mismatch);

    auto stale_output = authored0;
    stale_output.output_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredChildImage> stale_output_set{stale_output};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, stale_output_set).status ==
        dmc3::NestedReintegrationStatus::child_output_mismatch);

    auto grown = authored0;
    grown.bytes.push_back(std::byte{0});
    const std::vector<dmc3::AuthoredChildImage> grown_set{grown};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, grown_set).status ==
        dmc3::NestedReintegrationStatus::child_size_changed);

    const auto clean = clean_pnst_child(expansion.children[0].payload);
    const std::vector<dmc3::AuthoredChildImage> clean_set{clean};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, clean_set).status ==
        dmc3::NestedReintegrationStatus::no_changes);

    const std::vector<dmc3::AuthoredChildImage> duplicate_input{
        authored0, authored0};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, duplicate_input).status ==
        dmc3::NestedReintegrationStatus::duplicate_child_input);

    auto unknown = authored0;
    unknown.resource.logical_path += "-unknown";
    const std::vector<dmc3::AuthoredChildImage> unknown_set{unknown};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, unknown_set).status ==
        dmc3::NestedReintegrationStatus::child_not_found);

    auto stale_parent = parent;
    stale_parent.bytes[0x21U] = std::byte{0x7F};
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            stale_parent, expansion, one_alias).status ==
        dmc3::NestedReintegrationStatus::parent_span_mismatch);

    auto wrong_expansion = expansion;
    wrong_expansion.parser_format = "PACK";
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, wrong_expansion, one_alias).status ==
        dmc3::NestedReintegrationStatus::invalid_expansion);

    const std::vector<dmc3::AuthoredChildImage> none;
    assert(
        dmc3::NestedRelativeSlotReintegrator::reintegrate(
            parent, expansion, none).status ==
        dmc3::NestedReintegrationStatus::no_authored_children);

    return 0;
}
