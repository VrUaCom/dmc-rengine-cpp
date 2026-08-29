#include "dmc_rengine/profiles/dmc3/companion_index_locator.hpp"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] gdspaces::ResourcePayload make_payload(
    std::string_view source_id,
    std::string_view logical_path,
    std::string_view display_name) {
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = std::string{source_id},
                .logical_path = std::string{logical_path},
                .container_chain = {},
                .offset = 0U,
                .size = 4U,
            },
            .display_name = std::string{display_name},
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = {
            std::byte{'t'}, std::byte{'e'}, std::byte{'s'}, std::byte{'t'}},
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .semantic_evidence = {},
    };
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

[[nodiscard]] gdspaces::ResourceId container_id(std::string_view path) {
    return gdspaces::ResourceId{
        .source_id = "retail-source",
        .logical_path = std::string{path},
        .container_chain = {},
        .offset = 0U,
        .size = 0x2000U,
    };
}

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
        const auto candidates = dmc3::CompanionIndexLocator::candidates_for(
            container_id("GData.afs/scr/st001.pac"));
        assert(candidates.size() == 2U);
        assert(candidates[0].kind == dmc3::CompanionIndexCandidateKind::sibling_manifest);
        assert(candidates[0].logical_path == "GData.afs/scr/st001.index");
        assert(candidates[1].kind ==
               dmc3::CompanionIndexCandidateKind::expanded_directory_manifest);
        assert(candidates[1].logical_path == "GData.afs/scr/st001/st001.index");
    }

    {
        StaticSource source{
            "retail-source",
            {make_payload(
                "retail-source",
                "GData.afs/scr/st001.index",
                "totally-unrelated-display-name.index")}};
        const auto result = dmc3::CompanionIndexLocator::discover(
            source, container_id("GData.afs/scr/st001.pac"));
        assert(result.ok());
        assert(result.matched_kind ==
               dmc3::CompanionIndexCandidateKind::sibling_manifest);
        assert(result.payload.has_value());
        assert(result.payload->resource.id.logical_path ==
               "GData.afs/scr/st001.index");
    }

    {
        StaticSource source{
            "retail-source",
            {make_payload(
                "retail-source",
                "GData.afs/scr/st001/st001.index",
                "st001.index")}};
        const auto result = dmc3::CompanionIndexLocator::discover(
            source, container_id("GData.afs/scr/st001.pac"));
        assert(result.ok());
        assert(result.matched_kind ==
               dmc3::CompanionIndexCandidateKind::expanded_directory_manifest);
    }

    {
        StaticSource source{
            "retail-source",
            {
                make_payload(
                    "retail-source",
                    "GData.afs/scr/st001.index",
                    "st001.index"),
                make_payload(
                    "retail-source",
                    "GData.afs/scr/st001/st001.index",
                    "st001.index"),
            }};
        const auto result = dmc3::CompanionIndexLocator::discover(
            source, container_id("GData.afs/scr/st001.pac"));
        assert(!result.ok());
        assert(!result.payload.has_value());
        assert(has_code(
            result.diagnostics,
            "gdspaces.dmc3-companion-index.ambiguous"));
    }

    {
        StaticSource source{
            "retail-source",
            {make_payload(
                "retail-source",
                "GData.afs/scr/not-st001.index",
                "st001.index")}};
        const auto result = dmc3::CompanionIndexLocator::discover(
            source, container_id("GData.afs/scr/st001.pac"));
        assert(!result.ok());
        assert(!result.payload.has_value());
        assert(result.diagnostics.empty());
    }

    {
        const auto candidates = dmc3::CompanionIndexLocator::candidates_for(
            container_id("GData.afs/scr/st001.pac::PAC/slot-0001"));
        assert(candidates.empty());
    }

    {
        StaticSource source{"another-source", {}};
        const auto result = dmc3::CompanionIndexLocator::discover(
            source, container_id("GData.afs/scr/st001.pac"));
        assert(!result.ok());
        assert(has_code(
            result.diagnostics,
            "gdspaces.dmc3-companion-index.invalid-container"));
    }

    return 0;
}
