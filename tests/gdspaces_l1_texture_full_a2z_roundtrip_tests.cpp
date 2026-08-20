#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <bit>
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

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::string sha256_of(const std::vector<std::byte>& bytes) {
    return sha256_of(std::span<const std::byte>{bytes.data(), bytes.size()});
}

[[nodiscard]] std::uint32_t payload_size(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5) {
    std::uint32_t total = 0U;
    while (true) {
        total += std::max(1U, (width + 3U) / 4U) *
            std::max(1U, (height + 3U) / 4U) * (dxt5 ? 16U : 8U);
        if (width == 1U && height == 1U) {
            break;
        }
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    std::uint8_t seed) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    std::vector<std::byte> payload(payload_size(width, height, true), std::byte{0});
    for (std::size_t index = 0U; index < payload.size(); ++index) {
        payload[index] = static_cast<std::byte>(
            (static_cast<unsigned>(seed) + index * 43U) & 0xFFU);
    }
    const auto built = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dmc3::Dmc3DdsCompression::dxt5,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> make_texture_slot(
    const std::vector<std::byte>& dds) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;
    const auto secondary_width = doc.width / 2U;
    const auto secondary_height = doc.height / 2U;

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (doc.mip_map_count << 8U) | 0x88U);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * 4U);
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(descriptor, 0x3CU, 0U);
    put_u32(descriptor, 0x40U, 0U);
    put_u32(
        descriptor, 0x44U,
        (secondary_height << 16U) | secondary_width);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_width)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_height)));
    put_u32(descriptor, 0x60U, 4U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);
    descriptor.insert(descriptor.end(), dds.begin(), dds.end());

    const auto framing = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{descriptor.data(), descriptor.size()});
    assert(framing.ok());
    return descriptor;
}

[[nodiscard]] std::vector<std::byte> relative_container(
    bool pnst,
    const std::vector<std::byte>& child0,
    const std::vector<std::byte>& child1) {
    const std::size_t first_offset = 0x20U;
    const std::size_t second_offset = first_offset + child0.size();
    std::vector<std::byte> bytes(second_offset + child1.size(), std::byte{0});
    bytes[0U] = std::byte{pnst ? 'P' : 'P'};
    bytes[1U] = std::byte{pnst ? 'N' : 'A'};
    bytes[2U] = std::byte{pnst ? 'S' : 'C'};
    bytes[3U] = pnst ? std::byte{'T'} : std::byte{0};
    put_u32(bytes, 4U, 2U);
    put_u32(bytes, 8U, static_cast<std::uint32_t>(first_offset));
    put_u32(bytes, 12U, static_cast<std::uint32_t>(second_offset));
    for (std::size_t index = 0x10U; index < first_offset; ++index) {
        bytes[index] = static_cast<std::byte>(0xA0U + index - 0x10U);
    }
    std::copy(child0.begin(), child0.end(), bytes.begin() + static_cast<std::ptrdiff_t>(first_offset));
    std::copy(child1.begin(), child1.end(), bytes.begin() + static_cast<std::ptrdiff_t>(second_offset));
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
        .id = "synthetic-l1-texture-a2z",
        .role = "dmc3-retail-nbz",
        .sha256 = sha256_of(bytes),
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
}

