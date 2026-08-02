#pragma once

#include "dmc_rengine/evidence/confidence.hpp"
#include "dmc_rengine/evidence/location.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::evidence {

struct EvidenceRecord final {
    std::string id;
    std::string claim_id;
    std::string title;
    std::string summary;
    Confidence confidence{Confidence::hypothesis};
    std::vector<EvidenceLocation> locations;
    std::vector<std::string> tags;
    std::vector<std::string> supersedes;

    [[nodiscard]] bool valid() const noexcept {
        if (id.empty() || claim_id.empty() || title.empty() || summary.empty()) {
            return false;
        }

        for (const auto& location : locations) {
            if (!location.valid()) {
                return false;
            }
        }

        return true;
    }

    friend bool operator==(const EvidenceRecord&, const EvidenceRecord&) = default;
};

} // namespace dmc::rengine::evidence
