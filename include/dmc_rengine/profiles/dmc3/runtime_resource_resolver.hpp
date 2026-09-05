#pragma once

#include "dmc_rengine/gdspaces/direct_path_source.hpp"
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

struct ArchiveSourceBinding final {
    std::uint32_t volume_index{};
    std::string source_id;

    [[nodiscard]] bool valid() const noexcept;
};

struct RuntimeSourceBindings final {
    // Empty exactly when the recovered type-0 physical registration did not
    // successfully link a node into this runtime topology.
    std::string physical_source_id;
    std::vector<ArchiveSourceBinding> archives;

    [[nodiscard]] bool valid_for(const RuntimeMountTopology& topology) const noexcept;
    [[nodiscard]] const ArchiveSourceBinding* archive(
        std::uint32_t volume_index) const noexcept;
};

enum class RuntimeLookupEvidenceClass {
    // The original ZIP/NBZ backend owns a normalized sorted lookup array and
    // queries it after 0x0E normalization.
    recovered_archive_index,

    // Product-native direct path lookup through an IDirectPathSource. On
    // Windows LocalDirectorySource this follows the host filesystem path
    // semantics rather than forcing archive-style key equality. It remains
    // product-classified because containment hardening and non-Windows host
    // behavior are intentionally not claimed as original CreateFileA parity.
    product_physical_native_path,

    // Fallback for physical sources that do not expose direct path lookup.
    // This is a source-derived 0x0C ResourceKeyIndex and is mechanically
    // different from the recovered original direct path backend.
    product_physical_index,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeLookupEvidenceClass evidence) noexcept {
    switch (evidence) {
    case RuntimeLookupEvidenceClass::recovered_archive_index:
        return "recovered-archive-index";
    case RuntimeLookupEvidenceClass::product_physical_native_path:
        return "product-physical-native-path";
    case RuntimeLookupEvidenceClass::product_physical_index:
        return "product-physical-index";
    }
    return "product-physical-index";
}

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
    RuntimeLookupEvidenceClass lookup_evidence{
        RuntimeLookupEvidenceClass::recovered_archive_index};
    std::string candidate;
    std::string provider_key;
    std::string source_id;
    std::optional<std::uint32_t> archive_volume_index;
    gdspaces::ResourceKeyMatchReport lookup;
    std::optional<gdspaces::DirectPathLookupResult> direct_lookup;
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

// DMC3-profile composition layer. It consumes only the explicit successful
// linked-list topology, never filename-discovery evidence. Archive
// ResourceKeyIndex values are derived inside resolve() from the exact currently
// mounted ISource enumeration. Physical sources may additionally expose
// IDirectPathSource so the physical phase can use source-native path lookup.
// Exact byte I/O remains in SourceRegistry/ISource.
class RuntimeResourceResolver final {
public:
    [[nodiscard]] static RuntimeResolutionReport resolve(
        std::string_view request,
        const RuntimeMountTopology& topology,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
