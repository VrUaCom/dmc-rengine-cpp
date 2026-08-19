#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
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
    std::vector<std::byte> bytes(0x30U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    bytes[0x10U] = std::byte{'D'};
    bytes[0x11U] = std::byte{'D'};
    bytes[0x12U] = std::byte{'S'};
    bytes[0x13U] = std::byte{' '};
    return bytes;
}

dmc::rengine::gdspaces::ResourcePayload pac_payload() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = make_pac();
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-volume-1",
                .logical_path = "GData.afs/test.pac",
                .container_chain = "nbz[17]",
                .offset = 1234U,
                .size = size,
            },
            .display_name = "test.pac",
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

dmc::rengine::gdspaces::EditOperation one_byte_edit(
    std::uint64_t revision,
    std::uint64_t offset,
    std::byte expected,
    std::byte replacement) {
    return dmc::rengine::gdspaces::EditOperation{
        .id = "overlay-root-edit",
        .base_revision = revision,
        .offset = offset,
        .expected = {expected},
        .replacement = {replacement},
        .description = "Edit root PAC before STORE-overlay authoring.",
    };
}

std::filesystem::path write_temp_nbz(
    std::span<const std::byte> bytes,
    std::string_view stem) {
    auto path = std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{stem} + ".nbz");
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

const dmc::rengine::gdspaces::ResourceRef* find_path(
    const std::vector<dmc::rengine::gdspaces::ResourceRef>& refs,
    std::string_view path) {
    const auto found = std::find_if(
        refs.begin(), refs.end(),
        [path](const auto& ref) { return ref.id.logical_path == path; });
    return found == refs.end() ? nullptr : &*found;
}

std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

} // namespace

