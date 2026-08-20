#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_synth_relative_slot_writer.hpp"

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

std::vector<std::byte> source_image(std::string_view magic) {
    assert(magic.size() == 4U);
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[index] = static_cast<std::byte>(magic[index]);
    }
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0U);
    put_u32(bytes, 16U, 0x40U);
    bytes[0x20U] = std::byte{'A'};
    bytes[0x40U] = std::byte{'B'};
    return bytes;
}

std::vector<std::byte> alias_source() {
    auto bytes = source_image(std::string_view{"PAC\0", 4U});
    put_u32(bytes, 12U, 0x20U);
    return bytes;
}

std::vector<std::byte> zero_slot_source() {
    std::vector<std::byte> bytes(16U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 0U);
    return bytes;
}

dmc::rengine::gdspaces::ResourcePayload payload(
    std::vector<std::byte> bytes,
    std::string logical_path,
    std::string format) {
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "runtime-synth-source",
                .logical_path = logical_path,
                .container_chain = "nbz[4]",
                .offset = 0x2000U,
                .size = size,
            },
            .display_name = std::move(logical_path),
            .format = std::move(format),
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

std::string sha256_of(const std::vector<std::byte>& bytes) {
    return dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{bytes.data(), bytes.size()}).hex();
}

std::vector<std::byte> repeated(std::size_t count, std::byte value) {
    return std::vector<std::byte>(count, value);
}

