#pragma once

#include "dmc_rengine/gdspaces/resource_key_index.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct SourceKeyIndexBinding final {
    std::string source_id;
    const gdspaces::ResourceKeyIndex* key_index{};

    [[nodiscard]] bool valid(std::uint32_t expected_flags) const noexcept;
};

struct ArchiveSourceBinding final {
    std::uint32_t volume_index{};
    SourceKeyIndexBinding source;

    [[nodiscard]] bool valid() const noexcept;
};

struct RuntimeSourceBindings final {
    SourceKeyIndexBinding physical;
    std::vector<ArchiveSourceBinding> archives;

    [[nodiscard]] bool valid_for(const VolumeBootstrapPlan& bootstrap) const noexcept;
    [[nodiscard]] const ArchiveSourceBinding* archive(
        std::uint32_t volume_index) const noexcept;
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

// DMC3-profile composition layer. It owns recovered candidate/provider/source
// traversal order while exact source I/O remains in SourceRegistry/ISource and
// per-source normalized lookup remains in ResourceKeyIndex.
class RuntimeResourceResolver final {
public:
    [[nodiscard]] static RuntimeResolutionReport resolve(
        std::string_view request,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
