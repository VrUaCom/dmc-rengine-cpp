#include "dmc_rengine/evidence/registry.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::evidence {

bool EvidenceRegistry::add(EvidenceRecord record) {
    if (!record.valid() || find(record.id) != nullptr) {
        return false;
    }

    records_.push_back(std::move(record));
    return true;
}

bool EvidenceRegistry::replace(EvidenceRecord record) {
    if (!record.valid()) {
        return false;
    }

    const auto iterator = std::find_if(
        records_.begin(), records_.end(),
        [&record](const EvidenceRecord& existing) {
            return existing.id == record.id;
        });

    if (iterator == records_.end()) {
        return false;
    }

    *iterator = std::move(record);
    return true;
}

const EvidenceRecord* EvidenceRegistry::find(std::string_view id) const noexcept {
    const auto iterator = std::find_if(
        records_.begin(), records_.end(),
        [id](const EvidenceRecord& record) {
            return record.id == id;
        });

    return iterator == records_.end() ? nullptr : &*iterator;
}

std::vector<const EvidenceRecord*> EvidenceRegistry::by_claim(
    std::string_view claim_id) const {
    std::vector<const EvidenceRecord*> result;
    for (const auto& record : records_) {
        if (record.claim_id == claim_id) {
            result.push_back(&record);
        }
    }
    return result;
}

std::vector<const EvidenceRecord*> EvidenceRegistry::by_confidence(
    Confidence confidence) const {
    std::vector<const EvidenceRecord*> result;
    for (const auto& record : records_) {
        if (record.confidence == confidence) {
            result.push_back(&record);
        }
    }
    return result;
}

std::size_t EvidenceRegistry::size() const noexcept {
    return records_.size();
}

bool EvidenceRegistry::empty() const noexcept {
    return records_.empty();
}

} // namespace dmc::rengine::evidence
