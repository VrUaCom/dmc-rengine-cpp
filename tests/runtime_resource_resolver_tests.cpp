#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class StaticSource final : public dmc::rengine::gdspaces::ISource {
public:
    StaticSource(std::string source_id,
                 std::vector<dmc::rengine::gdspaces::ResourceRef> resources)
        : source_id_(std::move(source_id)), resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override { return source_id_; }
    [[nodiscard]] std::string_view kind() const noexcept override { return "runtime-resolver-test"; }
    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef> enumerate() const override { return resources_; }
    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId&) const override { return std::nullopt; }

private:
    std::string source_id_;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef make_resource(
    std::string source_id, std::string logical_path, std::string identity_suffix) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = std::move(identity_suffix),
            .offset = 0U,
            .size = 16U,
        },
        .display_name = "resource.bin",
        .format = "unknown",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

void mount(dmc::rengine::gdspaces::SourceRegistry& registry,
           std::string source_id,
           std::vector<dmc::rengine::gdspaces::ResourceRef> resources) {
    assert(registry.mount(std::make_unique<StaticSource>(
        std::move(source_id), std::move(resources))));
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan three_volumes() {
    constexpr std::array<std::uint32_t, 3> present{0U, 1U, 2U};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ResourceKeyIndex;
    using dmc::rengine::gdspaces::SourceRegistry;
    using dmc::rengine::profiles::dmc3::ArchiveSourceBinding;
    using dmc::rengine::profiles::dmc3::ResourcePathPolicy;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::RuntimeResourceResolver;
    using dmc::rengine::profiles::dmc3::RuntimeSourceBindings;
    using dmc::rengine::profiles::dmc3::SourceKeyIndexBinding;

    const auto bootstrap = three_volumes();
    assert(bootstrap.valid());

    // Prefix order is outer authority: lower-precedence volume under prefix 0
    // beats a higher-precedence volume under prefix 1.
    {
        SourceRegistry registry;
        std::vector<dmc::rengine::gdspaces::ResourceRef> physical_resources;
        std::vector<dmc::rengine::gdspaces::ResourceRef> a2_resources{
            make_resource("archive-2", "GData.afs/st001.pac", "nbz[20]")};
        std::vector<dmc::rengine::gdspaces::ResourceRef> a1_resources;
        std::vector<dmc::rengine::gdspaces::ResourceRef> a0_resources{
            make_resource("archive-0", "GDataX360.afs/st001.pac", "nbz[1]")};

        auto physical_index = ResourceKeyIndex::build(
            "physical", ResourcePathPolicy::physical_flags, physical_resources);
        auto a2_index = ResourceKeyIndex::build(
            "archive-2", ResourcePathPolicy::archive_flags, a2_resources);
        auto a1_index = ResourceKeyIndex::build(
            "archive-1", ResourcePathPolicy::archive_flags, a1_resources);
        auto a0_index = ResourceKeyIndex::build(
            "archive-0", ResourcePathPolicy::archive_flags, a0_resources);

        mount(registry, "physical", physical_resources);
        mount(registry, "archive-2", a2_resources);
        mount(registry, "archive-1", a1_resources);
        mount(registry, "archive-0", a0_resources);

        RuntimeSourceBindings bindings{
            .physical = SourceKeyIndexBinding{"physical", &physical_index},
            .archives = {
                ArchiveSourceBinding{2U, {"archive-2", &a2_index}},
                ArchiveSourceBinding{0U, {"archive-0", &a0_index}},
                ArchiveSourceBinding{1U, {"archive-1", &a1_index}},
            },
        };
        assert(bindings.valid_for(bootstrap));

        const auto report = RuntimeResourceResolver::resolve(
            "scr\\st001.pac", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-0");
        assert(report.probes.size() == 3U);
        assert(report.probes[0].archive_volume_index == 2U);
        assert(report.probes[1].archive_volume_index == 1U);
        assert(report.probes[2].archive_volume_index == 0U);
    }

    // Highest volume wins within one candidate; archive normalization is lower-case.
    {
        SourceRegistry registry;
        std::vector<dmc::rengine::gdspaces::ResourceRef> physical_resources;
        std::vector<dmc::rengine::gdspaces::ResourceRef> a2_resources{
            make_resource("archive-2", "GDataX360.afs/ST001.PAC", "nbz[200]")};
        std::vector<dmc::rengine::gdspaces::ResourceRef> a1_resources;
        std::vector<dmc::rengine::gdspaces::ResourceRef> a0_resources{
            make_resource("archive-0", "gdatax360.afs/st001.pac", "nbz[2]")};
        auto physical_index = ResourceKeyIndex::build("physical", ResourcePathPolicy::physical_flags, physical_resources);
        auto a2_index = ResourceKeyIndex::build("archive-2", ResourcePathPolicy::archive_flags, a2_resources);
        auto a1_index = ResourceKeyIndex::build("archive-1", ResourcePathPolicy::archive_flags, a1_resources);
        auto a0_index = ResourceKeyIndex::build("archive-0", ResourcePathPolicy::archive_flags, a0_resources);
        mount(registry, "physical", physical_resources);
        mount(registry, "archive-2", a2_resources);
        mount(registry, "archive-1", a1_resources);
        mount(registry, "archive-0", a0_resources);
        RuntimeSourceBindings bindings{
            .physical = {"physical", &physical_index},
            .archives = {{2U, {"archive-2", &a2_index}}, {1U, {"archive-1", &a1_index}}, {0U, {"archive-0", &a0_index}}},
        };
        const auto report = RuntimeResourceResolver::resolve("room/St001.PAC", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-2");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].provider_key == "gdatax360.afs\\st001.pac");
    }

    // Ambiguity in the highest-precedence source stops without probing lower volumes.
    {
        SourceRegistry registry;
        std::vector<dmc::rengine::gdspaces::ResourceRef> empty;
        std::vector<dmc::rengine::gdspaces::ResourceRef> a2_resources{
            make_resource("archive-2", "GDataX360.afs/ST001.PAC", "nbz[10]"),
            make_resource("archive-2", "gdatax360.afs\\st001.pac", "nbz[11]")};
        auto physical_index = ResourceKeyIndex::build("physical", ResourcePathPolicy::physical_flags, empty);
        auto a2_index = ResourceKeyIndex::build("archive-2", ResourcePathPolicy::archive_flags, a2_resources);
        auto a1_index = ResourceKeyIndex::build("archive-1", ResourcePathPolicy::archive_flags, empty);
        auto a0_index = ResourceKeyIndex::build("archive-0", ResourcePathPolicy::archive_flags, empty);
        mount(registry, "physical", empty);
        mount(registry, "archive-2", a2_resources);
        mount(registry, "archive-1", empty);
        mount(registry, "archive-0", empty);
        RuntimeSourceBindings bindings{
            .physical = {"physical", &physical_index},
            .archives = {{2U, {"archive-2", &a2_index}}, {1U, {"archive-1", &a1_index}}, {0U, {"archive-0", &a0_index}}},
        };
        const auto report = RuntimeResourceResolver::resolve("st001.pac", bootstrap, bindings, registry);
        assert(report.status == RuntimeResolutionStatus::ambiguous);
        assert(report.ambiguous_matches.size() == 2U);
        assert(report.probes.size() == 1U);
    }

    // Wrong normalization profile is configuration failure, never a miss.
    {
        SourceRegistry registry;
        std::vector<dmc::rengine::gdspaces::ResourceRef> empty;
        auto bad_physical_index = ResourceKeyIndex::build("physical", ResourcePathPolicy::archive_flags, empty);
        auto a2_index = ResourceKeyIndex::build("archive-2", ResourcePathPolicy::archive_flags, empty);
        auto a1_index = ResourceKeyIndex::build("archive-1", ResourcePathPolicy::archive_flags, empty);
        auto a0_index = ResourceKeyIndex::build("archive-0", ResourcePathPolicy::archive_flags, empty);
        mount(registry, "physical", empty);
        mount(registry, "archive-2", empty);
        mount(registry, "archive-1", empty);
        mount(registry, "archive-0", empty);
        RuntimeSourceBindings bindings{
            .physical = {"physical", &bad_physical_index},
            .archives = {{2U, {"archive-2", &a2_index}}, {1U, {"archive-1", &a1_index}}, {0U, {"archive-0", &a0_index}}},
        };
        const auto report = RuntimeResourceResolver::resolve("st001.pac", bootstrap, bindings, registry);
        assert(report.status == RuntimeResolutionStatus::invalid_source_configuration);
        assert(report.probes.empty());
    }

    // Embedded NUL request fails before any provider/source probe.
    {
        SourceRegistry registry;
        const std::string embedded_nul{"room/st001\0evil.pac", 19U};
        RuntimeSourceBindings bindings{};
        const auto report = RuntimeResourceResolver::resolve(embedded_nul, bootstrap, bindings, registry);
        assert(report.status == RuntimeResolutionStatus::invalid_request);
        assert(report.probes.empty());
    }

    return 0;
}