[[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read_path(
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
    auto expansion = dmc::rengine::gdspaces::ContainerExpander::expand(payload, parsed);
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

    const auto source_dds = make_dds(256U, 256U, 0x21U);
    const auto source_texture_slot = make_texture_slot(source_dds);
    const std::vector<std::byte> inner_sibling{
        std::byte{'I'}, std::byte{'N'}, std::byte{'N'}, std::byte{'E'},
        std::byte{'R'}, std::byte{'-'}, std::byte{'O'}, std::byte{'K'}};
    const auto inner_pnst = relative_container(true, source_texture_slot, inner_sibling);
    const std::vector<std::byte> outer_sibling{
        std::byte{'O'}, std::byte{'U'}, std::byte{'T'}, std::byte{'E'},
        std::byte{'R'}, std::byte{'-'}, std::byte{'O'}, std::byte{'K'}};
    const auto outer_pac = relative_container(false, inner_pnst, outer_sibling);
    const std::vector<std::byte> untouched_member{
        std::byte{'N'}, std::byte{'B'}, std::byte{'Z'}, std::byte{'-'},
        std::byte{'S'}, std::byte{'I'}, std::byte{'B'}, std::byte{'L'},
        std::byte{'I'}, std::byte{'N'}, std::byte{'G'}};

    const std::vector<dmc3::NbzOverlayMember> overlay_members{
        dmc3::NbzOverlayMember{
            .logical_path = "GData.afs/texture-a2z.pac",
            .bytes = outer_pac,
        },
        dmc3::NbzOverlayMember{
            .logical_path = "SAVEDATA/texture-untouched.bin",
            .bytes = untouched_member,
        },
    };
    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U, 1U});
    assert(bootstrap.valid());
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(bootstrap, overlay_members);
    assert(overlay.ok());

    const auto source_path = write_temp(
        std::span<const std::byte>{overlay.bytes.data(), overlay.bytes.size()},
        "l1-texture-a2z-source");
    gdspaces::NbzZipSource source("l1-texture-a2z-source", source_path);
    assert(source.valid());

    const auto root = read_path(source, "GData.afs/texture-a2z.pac");
    assert(root.has_value());
    const auto registry = dmc3::make_container_parser_registry();
    const auto outer_expansion = expand_relative(*root, registry);
    assert(outer_expansion.children.size() == 2U);
    const auto inner_expansion = expand_relative(
        outer_expansion.children[0].payload, registry);
    assert(inner_expansion.children.size() == 2U);
    assert(inner_expansion.children[0].payload.bytes == source_texture_slot);

    const auto output_dds = make_dds(512U, 512U, 0x77U);
    const std::vector<dmc3::AuthoredPackedTextureDds> texture_edits{
        dmc3::AuthoredPackedTextureDds{
            .texture_index = 0U,
            .expected_source_sha256 = sha256_of(source_dds),
            .bytes = output_dds,
        },
    };
    const auto texture_reflow = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{
            inner_expansion.children[0].payload.bytes.data(),
            inner_expansion.children[0].payload.bytes.size()},
        texture_edits);
    assert(texture_reflow.ok());
    assert(texture_reflow.bytes.size() > source_texture_slot.size());

    const auto texture_child = authored_child(
        inner_expansion.children[0],
        texture_reflow.bytes,
        texture_reflow.receipt->writer_mode);
    const std::vector<dmc3::AuthoredChildImage> inner_edits{texture_child};
    const auto inner_reflow = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        outer_expansion.children[0].payload,
        inner_expansion,
        inner_edits);
    assert(inner_reflow.ok());
    assert(inner_reflow.bytes.size() > inner_pnst.size());

    const auto inner_child = authored_child(
        outer_expansion.children[0],
        inner_reflow.bytes,
        inner_reflow.receipt->writer_mode);
    const std::vector<dmc3::AuthoredChildImage> outer_edits{inner_child};
    const auto outer_reflow = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        *root,
        outer_expansion,
        outer_edits);
    assert(outer_reflow.ok());
    assert(outer_reflow.bytes.size() > outer_pac.size());

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
            return entry.logical_path == "GData.afs/texture-a2z.pac";
        });
    assert(root_entry != source.entries().end());
    const std::vector<gdspaces::NbzZipMemberReplacement> replacements{
        gdspaces::NbzZipMemberReplacement{
            .central_index = root_entry->central_index,
            .materialized_bytes = outer_reflow.bytes,
        },
    };

    const auto output_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-l1-texture-a2z-output.nbz";
    remove_if_present(output_path);
    const auto repacked = gdspaces::NbzZipRetailRepacker::write(
        source,
        *bound.snapshot,
        replacements,
        output_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 19U});
    assert(repacked.ok());
    assert(repacked.receipt->output_size > repacked.receipt->source_size);

    gdspaces::NbzZipSource reopened("l1-texture-a2z-reopened", output_path);
    assert(reopened.valid());
    const auto rebuilt_root = read_path(reopened, "GData.afs/texture-a2z.pac");
    assert(rebuilt_root.has_value());
    assert(rebuilt_root->bytes == outer_reflow.bytes);

    const auto reopened_outer = expand_relative(*rebuilt_root, registry);
    assert(reopened_outer.children.size() == 2U);
    assert(reopened_outer.children[1].payload.bytes == outer_sibling);
    const auto reopened_inner = expand_relative(
        reopened_outer.children[0].payload, registry);
    assert(reopened_inner.children.size() == 2U);
    assert(reopened_inner.children[1].payload.bytes == inner_sibling);

    const auto final_framing = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            reopened_inner.children[0].payload.bytes.data(),
            reopened_inner.children[0].payload.bytes.size()});
    assert(final_framing.ok());
    assert(final_framing.document.textures.size() == 1U);
    assert(final_framing.document.textures[0].width == 512U);
    assert(final_framing.document.textures[0].height == 512U);
    assert(final_framing.document.textures[0].secondary_width == 256U);
    const auto& final_entry = final_framing.document.textures[0];
    const auto final_dds = std::span<const std::byte>{
        reopened_inner.children[0].payload.bytes.data() +
            static_cast<std::ptrdiff_t>(final_entry.dds_offset),
        static_cast<std::size_t>(final_entry.dds_size)};
    assert(final_dds.size() == output_dds.size());
    assert(std::equal(final_dds.begin(), final_dds.end(), output_dds.begin(), output_dds.end()));

    const auto untouched_after = read_path(reopened, "SAVEDATA/texture-untouched.bin");
    assert(untouched_after.has_value());
    assert(untouched_after->bytes == untouched_member);
    assert(sha256_of(reopened_outer.children[1].payload.bytes) == sha256_of(outer_sibling));
    assert(sha256_of(reopened_inner.children[1].payload.bytes) == sha256_of(inner_sibling));
    assert(sha256_of(untouched_after->bytes) == sha256_of(untouched_member));
    assert(sha256_of(final_dds) == sha256_of(output_dds));

    std::error_code error;
    std::filesystem::remove(source_path, error);
    std::filesystem::remove(output_path, error);
    return 0;
}
