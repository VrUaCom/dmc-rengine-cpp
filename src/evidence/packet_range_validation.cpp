#include "dmc_rengine/evidence/packet_range_validation.hpp"

#include <map>
#include <string>

namespace dmc::rengine::evidence {
namespace {

void add_issue(
    PacketRangeValidation& result,
    const EvidenceRecord& record,
    const EvidenceLocation& location,
    std::string code,
    std::string message) {
    result.issues.push_back(PacketRangeIssue{
        .record_id = record.id,
        .artifact_id = location.artifact_id,
        .code = std::move(code),
        .message = std::move(message),
    });
}

} // namespace

PacketRangeValidation validate_packet_artifact_ranges(
    const EvidencePacket& packet) {
    PacketRangeValidation result;
    std::map<std::string, const ArtifactIdentity*, std::less<>> artifacts;
    for (const auto& artifact : packet.artifacts) {
        artifacts.emplace(artifact.id, &artifact);
    }

    for (const auto& record : packet.records) {
        for (const auto& location : record.locations) {
            const auto artifact_iterator = artifacts.find(location.artifact_id);
            if (artifact_iterator == artifacts.end()) {
                add_issue(
                    result,
                    record,
                    location,
                    "evidence-range.artifact-missing",
                    "The EvidenceLocation references an artifact absent from the packet.");
                continue;
            }
            const auto& artifact = *artifact_iterator->second;
            if (location.size.has_value() && *location.size == 0U) {
                add_issue(
                    result,
                    record,
                    location,
                    "evidence-range.zero-size",
                    "An explicit EvidenceLocation size must be non-zero.");
            }
            if (!location.file_offset.has_value()) {
                continue;
            }

            const auto offset = *location.file_offset;
            if (offset >= artifact.size) {
                add_issue(
                    result,
                    record,
                    location,
                    "evidence-range.file-offset-outside-artifact",
                    "The physical file offset is outside the referenced artifact size.");
                continue;
            }
            if (location.size.has_value() &&
                *location.size > artifact.size - offset) {
                add_issue(
                    result,
                    record,
                    location,
                    "evidence-range.file-range-outside-artifact",
                    "The physical file range extends beyond the referenced artifact size.");
            }
        }
    }
    return result;
}

} // namespace dmc::rengine::evidence