dmc::rengine::profiles::dmc3::ExactChildImage exact_child(
    std::uint32_t slot,
    dmc::rengine::profiles::dmc3::ExactChildAuthorityKind authority_kind,
    dmc::rengine::profiles::dmc3::ExactChildExtentKind extent_kind,
    std::string authority,
    std::vector<std::byte> bytes,
    std::string writer_mode = {}) {
    const auto sha = sha256_of(bytes);
    return dmc::rengine::profiles::dmc3::ExactChildImage{
        .slot_index = slot,
        .authority_kind = authority_kind,
        .extent_kind = extent_kind,
        .authority_id = std::move(authority),
        .writer_mode = std::move(writer_mode),
        .sha256 = sha,
        .bytes = std::move(bytes),
    };
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace formats = dmc::rengine::formats;

    auto pac_source = payload(
        source_image(std::string_view{"PAC\0", 4U}),
        "GData.afs/runtime-synth.pac",
        "pac");
    std::vector<dmc3::ExactChildImage> children{
        exact_child(
            0U,
            dmc3::ExactChildAuthorityKind::external_exact_resource,
            dmc3::ExactChildExtentKind::intrinsic_resource,
            "external:slot0",
            {std::byte{0x11}, std::byte{0x22}, std::byte{0x33}}),
        exact_child(
            2U,
            dmc3::ExactChildAuthorityKind::loose_resource,
            dmc3::ExactChildExtentKind::intrinsic_resource,
            "loose:GData.afs/child2.bin",
            repeated(70U, std::byte{0x5A})),
    };

    const auto rebuilt = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
        pac_source, children);
    assert(rebuilt.ok());
    assert(rebuilt.bytes.size() == 0x100U);
    assert(rebuilt.receipt->writer_mode == "runtime-synth-relative-slot");
    assert(rebuilt.receipt->children.size() == 2U);
    assert(rebuilt.receipt->children[0].slot_index == 0U);
    assert(
        rebuilt.receipt->children[0].extent_kind ==
        dmc3::ExactChildExtentKind::intrinsic_resource);
    assert(rebuilt.receipt->children[0].emitted_offset == 0x40U);
    assert(rebuilt.receipt->children[0].intrinsic_size == 3U);
    assert(rebuilt.receipt->children[1].slot_index == 2U);
    assert(rebuilt.receipt->children[1].emitted_offset == 0x80U);
    assert(rebuilt.receipt->children[1].intrinsic_size == 70U);

    const auto parsed = formats::PacParser::parse(rebuilt.bytes);
    assert(parsed.ok());
    assert(parsed.document->declared_slot_count == 3U);
    assert(parsed.document->entries[0].offset == 0x40U);
    assert(parsed.document->entries[0].populated);
    assert(!parsed.document->entries[1].populated);
    assert(parsed.document->entries[1].offset == 0U);
    assert(parsed.document->entries[2].offset == 0x80U);
    assert(parsed.document->entries[2].populated);
    assert(rebuilt.bytes[0x40U] == std::byte{0x11});
    assert(rebuilt.bytes[0x41U] == std::byte{0x22});
    assert(rebuilt.bytes[0x42U] == std::byte{0x33});
    for (std::size_t index = 0x43U; index < 0x80U; ++index) {
        assert(rebuilt.bytes[index] == std::byte{0});
    }
    for (std::size_t index = 0x80U; index < 0x80U + 70U; ++index) {
        assert(rebuilt.bytes[index] == std::byte{0x5A});
    }
    for (std::size_t index = 0x80U + 70U; index < 0x100U; ++index) {
        assert(rebuilt.bytes[index] == std::byte{0});
    }

    std::reverse(children.begin(), children.end());
    const auto reordered = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
        pac_source, children);
    assert(reordered.ok());
    assert(reordered.bytes == rebuilt.bytes);
    assert(reordered.receipt->output_sha256 == rebuilt.receipt->output_sha256);

    const std::vector<dmc3::ExactChildImage> missing{children.front()};
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, missing).status ==
        dmc3::RuntimeSynthStatus::missing_child);

    auto for_empty = children;
    for_empty.push_back(exact_child(
        1U,
        dmc3::ExactChildAuthorityKind::external_exact_resource,
        dmc3::ExactChildExtentKind::intrinsic_resource,
        "external:empty-slot",
        {std::byte{1}}));
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, for_empty).status ==
        dmc3::RuntimeSynthStatus::child_for_empty_slot);

    auto duplicate = children;
    duplicate.push_back(children.front());
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, duplicate).status ==
        dmc3::RuntimeSynthStatus::duplicate_child_input);

    auto unknown = children;
    unknown.push_back(exact_child(
        99U,
        dmc3::ExactChildAuthorityKind::external_exact_resource,
        dmc3::ExactChildExtentKind::intrinsic_resource,
        "external:unknown-slot",
        {std::byte{1}}));
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, unknown).status ==
        dmc3::RuntimeSynthStatus::unknown_child_slot);

    auto forbidden = children;
    forbidden[0].authority_kind =
        dmc3::ExactChildAuthorityKind::container_extracted_span;
    forbidden[0].extent_kind =
        dmc3::ExactChildExtentKind::container_inferred_span;
    forbidden[0].authority_id = "parent-span:0x20+0x20";
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, forbidden).status ==
        dmc3::RuntimeSynthStatus::forbidden_extracted_span);

    auto source_span_preserved = children;
    source_span_preserved[0].extent_kind =
        dmc3::ExactChildExtentKind::source_span_preserved;
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, source_span_preserved).status ==
        dmc3::RuntimeSynthStatus::unproven_intrinsic_extent);

    auto inferred_span = children;
    inferred_span[0].extent_kind =
        dmc3::ExactChildExtentKind::container_inferred_span;
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, inferred_span).status ==
        dmc3::RuntimeSynthStatus::unproven_intrinsic_extent);

    auto forged_writer_receipt = children;
    forged_writer_receipt[0].authority_kind =
        dmc3::ExactChildAuthorityKind::format_writer_receipt;
    forged_writer_receipt[0].extent_kind =
        dmc3::ExactChildExtentKind::writer_defined_complete_image;
    forged_writer_receipt[0].authority_id = "forged:runtime-synth-receipt";
    forged_writer_receipt[0].writer_mode = "runtime-synth-relative-slot";
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, forged_writer_receipt).status ==
        dmc3::RuntimeSynthStatus::unproven_intrinsic_extent);

    auto bad_hash = children;
    bad_hash.front().sha256.assign(64U, '0');
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source, bad_hash).status ==
        dmc3::RuntimeSynthStatus::child_hash_mismatch);

    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            pac_source,
            children,
            dmc3::RuntimeSynthSafetyLimits{.max_output_bytes = 0xFFU}).status ==
        dmc3::RuntimeSynthStatus::output_too_large);

    auto aliased = payload(
        alias_source(), "GData.afs/aliased.pac", "pac");
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            aliased, children).status ==
        dmc3::RuntimeSynthStatus::alias_topology_unsupported);

    auto pnst_source = payload(
        source_image("PNST"), "GData.afs/runtime-synth-pnst.pac", "pnst");
    const auto pnst = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
        pnst_source, children);
    assert(pnst.ok());
    assert(pnst.receipt->source_topology.format == "PNST");
    assert(pnst.receipt->output_topology.format == "PNST");
    assert(formats::PnstParser::parse(pnst.bytes).ok());

    auto zero = payload(
        zero_slot_source(), "GData.afs/zero.pac", "pac");
    const std::vector<dmc3::ExactChildImage> none;
    const auto zero_rebuilt = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
        zero, none);
    assert(zero_rebuilt.ok());
    assert(zero_rebuilt.bytes.size() == 0x40U);
    const auto zero_parsed = formats::PacParser::parse(zero_rebuilt.bytes);
    assert(zero_parsed.ok());
    assert(zero_parsed.document->declared_slot_count == 0U);

    std::vector<std::byte> dds(32U, std::byte{0});
    dds[0] = std::byte{'D'};
    dds[1] = std::byte{'D'};
    dds[2] = std::byte{'S'};
    dds[3] = std::byte{' '};
    auto unsupported = payload(std::move(dds), "texture.dds", "dds");
    assert(
        dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
            unsupported, none).status ==
        dmc3::RuntimeSynthStatus::unsupported_format);

    return 0;
}
