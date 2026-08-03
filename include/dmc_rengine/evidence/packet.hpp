#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/evidence/record.hpp"

#include <cstdint>
#include <map>
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
        std::map<std::string, std::uint64_t, std::less<>> artifact_sizes;
        for (const auto& artifact : artifacts) {
            if (!artifact.valid() || !artifact_ids.insert(artifact.id).second) {
                return false;
            }
            artifact_sizes.emplace(artifact.id, artifact.size);
        }

        std::set<std::string> record_ids;
        for (const auto& record : records) {
            if (!record.valid() || !record_ids.insert(record.id).second) {
                return false;
            }
            for (const auto& location : record.locations) {
                const auto artifact = artifact_sizes.find(location.artifact_id);
                if (artifact == artifact_sizes.end()) {
                    return false;
                }
                if (location.size.has_value() && *location.size == 0U) {
                    return false;
                }
                if (!location.file_offset.has_value()) {
                    continue;
                }

                const auto offset = *location.file_offset;
                if (offset >= artifact->second) {
                    return false;
                }
                if (location.size.has_value() &&
                    *location.size > artifact->second - offset) {
                    return false;
                }
            }
        }

        return true;
    }
};

} // namespace dmc::rengine::evidence
