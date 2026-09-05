#pragma once

#include "dmc_rengine/gdspaces/byte_provenance.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/enclosing_container_name_evidence.hpp"
#include "dmc_rengine/gdspaces/resource_name_evidence.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/resource_semantic_evidence.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourcePayload final {
    ResourceRef resource;
    std::vector<std::byte> bytes;
    std::vector<Diagnostic> diagnostics;
    std::optional<ByteProvenance> byte_provenance;

    // Extraction/alias naming domains. External `.index` and embedded aliases
    // remain deliberately separate evidence kinds.
    std::vector<ResourceNameEvidence> name_evidence;

    // Names stored by an enclosing physical container for this descendant.
    // These are neither external `.index` names nor embedded aliases.
    std::vector<EnclosingContainerNameEvidence> enclosing_container_name_evidence;

    // Byte/structure-backed semantic format evidence, independent of every
    // naming namespace above.
    std::vector<ResourceSemanticEvidence> semantic_evidence;

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
