#include "dmc_rengine/profiles/dmc3/texture_slot_expander.hpp"

#include <iomanip>
#include <limits>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string slot_component(std::uint32_t index) {
    std::ostringstream output;
    output << "slot-" << std::setfill('0') << std::setw(4) << index;
    return output.str();
}

[[nodiscard]] std::string synthetic_name(std::uint32_t index) {
    std::ostringstream output;
    output << "texture_" << std::setfill('0') << std::setw(4) << index
           << ".dds";
    return output.str();
}

[[nodiscard]] std::string child_chain(
    const gdspaces::ResourceId& parent,
    std::uint32_t index) {
    std::ostringstream output;
    if (!parent.container_chain.empty()) {
        output << parent.container_chain << '/';
    }
    output << "TEXTURE[" << index << ']';
    return output.str();
}

[[nodiscard]] std::optional<gdspaces::ByteProvenance> child_provenance(
    const gdspaces::ResourcePayload& parent,
    const TextureSlotEntry& texture) {
    if (parent.byte_provenance.has_value()) {
        if (!parent.byte_provenance->valid()) {
            return std::nullopt;
        }
        if (parent.byte_provenance->direct_byte_mapping()) {
            if (parent.byte_provenance->offset >
                std::numeric_limits<std::uint64_t>::max() - texture.dds_offset) {
                return std::nullopt;
            }
            return gdspaces::ByteProvenance{
                .kind = gdspaces::ByteOriginKind::direct_source_span,
                .authority_id = parent.byte_provenance->authority_id,
                .offset = parent.byte_provenance->offset + texture.dds_offset,
                .stored_size = texture.dds_size,
                .materialized_size = texture.dds_size,
                .transform = gdspaces::ByteTransform::none,
                .crc32 = std::nullopt,
            };
        }
    }

    return gdspaces::ByteProvenance{
        .kind = gdspaces::ByteOriginKind::materialized_parent_span,
        .authority_id = parent.resource.id.canonical(),
        .offset = texture.dds_offset,
        .stored_size = texture.dds_size,
        .materialized_size = texture.dds_size,
        .transform = gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };
}

void add_error(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourceId& resource,
    std::string code,
    std::string message) {
    expansion.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = gdspaces::DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

} // namespace

gdspaces::ContainerExpansion TextureSlotExpander::expand(
    const gdspaces::ResourcePayload& parent,
    TextureSlotFramingSafety safety) {
    gdspaces::ContainerExpansion expansion{
        .parent = parent.resource,
        .parser_format = "TEXTURE",
        .children = {},
        .diagnostics = {},
    };

    if (!parent.readable()) {
        add_error(
            expansion,
            parent.resource.id,
            "gdspaces.dmc3.texture.parent-unreadable",
            "Texture-slot expansion requires a readable parent payload.");
        return expansion;
    }

    const auto framing = TextureSlotFramingParser::parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        safety);
    if (!framing.ok()) {
        add_error(
            expansion,
            parent.resource.id,
            "gdspaces.dmc3.texture.invalid-framing",
            std::string{"Texture-slot framing is not accepted: "} +
                std::string{to_string(framing.status)} + ".");
        return expansion;
    }

    expansion.children.reserve(framing.document.textures.size());
    for (const auto& texture : framing.document.textures) {
        if (parent.resource.id.offset >
            std::numeric_limits<std::uint64_t>::max() - texture.dds_offset) {
            add_error(
                expansion,
                parent.resource.id,
                "gdspaces.dmc3.texture.child-offset-overflow",
                "A DDS child physical offset overflows the resource identity domain.");
            return expansion;
        }

        const auto begin = static_cast<std::size_t>(texture.dds_offset);
        const auto size = static_cast<std::size_t>(texture.dds_size);
        if (begin > parent.bytes.size() || size > parent.bytes.size() - begin) {
            add_error(
                expansion,
                parent.resource.id,
                "gdspaces.dmc3.texture.child-range-invalid",
                "A validated DDS child range no longer fits the current parent payload.");
            return expansion;
        }

        auto provenance = child_provenance(parent, texture);
        std::vector<gdspaces::Diagnostic> child_diagnostics;
        if (!provenance.has_value() || !provenance->valid()) {
            const gdspaces::Diagnostic diagnostic{
                .severity = gdspaces::DiagnosticSeverity::error,
                .code = "gdspaces.dmc3.texture.child-provenance-invalid",
                .message = "A DDS child could not be assigned safe byte provenance.",
                .resource = parent.resource.id,
            };
            expansion.diagnostics.push_back(diagnostic);
            child_diagnostics.push_back(diagnostic);
        }

        const auto name = synthetic_name(texture.texture_index);
        gdspaces::ResourceRef child_ref{
            .id = gdspaces::ResourceId{
                .source_id = parent.resource.id.source_id,
                .logical_path = parent.resource.id.logical_path +
                    "::TEXTURE/" + slot_component(texture.texture_index),
                .container_chain = child_chain(
                    parent.resource.id, texture.texture_index),
                .offset = parent.resource.id.offset + texture.dds_offset,
                .size = texture.dds_size,
            },
            .display_name = name,
            .format = "dds",
            .profile = parent.resource.profile,
            .synthetic_name = true,
            .container = false,
        };

        std::vector<std::byte> child_bytes(
            parent.bytes.begin() + static_cast<std::ptrdiff_t>(begin),
            parent.bytes.begin() + static_cast<std::ptrdiff_t>(begin + size));

        expansion.children.push_back(gdspaces::ContainerChild{
            .entry = formats::ContainerEntry{
                .slot_index = texture.texture_index,
                .offset = texture.dds_offset,
                .size = texture.dds_size,
                .logical_name = name,
                .populated = true,
                .synthetic_name = true,
            },
            .payload = gdspaces::ResourcePayload{
                .resource = std::move(child_ref),
                .bytes = std::move(child_bytes),
                .diagnostics = std::move(child_diagnostics),
                .byte_provenance = std::move(provenance),
            },
        });
    }

    return expansion;
}

} // namespace dmc::rengine::profiles::dmc3
