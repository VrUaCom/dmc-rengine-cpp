#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] std::uint32_t dds_payload_size(
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

[[nodiscard]] std::vector<std::byte> make_dds() {
    constexpr std::uint32_t width = 64U;
    constexpr std::uint32_t height = 64U;
    std::vector<std::byte> payload(
        dds_payload_size(width, height, true), std::byte{0x5A});
    const auto built = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dmc3::Dmc3DdsCompression::dxt5,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> make_texture_bundle() {
    const auto dds = make_dds();
    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(descriptor, 0x08U, 0x20000U | (doc.mip_map_count << 8U) | 0x88U);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * 4U);
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(descriptor, 0x44U, (doc.height << 16U) | doc.width);
    put_u32(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.width)));
    put_u32(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.height)));
    put_u32(descriptor, 0x60U, 4U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);

    const auto record_size = descriptor.size() + dds.size();
    const auto sectors = static_cast<std::uint32_t>(
        (record_size + dmc3::TextureSlotFramingParser::k_sector_size - 1U) /
        dmc3::TextureSlotFramingParser::k_sector_size);
    std::vector<std::byte> output(
        dmc3::TextureSlotFramingParser::k_bundle_header_size, std::byte{0});
    put_u32(output, 0U, 1U);
    put_u32(output, 4U, sectors);
    output.insert(output.end(), descriptor.begin(), descriptor.end());
    output.insert(output.end(), dds.begin(), dds.end());
    output.insert(
        output.end(),
        static_cast<std::size_t>(sectors) *
                dmc3::TextureSlotFramingParser::k_sector_size -
            record_size,
        std::byte{0});
    return output;
}

[[nodiscard]] gdspaces::ResourcePayload index_payload(
    std::string source_id,
    std::string logical_path,
    std::string_view text) {
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = std::move(source_id),
                .logical_path = std::move(logical_path),
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "manifest.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .semantic_evidence = {},
    };
}

[[nodiscard]] gdspaces::ContainerExpansion expansion_fixture() {
    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "container-source",
            .logical_path = "GData.afs/st001.pac",
            .container_chain = "nbz[12]",
            .offset = 0x1000U,
            .size = 0x2000U,
        },
        .display_name = "st001.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };

    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
        .external_index_evidence = std::nullopt,
    };

    expansion.children.push_back(gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = 0U,
            .offset = 0x100U,
            .size = 4U,
            .logical_name = "slot_0000.bin",
            .populated = true,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = gdspaces::ResourceId{
                    .source_id = parent.id.source_id,
                    .logical_path = "GData.afs/st001.pac::PAC/slot-0000",
                    .container_chain = "nbz[12]/PAC[0]",
                    .offset = 0x1100U,
                    .size = 4U,
                },
                .display_name = "slot_0000.bin",
                .format = "hits",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = {
                std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}},
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
            .semantic_evidence = {},
        },
    });

    return expansion;
}

[[nodiscard]] gdspaces::ContainerExpansion texture_expansion_fixture() {
    const auto texture = make_texture_bundle();
    const auto texture_size = static_cast<std::uint64_t>(texture.size());
    const gdspaces::ResourceRef parent{
        .id = gdspaces::ResourceId{
            .source_id = "container-source",
            .logical_path = "GData.afs/obj/em000.pac",
            .container_chain = "NBZ[41]",
            .offset = 0x4000U,
            .size = texture_size + 0x100U,
        },
        .display_name = "em000.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };

    gdspaces::ContainerExpansion expansion{
        .parent = parent,
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
        .external_index_evidence = std::nullopt,
    };
    expansion.children.push_back(gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = 0U,
            .offset = 0x100U,
            .size = texture_size,
            .logical_name = "slot_0000.bin",
            .populated = true,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = gdspaces::ResourceId{
                    .source_id = parent.id.source_id,
                    .logical_path = "GData.afs/obj/em000.pac::PAC/slot-0000",
                    .container_chain = "NBZ[41]/PAC[0]",
                    .offset = 0x4100U,
                    .size = texture_size,
                },
                .display_name = "slot_0000.bin",
                .format = "unknown",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = texture,
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
            .semantic_evidence = {},
        },
    });
    return expansion;
}

