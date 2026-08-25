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

struct DirectPathLookupResult final {
    DirectPathLookupStatus status{DirectPathLookupStatus::not_found};
    std::optional<ResourceRef> resource;
    std::string detail;

    [[nodiscard]] bool resolved() const noexcept {
        return status == DirectPathLookupStatus::resolved && resource.has_value();
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
