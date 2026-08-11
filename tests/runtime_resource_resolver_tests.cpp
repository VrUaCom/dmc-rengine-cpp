#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

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
    StaticSource(
        std::string source_id,
        std::vector<dmc::rengine::gdspaces::ResourceRef> resources)
        : source_id_(std::move(source_id)),
          resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "runtime-resolver-test";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        return resources_;
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId&) const override {
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef make_resource(
    std::string source_id,
    std::string logical_path,
    std::string identity_suffix) {
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

void mount(
    dmc::rengine::gdspaces::SourceRegistry& registry,
    std::string source_id,
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources = {}) {
    assert(registry.mount(std::make_unique<StaticSource>(
        std::move(source_id), std::move(resources))));
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan three_volumes() {
    constexpr std::array<std::uint32_t, 3> present{0U, 1U, 2U};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeSourceBindings bindings3() {
    return dmc::rengine::profiles::dmc3::RuntimeSourceBindings{
        .physical_source_id = "physical",
        .archives = {
            {2U, "archive-2"},
            {0U, "archive-0"},
            {1U, "archive-1"},
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::SourceRegistry;
    using dmc::rengine::profiles::dmc3::ResourceProviderClass;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::RuntimeResourceResolver;
    using dmc::rengine::profiles::dmc3::RuntimeSourceBindings;
    using dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy;

    const auto bootstrap = three_volumes();
    const auto bindings = bindings3();
    assert(bootstrap.valid());
    assert(bindings.valid_for(bootstrap));

    // Prefix order is outside mount traversal. A lower-precedence volume hit
    // under prefix 0 beats a higher-precedence volume hit under prefix 1.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-2", {
            make_resource(
                "archive-2", "GData.afs/st001.pac", "nbz[20]"),
        });
        mount(registry, "archive-1");
        mount(registry, "archive-0", {
            make_resource(
                "archive-0", "GDataX360.afs/st001.pac", "nbz[1]"),
        });

        const auto report = RuntimeResourceResolver::resolve(
            "scr\\st001.pac", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.status == RuntimeResolutionStatus::resolved);
        assert(report.resolved->id.source_id == "archive-0");
        assert(report.probes.size() == 3U);
        assert(report.probes[0].lookup_attempt_index == 0U);
        assert(report.probes[0].archive_volume_index == 2U);
        assert(report.probes[1].archive_volume_index == 1U);
        assert(report.probes[2].archive_volume_index == 0U);
        assert(report.probes[2].lookup.unique());
    }

    // Within one candidate attempt, highest contiguous archive volume wins.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-2", {
            make_resource(
                "archive-2", "GDataX360.afs/ST001.PAC", "nbz[200]"),
        });
        mount(registry, "archive-1");
        mount(registry, "archive-0", {
            make_resource(
                "archive-0", "gdatax360.afs/st001.pac", "nbz[2]"),
        });

        const auto report = RuntimeResourceResolver::resolve(
            "room/St001.PAC", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-2");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].archive_volume_index == 2U);
        assert(report.probes[0].provider == ResourceProviderClass::archive);
        assert(report.probes[0].provider_key ==
            "gdatax360.afs\\st001.pac");
    }

    // All six archive candidates are exhausted before the first physical
    // candidate. An archive empty-prefix hit therefore beats a physical
    // prefix-0 hit.
    {
        SourceRegistry registry;
        mount(registry, "physical", {
            make_resource(
                "physical", "GDataX360.afs/st001.pac", "direct"),
        });
        mount(registry, "archive-2");
        mount(registry, "archive-1");
        mount(registry, "archive-0", {
            make_resource("archive-0", "st001.pac", "nbz[99]"),
        });

        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-0");
        assert(report.probes.size() == 18U);
        assert(report.probes.back().lookup_attempt_index == 5U);
        assert(report.probes.back().archive_volume_index == 0U);
        for (const auto& probe : report.probes) {
            assert(probe.provider == ResourceProviderClass::archive);
        }
    }

    // Physical provider is reached only after all archive attempts fail.
    {
        SourceRegistry registry;
        mount(registry, "physical", {
            make_resource(
                "physical", "GDataX360.afs/St001.PAC", "direct"),
        });
        mount(registry, "archive-2");
        mount(registry, "archive-1");
        mount(registry, "archive-0");

        const auto report = RuntimeResourceResolver::resolve(
            "ROOM\\St001.PAC", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "physical");
        assert(report.probes.size() == 19U);
        assert(report.probes.back().lookup_attempt_index == 6U);
        assert(report.probes.back().provider == ResourceProviderClass::physical);
        assert(!report.probes.back().archive_volume_index.has_value());
        assert(report.probes.back().provider_key ==
            "GDataX360.afs\\St001.PAC");
    }

    // Duplicate normalized keys inside the first runtime mount are exposed as
    // ambiguity. The resolver does not silently continue to a lower volume.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-2", {
            make_resource(
                "archive-2", "GDataX360.afs/ST001.PAC", "nbz[10]"),
            make_resource(
                "archive-2", "gdatax360.afs\\st001.pac", "nbz[11]"),
        });
        mount(registry, "archive-1", {
            make_resource(
                "archive-1", "GDataX360.afs/st001.pac", "nbz[12]"),
        });
        mount(registry, "archive-0");

        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, bindings, registry);
        assert(!report.ok());
        assert(report.status == RuntimeResolutionStatus::ambiguous);
        assert(report.ambiguous_matches.size() == 2U);
        assert(report.probes.size() == 1U);
        assert(report.probes[0].lookup.ambiguous());
        assert(report.probes[0].archive_volume_index == 2U);
    }

    // Complete miss records every source-level probe: 6 archive candidates x
    // 3 archive mounts, then 6 physical candidates.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-2");
        mount(registry, "archive-1");
        mount(registry, "archive-0");

        const auto report = RuntimeResourceResolver::resolve(
            "missing.pac", bootstrap, bindings, registry);
        assert(!report.ok());
        assert(report.status == RuntimeResolutionStatus::not_found);
        assert(report.probes.size() == 24U);
        assert(report.probes.front().lookup_attempt_index == 0U);
        assert(report.probes.back().lookup_attempt_index == 11U);
    }

    // Missing a source that belongs to the runtime mount set is configuration
    // failure, not a normal resource miss.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-1");
        mount(registry, "archive-0");

        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, bindings, registry);
        assert(report.status ==
            RuntimeResolutionStatus::invalid_source_configuration);
        assert(report.probes.empty());
    }

    // Binding set must match the contiguous runtime bootstrap exactly.
    {
        auto invalid = bindings;
        invalid.archives[0].volume_index = 7U;
        assert(!invalid.valid_for(bootstrap));

        invalid = bindings;
        invalid.archives[0].source_id = "archive-1";
        assert(!invalid.valid_for(bootstrap));

        invalid = bindings;
        invalid.physical_source_id.clear();
        assert(!invalid.valid_for(bootstrap));
    }

    // Invalid request never reaches source lookup.
    {
        SourceRegistry registry;
        mount(registry, "physical");
        mount(registry, "archive-2");
        mount(registry, "archive-1");
        mount(registry, "archive-0");

        const auto report = RuntimeResourceResolver::resolve(
            "room/", bootstrap, bindings, registry);
        assert(report.status == RuntimeResolutionStatus::invalid_request);
        assert(report.probes.empty());
    }

    // Zero archive volumes still reproduces the archive pass (with no source
    // nodes) before physical lookup.
    {
        const auto no_archives = VolumeBootstrapPolicy::plan(
            std::span<const std::uint32_t>{});
        const RuntimeSourceBindings physical_only{
            .physical_source_id = "physical",
            .archives = {},
        };
        assert(no_archives.valid());
        assert(physical_only.valid_for(no_archives));

        SourceRegistry registry;
        mount(registry, "physical", {
            make_resource(
                "physical", "GDataX360.afs/st001.pac", "direct"),
        });
        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", no_archives, physical_only, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "physical");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].lookup_attempt_index == 6U);
    }

    return 0;
}
