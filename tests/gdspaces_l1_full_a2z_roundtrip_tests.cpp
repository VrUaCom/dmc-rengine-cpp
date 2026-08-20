#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
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

[[nodiscard]] std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const auto value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] std::string sha256_of(
    std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::string sha256_of(
    const std::vector<std::byte>& bytes) {
    return sha256_of(std::span<const std::byte>{bytes.data(), bytes.size()});
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

    for (std::size_t index = 0x14U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0x90U + index - 0x14U);
    }
    const auto leaf0 = ascii("LEAF-ORIGINAL-01");
    assert(leaf0.size() == 0x10U);
    std::copy(leaf0.begin(), leaf0.end(), bytes.begin() + 0x20U);

    bytes[0x30U] = std::byte{'D'};
    bytes[0x31U] = std::byte{'D'};
    bytes[0x32U] = std::byte{'S'};
    bytes[0x33U] = std::byte{' '};
    for (std::size_t index = 0x34U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0x33U ^ index);
    }
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

    for (std::size_t index = 0x14U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0xA0U + index - 0x14U);
    }
    std::copy(nested.begin(), nested.end(), bytes.begin() + 0x20U);

    bytes[0x70U] = std::byte{'D'};
    bytes[0x71U] = std::byte{'D'};
    bytes[0x72U] = std::byte{'S'};
    bytes[0x73U] = std::byte{' '};
    for (std::size_t index = 0x74U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0x66U ^ index);
    }
    return bytes;
}

[[nodiscard]] std::filesystem::path write_temp(
    std::span<const std::byte> bytes,
    std::string_view stem) {
    const auto path = std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{stem} + ".nbz");
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

void remove_if_present(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::remove(path, error);
    std::filesystem::remove(
        std::filesystem::path{path.string() + ".dmc-rengine-repack.tmp"},
        error);
}

