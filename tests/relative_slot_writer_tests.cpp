#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
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

std::vector<std::byte> make_pac() {
    std::vector<std::byte> bytes(0x50U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 5U); // table end 0x1c; protected prefix reaches 0x20.
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0U);
    put_u32(bytes, 16U, 0x30U);
    put_u32(bytes, 20U, 0x20U); // alias of slot 0.
    put_u32(bytes, 24U, 0x40U);
    return bytes;
}

std::vector<std::byte> make_pnst() {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0U);
    put_u32(bytes, 16U, 0x30U);
    return bytes;
}

dmc::rengine::gdspaces::ResourcePayload payload_for(
    std::vector<std::byte> bytes,
    std::string logical_path,
    std::string format) {
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "test-source",
                .logical_path = logical_path,
                .container_chain = "nbz[0]",
                .offset = 0U,
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

dmc::rengine::gdspaces::EditOperation edit(
    std::string id,
    std::uint64_t revision,
    std::uint64_t offset,
    std::vector<std::byte> expected,
    std::vector<std::byte> replacement) {
    return dmc::rengine::gdspaces::EditOperation{
        .id = std::move(id),
        .base_revision = revision,
        .offset = offset,
        .expected = std::move(expected),
        .replacement = std::move(replacement),
        .description = "relative-slot writer regression edit",
    };
}

} // namespace

