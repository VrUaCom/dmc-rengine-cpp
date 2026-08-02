#include "dmc_rengine/evidence/packet_registry.hpp"

#include <algorithm>

namespace dmc::rengine::evidence {

bool EvidencePacketRegistry::add(const EvidencePacket& packet) {
    if (!packet.valid() || packets_.contains(packet.id)) {
        return false;
    }

    EvidencePacketDescriptor descriptor{
        .schema_version = packet.schema_version,
        .id = packet.id,
        .title = packet.title,
        .project = packet.project,
        .artifact_ids = {},
        .record_ids = {},
    };
    descriptor.artifact_ids.reserve(packet.artifacts.size());
    for (const auto& artifact : packet.artifacts) {
        descriptor.artifact_ids.push_back(artifact.id);
    }
    descriptor.record_ids.reserve(packet.records.size());
    for (const auto& record : packet.records) {
        descriptor.record_ids.push_back(record.id);
    }
    std::sort(descriptor.artifact_ids.begin(), descriptor.artifact_ids.end());
    std::sort(descriptor.record_ids.begin(), descriptor.record_ids.end());

    return packets_.emplace(descriptor.id, std::move(descriptor)).second;
}

const EvidencePacketDescriptor* EvidencePacketRegistry::find(
    std::string_view id) const noexcept {
    const auto iterator = packets_.find(id);
    return iterator == packets_.end() ? nullptr : &iterator->second;
}

const std::map<std::string, EvidencePacketDescriptor, std::less<>>&
EvidencePacketRegistry::all() const noexcept {
    return packets_;
}

std::size_t EvidencePacketRegistry::size() const noexcept {
    return packets_.size();
}

bool EvidencePacketRegistry::empty() const noexcept {
    return packets_.empty();
}

} // namespace dmc::rengine::evidence
