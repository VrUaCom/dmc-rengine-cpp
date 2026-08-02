#pragma once

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/evidence/packet.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::evidence {

struct EvidenceImportLimits final {
    core::json::ParseLimits json{};
    std::size_t max_artifacts{4096U};
    std::size_t max_records{100000U};
    std::size_t max_locations_per_record{4096U};
    std::size_t max_tags_per_record{1024U};
    std::size_t max_supersedes_per_record{1024U};
    std::size_t max_id_bytes{256U};
    std::size_t max_title_bytes{4096U};
    std::size_t max_summary_bytes{1024U * 1024U};
};

struct EvidenceImportDiagnostic final {
    std::string path;
    std::string message;
};

struct EvidenceImportResult final {
    std::optional<EvidencePacket> packet;
    std::vector<EvidenceImportDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return packet.has_value() && diagnostics.empty();
    }
};

[[nodiscard]] EvidenceImportResult evidence_packet_from_json(
    std::string_view input,
    EvidenceImportLimits limits = {});

} // namespace dmc::rengine::evidence
