#pragma once

#include "dmc_rengine/gdspaces/resource_key_index.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct ArchiveSourceBinding final {
    std::uint32_t volume_index{};
    std::string source_id;

    [[nodiscard]] bool valid() const noexcept {
        return !source_id.empty() &&
            VolumeBootstrapPolicy::runtime_index_valid(volume_index);
    }
};

struct ArchiveResourceIndexBinding final {
    std::uint32_t volume_index{};
    std::string source_id;
    gdspaces::ResourceKeyIndex key_index;

    [[nodiscard]] bool valid() const noexcept;
};

class RuntimeResourceIndexBindings final {
public:
    // Builds provider-normalized indexes directly from the currently mounted
    // exact source enumerations. Callers cannot substitute an unrelated index
    // with a matching source-id string.
    [[nodiscard]] static std::optional<RuntimeResourceIndexBindings> build(
        const VolumeBootstrapPlan& bootstrap,
        std::string physical_source_id,
        std::span<const ArchiveSourceBinding> archives,
        const gdspaces::SourceRegistry& sources);

    [[nodiscard]] bool valid_for(
        const VolumeBootstrapPlan& bootstrap,
        const gdspaces::SourceRegistry& sources) const noexcept;

    [[nodiscard]] std::string_view physical_source_id() const noexcept;
    [[nodiscard]] const gdspaces::ResourceKeyIndex& physical_index() const noexcept;
    [[nodiscard]] const ArchiveResourceIndexBinding* archive(
        std::uint32_t volume_index) const noexcept;

private:
    std::string physical_source_id_;
    gdspaces::ResourceKeyIndex physical_index_;
    std::vector<ArchiveResourceIndexBinding> archives_;
};

enum class RuntimeResolutionStatus {
    resolved,
    not_found,
    ambiguous,
    invalid_request,
    invalid_source_configuration,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeResolutionStatus status) noexcept {
    switch (status) {
    case RuntimeResolutionStatus::resolved: return "resolved";
    case RuntimeResolutionStatus::not_found: return "not-found";
    case RuntimeResolutionStatus::ambiguous: return "ambiguous";
    case RuntimeResolutionStatus::invalid_request: return "invalid-request";
    case RuntimeResolutionStatus::invalid_source_configuration:
        return "invalid-source-configuration";
    }
    return "invalid-request";
}

struct RuntimeResolutionProbe final {
    std::size_t lookup_attempt_index{};
    ResourceProviderClass provider{ResourceProviderClass::archive};
    std::string candidate;
    std::string provider_key;
    std::string source_id;
    std::optional<std::uint32_t> archive_volume_index;
    gdspaces::ResourceKeyMatchReport lookup;
};

struct RuntimeResolutionReport final {
    std::string request;
    RuntimeResolutionStatus status{RuntimeResolutionStatus::invalid_request};
    std::optional<gdspaces::ResourceRef> resolved;
    std::vector<gdspaces::ResourceRef> ambiguous_matches;
    std::vector<RuntimeResolutionProbe> probes;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RuntimeResolutionStatus::resolved && resolved.has_value();
    }
};

class RuntimeResourceResolver final {
public:
    [[nodiscard]] static RuntimeResolutionReport resolve(
        std::string_view request,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeResourceIndexBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