int main() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    // Same-size payload edit is emitted byte-for-byte while topology, alias
    // identity and protected container prefix remain unchanged.
    auto pac_source = payload_for(make_pac(), "room/test.pac", "pac");
    gdspaces::WorkingCopy pac_working{pac_source};
    const auto pac_edit = pac_working.apply(edit(
        "pac-payload",
        0U,
        0x22U,
        {std::byte{0}},
        {std::byte{0x5A}}));
    assert(pac_edit.applied);
    const auto pac_rebuild = dmc3::RelativeSlotLayoutWriter::rebuild(
        pac_source, pac_working);
    assert(pac_rebuild.ok());
    assert(pac_rebuild.bytes.size() == pac_source.bytes.size());
    assert(pac_rebuild.bytes[0x22U] == std::byte{0x5A});
    assert(pac_rebuild.receipt->working_revision == 1U);
    assert(pac_rebuild.receipt->source_sha256 !=
           pac_rebuild.receipt->output_sha256);
    assert(pac_rebuild.receipt->source_topology ==
           pac_rebuild.receipt->output_topology);
    assert(pac_rebuild.receipt->source_topology.protected_prefix_size == 0x20U);
    assert(pac_rebuild.receipt->source_topology.entries[0].offset == 0x20U);
    assert(pac_rebuild.receipt->source_topology.entries[3].offset == 0x20U);
    assert(formats::PacParser::parse(pac_rebuild.bytes).ok());

    // WorkingCopy supports insertion/deletion generally, but this writer tier
    // explicitly rejects materialized size changes.
    gdspaces::WorkingCopy size_changed{pac_source};
    assert(size_changed.apply(edit(
        "grow",
        0U,
        0x22U,
        {std::byte{0}},
        {std::byte{1}, std::byte{2}})).applied);
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(pac_source, size_changed).status ==
        dmc3::RelativeSlotRebuildStatus::size_changed);

    // A structural table edit that still parses is not a layout-preserving
    // rebuild merely because total byte count stayed constant.
    gdspaces::WorkingCopy topology_changed{pac_source};
    assert(topology_changed.apply(edit(
        "offset-table",
        0U,
        8U,
        {std::byte{0x20}},
        {std::byte{0x24}})).applied);
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            pac_source, topology_changed).status ==
        dmc3::RelativeSlotRebuildStatus::topology_changed);

    // Non-structural bytes in the pre-payload region remain protected because
    // the original packed image, not an inferred packer, owns that layout.
    gdspaces::WorkingCopy prefix_changed{pac_source};
    assert(prefix_changed.apply(edit(
        "pre-payload",
        0U,
        0x1DU,
        {std::byte{0}},
        {std::byte{0x7F}})).applied);
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            pac_source, prefix_changed).status ==
        dmc3::RelativeSlotRebuildStatus::protected_prefix_changed);

    // Changing magic fails the canonical output parser before any writer can
    // claim a valid rebuild.
    gdspaces::WorkingCopy invalid_output{pac_source};
    assert(invalid_output.apply(edit(
        "magic",
        0U,
        0U,
        {std::byte{'P'}},
        {std::byte{'X'}})).applied);
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            pac_source, invalid_output).status ==
        dmc3::RelativeSlotRebuildStatus::output_parse_failed);

    // Immutable-source SHA binds a WorkingCopy to the exact source bytes even
    // when ResourceId is unchanged.
    auto changed_source = pac_source;
    gdspaces::WorkingCopy hash_bound{changed_source};
    changed_source.bytes[0x22U] = std::byte{0x33};
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            changed_source, hash_bound).status ==
        dmc3::RelativeSlotRebuildStatus::source_hash_mismatch);

    // Canonical ResourceId is also part of the writer trust boundary.
    auto other_source = pac_source;
    other_source.resource.id.logical_path = "room/other.pac";
    other_source.resource.display_name = "other.pac";
    gdspaces::WorkingCopy other_working{other_source};
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            pac_source, other_working).status ==
        dmc3::RelativeSlotRebuildStatus::resource_mismatch);

    // The same bounded writer applies to PNST because the shared physical slot
    // envelope is canonical while semantic schemas remain separate.
    auto pnst_source = payload_for(make_pnst(), "room/test_pnst.pac", "pnst");
    gdspaces::WorkingCopy pnst_working{pnst_source};
    assert(pnst_working.apply(edit(
        "pnst-payload",
        0U,
        0x35U,
        {std::byte{0}},
        {std::byte{0x44}})).applied);
    const auto pnst_rebuild = dmc3::RelativeSlotLayoutWriter::rebuild(
        pnst_source, pnst_working);
    assert(pnst_rebuild.ok());
    assert(pnst_rebuild.receipt->source_topology.format == "PNST");
    assert(formats::PnstParser::parse(pnst_rebuild.bytes).ok());

    // With no populated slots there is no evidenced payload domain, so the
    // whole image is protected in layout-preserving mode.
    std::vector<std::byte> empty_pac(16U, std::byte{0});
    empty_pac[0] = std::byte{'P'};
    empty_pac[1] = std::byte{'A'};
    empty_pac[2] = std::byte{'C'};
    empty_pac[3] = std::byte{0};
    put_u32(empty_pac, 4U, 0U);
    auto empty_source = payload_for(std::move(empty_pac), "room/empty.pac", "pac");
    gdspaces::WorkingCopy empty_clean{empty_source};
    assert(dmc3::RelativeSlotLayoutWriter::rebuild(
        empty_source, empty_clean).ok());
    gdspaces::WorkingCopy empty_changed{empty_source};
    assert(empty_changed.apply(edit(
        "empty-tail",
        0U,
        12U,
        {std::byte{0}},
        {std::byte{1}})).applied);
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            empty_source, empty_changed).status ==
        dmc3::RelativeSlotRebuildStatus::protected_prefix_changed);

    // Magic-first writer support is intentionally limited to PAC/PNST.
    std::vector<std::byte> dds(16U, std::byte{0});
    dds[0] = std::byte{'D'};
    dds[1] = std::byte{'D'};
    dds[2] = std::byte{'S'};
    dds[3] = std::byte{' '};
    auto unsupported = payload_for(std::move(dds), "texture/test.dds", "dds");
    gdspaces::WorkingCopy unsupported_working{unsupported};
    assert(
        dmc3::RelativeSlotLayoutWriter::rebuild(
            unsupported, unsupported_working).status ==
        dmc3::RelativeSlotRebuildStatus::unsupported_format);

    return 0;
}
