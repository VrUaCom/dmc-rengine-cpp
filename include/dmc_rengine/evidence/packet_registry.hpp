#pragma once

#include "dmc_rengine/evidence/packet.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::evidence {

struct EvidencePacketDescriptor final {
    std::uint32_t schema_version{};
    std::string id;
    std::string title;
    std::string project;
    std::vector<std::string> artifact_ids;
    std::vector<std::string> record_ids;

    [[nodiscard]] bool valid() const noexcept {
        return schema_version != 0U && !id.empty() &&
               !title.empty() && !project.empty();
    }
};

class EvidencePacketRegistry final {
public:
    [[nodiscard]] bool add(const EvidencePacket& packet);
    [[nodiscard]] const EvidencePacketDescriptor* find(
        std::string_view id) const noexcept;
    [[nodiscard]] const std::map<
        std::string, EvidencePacketDescriptor, std::less<>>& all() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::map<std::string, EvidencePacketDescriptor, std::less<>> packets_;
};

} // namespace dmc::rengine::evidence
