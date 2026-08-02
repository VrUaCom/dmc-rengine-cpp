#pragma once

#include "dmc_rengine/evidence/packet.hpp"

#include <string>

namespace dmc::rengine::evidence {

[[nodiscard]] std::string to_json(const EvidencePacket& packet);

} // namespace dmc::rengine::evidence
