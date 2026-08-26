#pragma once

#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class PhysicalProviderPlanStatus {
    ready,
    invalid_c_string,
    candidate_overflow,
    normalized_candidate_empty,
    root_overflow,
    joined_path_overflow,
};

struct PhysicalProviderPathPlan final {
    PhysicalProviderPlanStatus status{PhysicalProviderPlanStatus::invalid_c_string};
    std::string root;
    std::string candidate;
    std::string normalized_candidate;
    std::string joined_path;

    [[nodiscard]] bool ready() const noexcept {
        return status == PhysicalProviderPlanStatus::ready;
    }
};

// Pure model of the recovered game-side type-0 path preparation boundary:
// bounded candidate copy -> 0x0C normalization -> bounded root join.
//
// It deliberately stops before executing CreateFileA. This lets tests/receipts
// compare product-side identity selection against the recovered original path
// contract without making the portable GDSpaces resolver depend on Win32.
class PhysicalProviderModel final {
public:
    [[nodiscard]] static PhysicalProviderPathPlan plan(
        std::string_view root,
        std::string_view candidate);
};

} // namespace dmc::rengine::profiles::dmc3
