#pragma once

#include "dmc_rengine/gdspaces/byte_provenance.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_name_evidence.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourcePayload final {
    ResourceRef resource;
    std::vector<std::byte> bytes;
    std::vector<Diagnostic> diagnostics;
    std::optional<ByteProvenance> byte_provenance;
    std::vector<ResourceNameEvidence> name_evidence;

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
