#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/evidence/record.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace dmc::rengine::evidence {

struct EvidencePacket final {
    std::uint32_t schema_version{1};
    std::string id;
    std::string title;
    std::string project{"DMC Rengine"};
    std::vector<ArtifactIdentity> artifacts;
    std::vector<EvidenceRecord> records;

    [[nodiscard]] bool valid() const {
        if (schema_version == 0U || id.empty() || title.empty() || project.empty()) {
            return false;
        }

        std::set<std::string> artifact_ids;
        for (const auto& artifact : artifacts) {
            if (!artifact.valid() || !artifact_ids.insert(artifact.id).second) {
                return false;
            }
        }

        std::set<std::string> record_ids;
        for (const auto& record : records) {
            if (!record.valid() || !record_ids.insert(record.id).second) {
                return false;
            }
            for (const auto& location : record.locations) {
                if (!artifact_ids.contains(location.artifact_id)) {
                    return false;
                }
            }
        }

        return true;
    }
};

} // namespace dmc::rengine::evidence
