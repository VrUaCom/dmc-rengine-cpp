#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
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
                 std::vector<dmc::rengine::gdspaces::ResourceRef> resources = {})
        : source_id_(std::move(source_id)), resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override { return source_id_; }
    [[nodiscard]] std::string_view kind() const noexcept override { return "physical-receipt-static"; }
    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef> enumerate() const override {
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

[[nodiscard]] std::filesystem::path unique_test_root() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
        ("dmc-rengine-l2-physical-receipt-" + std::to_string(stamp));
}

void write_file(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream << "L2-physical-receipt";
    assert(stream.good());
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan no_volumes() {
    constexpr std::array<std::uint32_t, 0> present{};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

[[nodiscard]] dmc::rengine::profiles::dmc3::VolumeBootstrapPlan one_volume() {
    constexpr std::array<std::uint32_t, 1> present{0U};
    return dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy::plan(present);
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::LocalDirectorySource;
    using dmc::rengine::gdspaces::SourceRegistry;
    using dmc::rengine::profiles::dmc3::ArchiveSourceBinding;
    using dmc::rengine::profiles::dmc3::RuntimeLookupEvidenceClass;
    using dmc::rengine::profiles::dmc3::RuntimeResolutionStatus;
    using dmc::rengine::profiles::dmc3::RuntimeResourceResolver;
    using dmc::rengine::profiles::dmc3::RuntimeSourceBindings;

    const auto root = unique_test_root();
    std::error_code cleanup_error;
    std::filesystem::remove_all(root, cleanup_error);
    write_file(root / "GDataX360.afs" / "physical-hit.pac");

    // Controlled physical hit: no archive volumes, first physical candidate hits
    // through IDirectPathSource and preserves the enumerated ResourceRef identity.
    {
        SourceRegistry registry;
        assert(registry.mount(std::make_unique<LocalDirectorySource>(
            "physical", root, true)));
        RuntimeSourceBindings bindings{
            .physical_source_id = "physical",
            .archives = {},
        };
        const auto report = RuntimeResourceResolver::resolve(
            "physical-hit.pac", no_volumes(), bindings, registry);
        assert(report.ok());
        assert(report.resolved->id.logical_path ==
            "GDataX360.afs/physical-hit.pac");
        assert(report.probes.size() == 1U);
        assert(report.probes[0].lookup_evidence ==
            RuntimeLookupEvidenceClass::product_physical_native_path);
        assert(report.probes[0].direct_lookup.has_value());
        assert(report.probes[0].direct_lookup->resolved());
    }

    // Controlled complete miss: all six physical candidates are tested natively
    // and the resolver returns not-found without manufacturing an index match.
    {
        SourceRegistry registry;
        assert(registry.mount(std::make_unique<LocalDirectorySource>(
            "physical", root, true)));
        RuntimeSourceBindings bindings{
            .physical_source_id = "physical",
            .archives = {},
        };
        const auto report = RuntimeResourceResolver::resolve(
            "missing.pac", no_volumes(), bindings, registry);
        assert(report.status == RuntimeResolutionStatus::not_found);
        assert(report.probes.size() == 6U);
        for (const auto& probe : report.probes) {
            assert(probe.lookup_evidence ==
                RuntimeLookupEvidenceClass::product_physical_native_path);
            assert(probe.direct_lookup.has_value());
            assert(!probe.direct_lookup->resolved());
        }
    }

    // Controlled archive->physical fallback: one mounted archive misses all six
    // archive candidates, then the first physical candidate resolves natively.
    {
        SourceRegistry registry;
        assert(registry.mount(std::make_unique<LocalDirectorySource>(
            "physical", root, true)));
        assert(registry.mount(std::make_unique<StaticSource>("archive-0")));
        RuntimeSourceBindings bindings{
            .physical_source_id = "physical",
            .archives = {ArchiveSourceBinding{0U, "archive-0"}},
        };
        const auto report = RuntimeResourceResolver::resolve(
            "physical-hit.pac", one_volume(), bindings, registry);
        assert(report.ok());
        assert(report.probes.size() == 7U);
        for (std::size_t index = 0U; index < 6U; ++index) {
            assert(report.probes[index].lookup_evidence ==
                RuntimeLookupEvidenceClass::recovered_archive_index);
            assert(!report.probes[index].direct_lookup.has_value());
        }
        assert(report.probes[6].lookup_evidence ==
            RuntimeLookupEvidenceClass::product_physical_native_path);
        assert(report.probes[6].direct_lookup.has_value());
        assert(report.probes[6].direct_lookup->resolved());
    }

    std::filesystem::remove_all(root, cleanup_error);
    return 0;
}
