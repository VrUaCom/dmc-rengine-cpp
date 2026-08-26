#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

enum class DirectPathLookupStatus {
    resolved,
    not_found,
    rejected,
    io_error,
};

// Spelled here rather than in each consumer. A frontend that invents its own
// wording for a recovered status turns a canonical fact into a product opinion,
// and two consumers then disagree about what the same result was called.
[[nodiscard]] constexpr std::string_view to_string(
    DirectPathLookupStatus status) noexcept {
    switch (status) {
    case DirectPathLookupStatus::resolved: return "resolved";
    case DirectPathLookupStatus::not_found: return "not-found";
    case DirectPathLookupStatus::rejected: return "rejected";
    case DirectPathLookupStatus::io_error: return "io-error";
    }
    return "not-found";
}

struct DirectPathLookupResult final {
    DirectPathLookupStatus status{DirectPathLookupStatus::not_found};
    std::optional<ResourceRef> resource;
    std::string detail;

    [[nodiscard]] bool valid() const noexcept {
        switch (status) {
        case DirectPathLookupStatus::resolved:
            return resource.has_value() && resource->valid();
        case DirectPathLookupStatus::not_found:
        case DirectPathLookupStatus::rejected:
        case DirectPathLookupStatus::io_error:
            return !resource.has_value();
        }
        return false;
    }

    [[nodiscard]] bool resolved() const noexcept {
        return valid() && status == DirectPathLookupStatus::resolved;
    }
};

// Optional source capability for resolving one already-normalized logical path
// through the source's native path backend rather than through an enumerated
// ResourceKeyIndex. The resolver still owns candidate construction,
// normalization and provider precedence; this interface only owns exact
// source-local path resolution and ResourceRef identity recovery.
class IDirectPathSource {
public:
    virtual ~IDirectPathSource() = default;

    [[nodiscard]] virtual DirectPathLookupResult lookup_direct_path(
        std::string_view logical_path) const = 0;
};

} // namespace dmc::rengine::gdspaces
