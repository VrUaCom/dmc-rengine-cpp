#pragma once

#include "dmc_rengine/evidence/record.hpp"

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace dmc::rengine::evidence {

class EvidenceRegistry final {
public:
    [[nodiscard]] bool add(EvidenceRecord record);
    [[nodiscard]] bool replace(EvidenceRecord record);
    [[nodiscard]] const EvidenceRecord* find(std::string_view id) const noexcept;
    [[nodiscard]] std::vector<const EvidenceRecord*> by_claim(
        std::string_view claim_id) const;
    [[nodiscard]] std::vector<const EvidenceRecord*> by_confidence(
        Confidence confidence) const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<EvidenceRecord> records_;
};

} // namespace dmc::rengine::evidence
