#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
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
    }

    {
        // No sidecar evidence is still a valid naming state: topology derives
        // the extracted ordinal while historical external labels remain absent.
        auto expansion = expansion_fixture();
        const auto result = dmc3::Dmc3NamingPipeline::apply(expansion);
        assert(result.ok());
        assert(!result.explicit_external_index_used);
        assert(!result.companion_index_discovered);
        assert(result.snapshot.has_value());
        assert(!result.snapshot->external_index_evidence.has_value());
        assert(result.snapshot->children[0].extracted_ordinal.has_value());
        assert(*result.snapshot->children[0].extracted_ordinal == 0U);
        assert(!result.snapshot->children[0].external_index_name.has_value());
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
