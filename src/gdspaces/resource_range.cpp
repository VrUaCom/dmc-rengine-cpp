#include "dmc_rengine/gdspaces/resource_range.hpp"

#include <algorithm>

namespace dmc::rengine::gdspaces {

bool ResourceRangePayload::readable() const noexcept {
    return exact() && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

} // namespace dmc::rengine::gdspaces
