#pragma once

#include "dmc_rengine/evidence/packet.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::evidence {

struct PacketRangeIssue final {
    std::string record_id;
    std::string artifact_id;
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !record_id.empty() && !artifact_id.empty() &&
               !code.empty() && !message.empty();
    }
};

struct PacketRangeValidation final {
    std::vector<PacketRangeIssue> issues;

    [[nodiscard]] bool ok() const noexcept {
        return issues.empty();
    }
};

[[nodiscard]] PacketRangeValidation validate_packet_artifact_ranges(
    const EvidencePacket& packet);

} // namespace dmc::rengine::evidence
