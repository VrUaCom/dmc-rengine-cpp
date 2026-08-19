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

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan no_volumes() {
    constexpr std::array<std::uint32_t, 0> present{};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeSourceBindings three_bindings() {
    using dmc::rengine::profiles::dmc3::ArchiveSourceBinding;
    return dmc::rengine::profiles::dmc3::RuntimeSourceBindings{
        .physical_source_id = "physical",
        .archives = {
            ArchiveSourceBinding{2U, "archive-2"},
            ArchiveSourceBinding{0U, "archive-0"},
            ArchiveSourceBinding{1U, "archive-1"},
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::SourceRegistry;
    using dmc::rengine::profiles::dmc3::ArchiveSourceBinding;
    using dmc::rengine::profiles::dmc3::RuntimeLookupEvidenceClass;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::RuntimeResourceResolver;
    using dmc::rengine::profiles::dmc3::RuntimeSourceBindings;

    const auto bootstrap = three_volumes();
    assert(bootstrap.valid());

    // Prefix order is outer authority: lower-precedence volume under prefix 0
    // beats a higher-precedence volume under prefix 1.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {
            make_resource("archive-2", "GData.afs/st001.pac", "nbz[20]")});
        mount(registry, "archive-1", {});
        mount(registry, "archive-0", {
            make_resource("archive-0", "GDataX360.afs/st001.pac", "nbz[1]")});

        auto bindings = three_bindings();
        assert(bindings.valid_for(bootstrap));
        const auto report = RuntimeResourceResolver::resolve(
            "scr\\st001.pac", bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-0");
        assert(report.probes.size() == 3U);
        assert(report.probes[0].archive_volume_index == 2U);
        assert(report.probes[1].archive_volume_index == 1U);
        assert(report.probes[2].archive_volume_index == 0U);
        for (const auto& probe : report.probes) {
            assert(probe.lookup_evidence == RuntimeLookupEvidenceClass::recovered_archive_index);
        }
    }

    // Highest volume wins within one candidate; archive normalization is lower-case.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {
            make_resource("archive-2", "GDataX360.afs/ST001.PAC", "nbz[200]")});
        mount(registry, "archive-1", {});
        mount(registry, "archive-0", {
            make_resource("archive-0", "gdatax360.afs/st001.pac", "nbz[2]")});
        const auto report = RuntimeResourceResolver::resolve(
            "room/St001.PAC", bootstrap, three_bindings(), registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "archive-2");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].provider_key == "gdatax360.afs\\st001.pac");
    }

    // Ambiguity in the highest-precedence archive stops without probing lower volumes.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {
            make_resource("archive-2", "GDataX360.afs/ST001.PAC", "nbz[10]"),
            make_resource("archive-2", "gdatax360.afs\\st001.pac", "nbz[11]")});
        mount(registry, "archive-1", {});
        mount(registry, "archive-0", {});
        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, three_bindings(), registry);
        assert(report.status == RuntimeResolutionStatus::ambiguous);
        assert(report.ambiguous_matches.size() == 2U);
        assert(report.probes.size() == 1U);
    }

    // Complete miss preserves all six archive candidates x three volumes, then
    // all six physical candidates: 24 probes in exact provider phase order.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {});
        mount(registry, "archive-1", {});
        mount(registry, "archive-0", {});
        const auto report = RuntimeResourceResolver::resolve(
            "missing.pac", bootstrap, three_bindings(), registry);
        assert(report.status == RuntimeResolutionStatus::not_found);
        assert(report.probes.size() == 24U);
        for (std::size_t index = 0U; index < 18U; ++index) {
            assert(report.probes[index].archive_volume_index.has_value());
            assert(report.probes[index].lookup_evidence ==
                RuntimeLookupEvidenceClass::recovered_archive_index);
        }
        for (std::size_t index = 18U; index < 24U; ++index) {
            assert(!report.probes[index].archive_volume_index.has_value());
            assert(report.probes[index].lookup_evidence ==
                RuntimeLookupEvidenceClass::product_physical_index);
        }
    }

    // Zero archive volumes is valid: the runtime-equivalent plan reaches the
    // physical pass directly. Physical matching is explicitly product-classified
    // until the exact type-0 filename/open comparison is recovered.
    {
        const auto empty_bootstrap = no_volumes();
        assert(empty_bootstrap.valid());
        SourceRegistry registry;
        mount(registry, "physical", {
            make_resource("physical", "GDataX360.afs/st001.pac", "disk")});
        RuntimeSourceBindings bindings{
            .physical_source_id = "physical",
            .archives = {},
        };
        assert(bindings.valid_for(empty_bootstrap));
        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", empty_bootstrap, bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.source_id == "physical");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].lookup_evidence ==
            RuntimeLookupEvidenceClass::product_physical_index);
    }

    // Missing mounted archive source is configuration failure before probes.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {});
        mount(registry, "archive-0", {});
        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, three_bindings(), registry);
        assert(report.status == RuntimeResolutionStatus::invalid_source_configuration);
        assert(report.probes.empty());
    }

    // Duplicate volume and duplicate source bindings are rejected structurally.
    {
        RuntimeSourceBindings duplicate_volume{
            .physical_source_id = "physical",
            .archives = {
                ArchiveSourceBinding{0U, "archive-0"},
                ArchiveSourceBinding{0U, "archive-1"},
                ArchiveSourceBinding{2U, "archive-2"},
            },
        };
        assert(!duplicate_volume.valid_for(bootstrap));

        RuntimeSourceBindings duplicate_source{
            .physical_source_id = "physical",
            .archives = {
                ArchiveSourceBinding{0U, "archive-x"},
                ArchiveSourceBinding{1U, "archive-x"},
                ArchiveSourceBinding{2U, "archive-2"},
            },
        };
        assert(!duplicate_source.valid_for(bootstrap));
    }

    // The index is now derived from the exact mounted source enumeration. A
    // source that emits a ResourceRef belonging to another source cannot be
    // silently accepted as a same-id stale index configuration.
    {
        SourceRegistry registry;
        mount(registry, "physical", {});
        mount(registry, "archive-2", {
            make_resource("not-archive-2", "GDataX360.afs/st001.pac", "bad")});
        mount(registry, "archive-1", {});
        mount(registry, "archive-0", {});
        const auto report = RuntimeResourceResolver::resolve(
            "st001.pac", bootstrap, three_bindings(), registry);
        assert(report.status == RuntimeResolutionStatus::invalid_source_configuration);
        assert(report.probes.empty());
    }

    // Embedded NUL request fails before any provider/source probe.
    {
        SourceRegistry registry;
        const std::string embedded_nul{"room/st001\0evil.pac", 19U};
        RuntimeSourceBindings bindings{};
        const auto report = RuntimeResourceResolver::resolve(
            embedded_nul, bootstrap, bindings, registry);
        assert(report.status == RuntimeResolutionStatus::invalid_request);
        assert(report.probes.empty());
    }

    return 0;
}
