#pragma once

#include "dmc_rengine/evidence/confidence.hpp"
#include "dmc_rengine/evidence/location.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dmc::rengine::evidence {

/// A figure a record states, bound to the name of the counter that produces it.
///
/// A summary is prose and its numbers age silently: a later fix moves a count
/// and the sentence claiming it stays as it was. Naming the counter makes the
/// claim checkable against a fresh report, which is the only thing that turns
/// "published once" into "still true".
struct EvidenceFigure final {
    std::string counter;
    std::uint64_t value{};

    [[nodiscard]] bool valid() const noexcept { return !counter.empty(); }

    friend bool operator==(const EvidenceFigure&, const EvidenceFigure&) = default;
};

struct EvidenceRecord final {
    std::string id;
    std::string claim_id;
    std::string title;
    std::string summary;
    Confidence confidence{Confidence::hypothesis};
    std::vector<EvidenceLocation> locations;
    std::vector<std::string> tags;
    std::vector<std::string> supersedes;
    /// Figures this record states, each named by the counter it comes from.
    /// Optional: a record that claims no countable quantity carries none.
    std::vector<EvidenceFigure> figures;

    [[nodiscard]] bool valid() const noexcept {
        if (id.empty() || claim_id.empty() || title.empty() || summary.empty()) {
            return false;
        }

        for (const auto& figure : figures) {
            if (!figure.valid()) {
                return false;
            }
        }

        for (const auto& location : locations) {
            if (!location.valid()) {
                return false;
            }
        }

        return true;
    }

    friend bool operator==(const EvidenceRecord&, const EvidenceRecord&) = default;
};

} // namespace dmc::rengine::evidence