class StaticSource final : public gdspaces::ISource {
public:
    StaticSource(std::string source_id, std::vector<gdspaces::ResourcePayload> payloads)
        : source_id_(std::move(source_id)), payloads_(std::move(payloads)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "test-static";
    }

    [[nodiscard]] std::vector<gdspaces::ResourceRef> enumerate() const override {
        std::vector<gdspaces::ResourceRef> result;
        result.reserve(payloads_.size());
        for (const auto& payload : payloads_) {
            result.push_back(payload.resource);
        }
        return result;
    }

    [[nodiscard]] std::optional<gdspaces::ResourcePayload> read(
        const gdspaces::ResourceId& resource) const override {
        for (const auto& payload : payloads_) {
            if (payload.resource.id == resource) {
                return payload;
            }
        }
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<gdspaces::ResourcePayload> payloads_;
};

[[nodiscard]] bool has_code(
    const std::vector<gdspaces::Diagnostic>& diagnostics,
    std::string_view code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    {
        // Explicit retained-corpus evidence may intentionally come from a
        // different source than the materialized container. It outranks any
        // companion-source discovery path and remains the sealed authority.
        auto expansion = expansion_fixture();
        const auto original_id = expansion.children[0].payload.resource.id;
        const auto original_bytes = expansion.children[0].payload.bytes;
        const auto explicit_index = index_payload(
            "retained-corpus-source",
            "retained/st001.index",
            "st001_000.ukn\n");
        StaticSource irrelevant_companion{"container-source", {}};

        const auto result = dmc3::Dmc3NamingPipeline::apply(
            expansion, &explicit_index, &irrelevant_companion);
        assert(result.ok());
        assert(result.explicit_external_index_used);
        assert(!result.companion_index_discovered);
        assert(result.snapshot.has_value());
        assert(result.snapshot->external_index_evidence.has_value());
        assert(
            result.snapshot->external_index_evidence->manifest_resource ==
            explicit_index.resource.id);
        assert(result.snapshot->children.size() == 1U);
        assert(result.snapshot->children[0].physical_slot_index == 0U);
        assert(result.snapshot->children[0].extracted_ordinal.has_value());
        assert(*result.snapshot->children[0].extracted_ordinal == 0U);
        assert(
            result.snapshot->children[0].external_index_raw_label ==
            std::optional<std::string>{"st001_000.ukn"});
        assert(
            result.snapshot->children[0].external_index_name ==
            std::optional<std::string>{"st001_000.ukn"});
        assert(expansion.children[0].payload.resource.display_name == "st001_000.hits");
        assert(result.snapshot->children[0].canonical_display_name == "st001_000.hits");
        assert(!result.derived_display_names_applied);
        assert(expansion.children[0].payload.resource.id == original_id);
        assert(expansion.children[0].payload.bytes == original_bytes);
    }

    {
        // Without explicit evidence, one exact source-native companion may be
        // discovered and passed through the same reconciliation/snapshot path.
        auto expansion = expansion_fixture();
        StaticSource companion{
            "container-source",
            {index_payload(
                "container-source",
                "GData.afs/st001.index",
                "st001_000.ukn\n")}};

        const auto result = dmc3::Dmc3NamingPipeline::apply(
            expansion, nullptr, &companion);
        assert(result.ok());
        assert(!result.explicit_external_index_used);
        assert(result.companion_index_discovered);
        assert(
            result.companion_kind ==
            dmc3::CompanionIndexCandidateKind::sibling_manifest);
        assert(result.snapshot.has_value());
        assert(result.snapshot->external_index_evidence.has_value());
        assert(
            result.snapshot->external_index_evidence->manifest_resource.logical_path ==
            "GData.afs/st001.index");
        assert(!result.derived_display_names_applied);
    }

    {
        // No sidecar evidence is still a valid naming state: topology derives
        // the extracted ordinal while historical external labels remain absent.
        // The primary presentation name is no longer the parser's slot_XXXX
        // placeholder: it is an explicitly derived container/ordinal/semantic
        // display name and remains synthetic rather than historical authority.
        auto expansion = expansion_fixture();
        const auto original_id = expansion.children[0].payload.resource.id;
        const auto original_bytes = expansion.children[0].payload.bytes;
        const auto result = dmc3::Dmc3NamingPipeline::apply(expansion);
        assert(result.ok());
        assert(!result.explicit_external_index_used);
        assert(!result.companion_index_discovered);
        assert(result.derived_display_names_applied);
        assert(result.snapshot.has_value());
        assert(!result.snapshot->external_index_evidence.has_value());
        assert(result.snapshot->children[0].extracted_ordinal.has_value());
        assert(*result.snapshot->children[0].extracted_ordinal == 0U);
        assert(!result.snapshot->children[0].external_index_name.has_value());
        assert(result.snapshot->children[0].canonical_display_name == "st001_000.hits");
        assert(expansion.children[0].payload.resource.display_name == "slot_0000.bin");
        assert(expansion.children[0].payload.resource.synthetic_name);
        assert(expansion.children[0].payload.resource.id == original_id);
        assert(expansion.children[0].payload.bytes == original_bytes);
    }

    {
        // Critical no-index mobile case: a DMC3 texture bundle is identified
        // from its own structure, sealed as profile semantic evidence, and gets
        // a useful em000_000.ptx display without inventing `.index` evidence.
        auto expansion = texture_expansion_fixture();
        const auto original_id = expansion.children[0].payload.resource.id;
        const auto original_bytes = expansion.children[0].payload.bytes;
        const auto result = dmc3::Dmc3NamingPipeline::apply(expansion);
        assert(result.ok());
        assert(result.profile_semantics_applied);
        assert(result.derived_display_names_applied);
        assert(result.snapshot.has_value());
        assert(!result.snapshot->external_index_evidence.has_value());
        const auto& identity = result.snapshot->children[0];
        assert(identity.extracted_ordinal == 0U);
        assert(!identity.external_index_name.has_value());
        assert(identity.semantic_format == "texture-bundle");
        assert(identity.canonical_extension == "ptx");
        assert(identity.semantic_format_evidence.has_value());
        assert(
            identity.semantic_format_evidence->kind() ==
            gdspaces::ResourceSemanticEvidenceKind::profile_structural_format);
        assert(identity.canonical_display_name == "em000_000.ptx");
        assert(expansion.children[0].payload.resource.display_name == "slot_0000.bin");
        assert(expansion.children[0].payload.resource.synthetic_name);
        assert(expansion.children[0].payload.resource.id == original_id);
        assert(expansion.children[0].payload.bytes == original_bytes);
    }

    {
        // Ambiguous exact companions are fail-closed before reconciliation and
        // the caller's materialized expansion remains untouched.
        auto expansion = expansion_fixture();
        const auto before_name = expansion.children[0].payload.resource.display_name;
        const auto before_bytes = expansion.children[0].payload.bytes;
        StaticSource companion{
            "container-source",
            {
                index_payload(
                    "container-source",
                    "GData.afs/st001.index",
                    "st001_000.ukn\n"),
                index_payload(
                    "container-source",
                    "GData.afs/st001/st001.index",
                    "st001_000.ukn\n"),
            }};

        const auto result = dmc3::Dmc3NamingPipeline::apply(
            expansion, nullptr, &companion);
        assert(!result.ok());
        assert(has_code(
            result.diagnostics,
            "gdspaces.dmc3-companion-index.ambiguous"));
        assert(expansion.children[0].payload.resource.display_name == before_name);
        assert(expansion.children[0].payload.bytes == before_bytes);
        assert(expansion.children[0].payload.name_evidence.empty());
        assert(!expansion.external_index_evidence.has_value());
    }

    return 0;
}