int main() {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    // First complete synthetic root-resource L1 authoring vertical:
    // ResourcePayload -> WorkingCopy -> layout-preserving PAC rebuild ->
    // STORE-only next-volume NBZ -> NbzZipSource reopen -> PAC reparse.
    auto original = pac_payload();
    gdspaces::WorkingCopy working{original};
    const auto edited = working.apply(one_byte_edit(
        0U, 0x20U, std::byte{0}, std::byte{0x5A}));
    assert(edited.applied);

    const auto rebuilt = dmc3::RelativeSlotLayoutWriter::rebuild(
        original, working);
    assert(rebuilt.ok());
    assert(rebuilt.bytes[0x20U] == std::byte{0x5A});

    const std::vector<std::uint32_t> present_volumes{0U, 1U};
    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_volumes);
    assert(bootstrap.valid());
    assert(bootstrap.first_missing_index == 2U);
    assert(bootstrap.present_after_first_gap.empty());

    std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = original.resource.id.logical_path,
            .bytes = rebuilt.bytes,
        },
        dmc3::NbzOverlayMember{
            .logical_path = "SAVEDATA/overlay-note.bin",
            .bytes = ascii("OVERLAY"),
        },
    };

    const auto overlay = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, members);
    assert(overlay.ok());
    assert(overlay.receipt->volume_index == 2U);
    assert(overlay.receipt->filename == "DMC3-2.nbz");
    assert(overlay.receipt->members.size() == 2U);
    assert(overlay.receipt->archive_size == overlay.bytes.size());

    // Input order is product authoring input, not archive-layout authority.
    // Canonical normalized-key ordering makes generated output deterministic.
    std::reverse(members.begin(), members.end());
    const auto overlay_reordered = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, members);
    assert(overlay_reordered.ok());
    assert(overlay_reordered.bytes == overlay.bytes);
    assert(
        overlay_reordered.receipt->archive_sha256 ==
        overlay.receipt->archive_sha256);

    const auto path = write_temp_nbz(
        std::span<const std::byte>{overlay.bytes.data(), overlay.bytes.size()},
        "store-overlay");
    gdspaces::NbzZipSource reopened("generated-overlay-volume-2", path);
    assert(reopened.valid());
    assert(reopened.index_receipt().has_value());
    assert(reopened.index_receipt()->central_offset_matches);
    assert(reopened.index_receipt()->entry_count_matches);
    assert(reopened.index_receipt()->walked_entry_count == 2U);

    const auto refs = reopened.enumerate();
    assert(refs.size() == 2U);
    const auto* root_ref = find_path(refs, original.resource.id.logical_path);
    assert(root_ref != nullptr);
    assert(root_ref->id.source_id == "generated-overlay-volume-2");
    assert(root_ref->id.source_id != original.resource.id.source_id);
    assert(root_ref->id.logical_path == original.resource.id.logical_path);

    const auto reopened_root = reopened.read(root_ref->id);
    assert(reopened_root.has_value());
    assert(reopened_root->readable());
    assert(reopened_root->bytes == rebuilt.bytes);
    assert(reopened_root->bytes != original.bytes);
    assert(reopened_root->resource.format == "pac");
    assert(reopened_root->byte_provenance.has_value());
    assert(
        reopened_root->byte_provenance->kind ==
        gdspaces::ByteOriginKind::direct_source_span);
    assert(
        reopened_root->byte_provenance->transform ==
        gdspaces::ByteTransform::zip_stored);
    assert(
        reopened_root->byte_provenance->stored_size ==
        reopened_root->byte_provenance->materialized_size);
    assert(reopened_root->byte_provenance->crc32.has_value());

    const auto reparsed = formats::PacParser::parse(reopened_root->bytes);
    assert(reparsed.ok());
    assert(
        reparsed.document->declared_slot_count ==
        rebuilt.receipt->output_topology.declared_slot_count);
    assert(
        reparsed.document->container_size ==
        rebuilt.receipt->output_topology.container_size);
    assert(reparsed.document->entries.size() == 1U);
    assert(
        reparsed.document->entries[0].offset ==
        rebuilt.receipt->output_topology.entries[0].offset);
    assert(
        reparsed.document->entries[0].size ==
        rebuilt.receipt->output_topology.entries[0].size);

    std::error_code remove_error;
    std::filesystem::remove(path, remove_error);

    // Filling a runtime gap must not silently activate already-present later
    // volumes. Product writer therefore refuses this by default.
    const std::vector<std::uint32_t> gapped_volumes{0U, 2U};
    const auto gapped = dmc3::VolumeBootstrapPolicy::plan(gapped_volumes);
    assert(gapped.valid());
    assert(gapped.first_missing_index == 1U);
    assert(gapped.present_after_first_gap.size() == 1U);
    assert(
        dmc3::NbzStoreOverlayWriter::build(gapped, members).status ==
        dmc3::NbzOverlayWriteStatus::post_gap_volume_present);

    // Writer-owned archives never introduce comparator-equal normalized keys;
    // original CRT duplicate-winner behavior is intentionally not invented.
    const std::vector<dmc3::NbzOverlayMember> collision{
        dmc3::NbzOverlayMember{
            .logical_path = "Folder/File.PAC",
            .bytes = ascii("A"),
        },
        dmc3::NbzOverlayMember{
            .logical_path = "folder\\file.pac",
            .bytes = ascii("B"),
        },
    };
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, collision).status ==
        dmc3::NbzOverlayWriteStatus::normalized_key_collision);

    const std::vector<dmc3::NbzOverlayMember> duplicate{
        dmc3::NbzOverlayMember{
            .logical_path = "same.pac",
            .bytes = ascii("A"),
        },
        dmc3::NbzOverlayMember{
            .logical_path = "same.pac",
            .bytes = ascii("B"),
        },
    };
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, duplicate).status ==
        dmc3::NbzOverlayWriteStatus::duplicate_member_path);

    const std::string embedded_nul{"bad\0name.pac", 12U};
    const std::vector<dmc3::NbzOverlayMember> invalid_name{
        dmc3::NbzOverlayMember{
            .logical_path = embedded_nul,
            .bytes = ascii("A"),
        },
    };
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, invalid_name).status ==
        dmc3::NbzOverlayWriteStatus::invalid_member_path);

    const std::vector<dmc3::NbzOverlayMember> directory_name{
        dmc3::NbzOverlayMember{
            .logical_path = "folder/",
            .bytes = ascii("A"),
        },
    };
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, directory_name).status ==
        dmc3::NbzOverlayWriteStatus::invalid_member_path);

    const std::vector<dmc3::NbzOverlayMember> empty_member{
        dmc3::NbzOverlayMember{
            .logical_path = "empty.pac",
            .bytes = {},
        },
    };
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, empty_member).status ==
        dmc3::NbzOverlayWriteStatus::empty_member);

    const std::vector<dmc3::NbzOverlayMember> none;
    assert(
        dmc3::NbzStoreOverlayWriter::build(bootstrap, none).status ==
        dmc3::NbzOverlayWriteStatus::no_members);

    // No retail numbered archives is valid: generated DMC3-0.nbz becomes the
    // first archive mount. This is distinct from overwriting a retail file.
    const std::vector<std::uint32_t> no_volumes;
    const auto empty_bootstrap = dmc3::VolumeBootstrapPolicy::plan(no_volumes);
    const std::vector<dmc3::NbzOverlayMember> first_member{
        dmc3::NbzOverlayMember{
            .logical_path = "first.pac",
            .bytes = rebuilt.bytes,
        },
    };
    const auto first_overlay = dmc3::NbzStoreOverlayWriter::build(
        empty_bootstrap, first_member);
    assert(first_overlay.ok());
    assert(first_overlay.receipt->volume_index == 0U);
    assert(first_overlay.receipt->filename == "DMC3-0.nbz");

    return 0;
}
