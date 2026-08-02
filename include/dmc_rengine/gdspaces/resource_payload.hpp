#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourcePayload final {
    ResourceRef resource;
    std::vector<std::byte> bytes;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool readable() const noexcept {
        if (!resource.valid()) {
            return false;
        }

        for (const auto& diagnostic : diagnostics) {
            if (diagnostic.severity == DiagnosticSeverity::error) {
                return false;
            }
        }

        return true;
    }
};

} // namespace dmc::rengine::gdspaces
