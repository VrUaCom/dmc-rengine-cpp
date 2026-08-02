#pragma once

#include <optional>
#include <string_view>

namespace dmc::rengine::evidence {

enum class Confidence {
    hypothesis,
    candidate,
    low,
    medium,
    high,
    confirmed,
    corrected,
    rejected,
};

[[nodiscard]] constexpr std::string_view to_string(Confidence value) noexcept {
    switch (value) {
    case Confidence::hypothesis: return "hypothesis";
    case Confidence::candidate: return "candidate";
    case Confidence::low: return "low";
    case Confidence::medium: return "medium";
    case Confidence::high: return "high";
    case Confidence::confirmed: return "confirmed";
    case Confidence::corrected: return "corrected";
    case Confidence::rejected: return "rejected";
    }
    return "hypothesis";
}

[[nodiscard]] constexpr std::optional<Confidence> confidence_from_string(
    std::string_view value) noexcept {
    if (value == "hypothesis") return Confidence::hypothesis;
    if (value == "candidate") return Confidence::candidate;
    if (value == "low") return Confidence::low;
    if (value == "medium") return Confidence::medium;
    if (value == "high") return Confidence::high;
    if (value == "confirmed") return Confidence::confirmed;
    if (value == "corrected") return Confidence::corrected;
    if (value == "rejected") return Confidence::rejected;
    return std::nullopt;
}

} // namespace dmc::rengine::evidence