[[nodiscard]] dmc::rengine::evidence::ArtifactIdentity artifact_for(
    std::span<const std::byte> bytes) {
    return dmc::rengine::evidence::ArtifactIdentity{
        .id = "synthetic-l1-a2z",
        .role = "dmc3-retail-nbz",
        .sha256 = sha256_of(bytes),
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
}

[[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload>
read_path(
    const dmc::rengine::gdspaces::NbzZipSource& source,
    std::string_view logical_path) {
    const auto refs = source.enumerate();
    const auto found = std::find_if(
        refs.begin(), refs.end(),
        [logical_path](const auto& resource) {
            return resource.id.logical_path == logical_path;
        });
    if (found == refs.end()) {
        return std::nullopt;
    }
    return source.read(found->id);
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion expand_relative(
    const dmc::rengine::gdspaces::ResourcePayload& payload,
    const dmc::rengine::formats::ContainerParserRegistry& registry) {
    const auto parsed = registry.parse(
        std::span<const std::byte>{payload.bytes.data(), payload.bytes.size()},
        payload.resource.id.logical_path);
    assert(parsed.ok());
    auto expansion = dmc::rengine::gdspaces::ContainerExpander::expand(
        payload, parsed);
    assert(expansion.usable());
    return expansion;
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredChildImage authored_child(
    const dmc::rengine::gdspaces::ContainerChild& child,
    std::vector<std::byte> output,
    std::string writer_mode) {
    return dmc::rengine::profiles::dmc3::AuthoredChildImage{
        .resource = child.payload.resource.id,
        .source_sha256 = sha256_of(child.payload.bytes),
        .output_sha256 = sha256_of(output),
        .revision = 1U,
        .writer_mode = std::move(writer_mode),
        .bytes = std::move(output),
    };
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto original_outer = outer_pac();
    const auto untouched_nbz_member = ascii("UNCHANGED-NBZ-SIBLING");
    const std::vector<dmc3::NbzOverlayMember> overlay_members{
        dmc3::NbzOverlayMember{
            .logical_path = "GData.afs/full-a2z.pac",
            .bytes = original_outer,
        },
        dmc3::NbzOverlayMember{
            .logical_path = "SAVEDATA/untouched.bin",
            .bytes = untouched_nbz_member,
        },
    };
    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U, 1U});
    assert(bootstrap.valid());
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, overlay_members);
    assert(overlay.ok());

    const auto source_path = write_temp(
        std::span<const std::byte>{overlay.bytes.data(), overlay.bytes.size()},
        "l1-full-a2z-source");
    gdspaces::NbzZipSource source("l1-full-a2z-source", source_path);
    assert(source.valid());
    assert(source.entries().size() == 2U);

    const auto root_payload = read_path(source, "GData.afs/full-a2z.pac");
    assert(root_payload.has_value());
    assert(root_payload->bytes == original_outer);

    const auto registry = dmc3::make_container_parser_registry();
    const auto outer_expansion = expand_relative(*root_payload, registry);
    assert(outer_expansion.parser_format == "PAC");
    assert(outer_expansion.children.size() == 3U);
    assert(outer_expansion.children[0].payload.resource.container);
    assert(outer_expansion.children[0].entry.offset == 0x20U);
    assert(outer_expansion.children[0].entry.size == 0x50U);
    assert(outer_expansion.children[1].entry.offset == 0x70U);
    assert(outer_expansion.children[1].entry.size == 0x20U);
    assert(!outer_expansion.children[2].entry.populated);

    const auto inner_expansion = expand_relative(
        outer_expansion.children[0].payload, registry);
    assert(inner_expansion.parser_format == "PNST");
    assert(inner_expansion.children.size() == 3U);
    assert(inner_expansion.children[0].entry.offset == 0x20U);
    assert(inner_expansion.children[0].entry.size == 0x10U);
    assert(inner_expansion.children[1].entry.offset == 0x30U);
    assert(inner_expansion.children[1].entry.size == 0x20U);
    assert(!inner_expansion.children[2].entry.populated);

    const auto original_inner_sibling = inner_expansion.children[1].payload.bytes;
    const auto original_outer_sibling = outer_expansion.children[1].payload.bytes;
    auto changed_leaf = ascii(
        "LEAF-EXPANDED-BY-GDSPACES-L1-A2Z-REBUILD!");
    assert(changed_leaf.size() > inner_expansion.children[0].payload.bytes.size());

    const auto leaf_authored = authored_child(
        inner_expansion.children[0],
        changed_leaf,
        "synthetic-exact-leaf-edit");
    const std::vector<dmc3::AuthoredChildImage> inner_edits{leaf_authored};
    const auto inner_reflow = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        outer_expansion.children[0].payload,
        inner_expansion,
        inner_edits);
    assert(inner_reflow.ok());
    assert(inner_reflow.bytes.size() > outer_expansion.children[0].payload.bytes.size());
    assert(inner_reflow.receipt->spans.size() == 2U);
    assert(inner_reflow.receipt->spans[0].changed);
    assert(!inner_reflow.receipt->spans[1].changed);
    assert(u32(inner_reflow.bytes, 8U) == 0x20U);
    assert(u32(inner_reflow.bytes, 16U) == 0U);
    assert(u32(inner_reflow.bytes, 12U) > 0x30U);

    const auto nested_authored = authored_child(
        outer_expansion.children[0],
        inner_reflow.bytes,
        inner_reflow.receipt->writer_mode);
    const std::vector<dmc3::AuthoredChildImage> outer_edits{nested_authored};
    const auto outer_reflow = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        *root_payload,
        outer_expansion,
        outer_edits);
    assert(outer_reflow.ok());
    assert(outer_reflow.bytes.size() > original_outer.size());
    assert(outer_reflow.receipt->spans.size() == 2U);
    assert(outer_reflow.receipt->spans[0].changed);
    assert(!outer_reflow.receipt->spans[1].changed);
    assert(u32(outer_reflow.bytes, 8U) == 0x20U);
    assert(u32(outer_reflow.bytes, 16U) == 0U);
    assert(u32(outer_reflow.bytes, 12U) > 0x70U);

    const auto expected_artifact = artifact_for(
        std::span<const std::byte>{overlay.bytes.data(), overlay.bytes.size()});
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source,
        expected_artifact,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 17U});
    assert(bound.ok());

    const auto root_entry = std::find_if(
        source.entries().begin(), source.entries().end(),
        [](const auto& entry) {
            return entry.logical_path == "GData.afs/full-a2z.pac";
        });
    assert(root_entry != source.entries().end());
    const std::vector<gdspaces::NbzZipMemberReplacement> replacements{
        gdspaces::NbzZipMemberReplacement{
            .central_index = root_entry->central_index,
            .materialized_bytes = outer_reflow.bytes,
        },
    };

    const auto output_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-l1-full-a2z-output.nbz";
    remove_if_present(output_path);
    const auto repacked = gdspaces::NbzZipRetailRepacker::write(
        source,
        *bound.snapshot,
        replacements,
        output_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 19U});
    assert(repacked.ok());
    assert(repacked.receipt->output_size > repacked.receipt->source_size);
    assert(repacked.receipt->entries[root_entry->central_index].changed);

    gdspaces::NbzZipSource reopened("l1-full-a2z-reopened", output_path);
    assert(reopened.valid());
    const auto rebuilt_root = read_path(reopened, "GData.afs/full-a2z.pac");
    assert(rebuilt_root.has_value());
    assert(rebuilt_root->bytes == outer_reflow.bytes);

    const auto reopened_outer = expand_relative(*rebuilt_root, registry);
    assert(reopened_outer.children.size() == 3U);
    assert(reopened_outer.children[0].payload.bytes == inner_reflow.bytes);
    assert(reopened_outer.children[1].payload.bytes == original_outer_sibling);
    assert(!reopened_outer.children[2].entry.populated);

    const auto reopened_inner = expand_relative(
        reopened_outer.children[0].payload, registry);
    assert(reopened_inner.children.size() == 3U);
    assert(reopened_inner.children[0].payload.bytes == leaf_authored.bytes);
    assert(reopened_inner.children[1].payload.bytes == original_inner_sibling);
    assert(!reopened_inner.children[2].entry.populated);

    const auto untouched_after = read_path(reopened, "SAVEDATA/untouched.bin");
    assert(untouched_after.has_value());
    assert(untouched_after->bytes == untouched_nbz_member);

    // Final receipt: canonical recursive materialization reproduces the exact
    // requested leaf edit while both nested and NBZ-level siblings remain exact.
    assert(sha256_of(reopened_inner.children[0].payload.bytes) ==
           leaf_authored.output_sha256);
    assert(sha256_of(reopened_inner.children[1].payload.bytes) ==
           sha256_of(original_inner_sibling));
    assert(sha256_of(reopened_outer.children[1].payload.bytes) ==
           sha256_of(original_outer_sibling));
    assert(sha256_of(untouched_after->bytes) == sha256_of(untouched_nbz_member));

    std::error_code remove_error;
    std::filesystem::remove(source_path, remove_error);
    std::filesystem::remove(output_path, remove_error);
    return 0;
}
