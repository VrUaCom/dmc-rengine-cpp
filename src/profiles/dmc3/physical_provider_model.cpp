#include "dmc_rengine/profiles/dmc3/physical_provider_model.hpp"

#include "dmc_rengine/profiles/dmc3/physical_provider_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool c_string_compatible(std::string_view value) noexcept {
    return value.find('\0') == std::string_view::npos;
}

[[nodiscard]] PhysicalProviderPathPlan fail(
    PhysicalProviderPlanStatus status,
    std::string_view root,
    std::string_view candidate,
    std::string normalized = {}) {
    return PhysicalProviderPathPlan{
        .status = status,
        .root = std::string{root},
        .candidate = std::string{candidate},
        .normalized_candidate = std::move(normalized),
        .joined_path = {},
    };
}

} // namespace

PhysicalProviderPathPlan PhysicalProviderModel::plan(
    std::string_view root,
    std::string_view candidate) {
    constexpr auto capacity = PhysicalProviderContract::path_capacity;

    if (!c_string_compatible(root) || !ResourcePathPolicy::valid_input(candidate)) {
        return fail(
            PhysicalProviderPlanStatus::invalid_c_string, root, candidate);
    }

    // The recovered resolver copies the raw candidate into a 0x400 local
    // buffer before normalization, so the terminating NUL must also fit.
    if (candidate.size() >= capacity) {
        return fail(
            PhysicalProviderPlanStatus::candidate_overflow, root, candidate);
    }

    auto normalized = ResourcePathPolicy::physical(candidate);
    if (normalized.empty()) {
        return fail(
            PhysicalProviderPlanStatus::normalized_candidate_empty,
            root,
            candidate,
            std::move(normalized));
    }

    // The recovered root-join helper uses the same 0x400 destination capacity.
    // A root that cannot fit with its terminating NUL cannot produce a path.
    if (root.size() >= capacity) {
        return fail(
            PhysicalProviderPlanStatus::root_overflow,
            root,
            candidate,
            std::move(normalized));
    }

    std::string joined{root};
    if (!joined.empty() &&
        !PhysicalProviderContract::root_has_join_separator(joined.back())) {
        joined.push_back('\\');
    }
    joined.append(normalized);

    if (joined.size() >= capacity) {
        return fail(
            PhysicalProviderPlanStatus::joined_path_overflow,
            root,
            candidate,
            std::move(normalized));
    }

    return PhysicalProviderPathPlan{
        .status = PhysicalProviderPlanStatus::ready,
        .root = std::string{root},
        .candidate = std::string{candidate},
        .normalized_candidate = std::move(normalized),
        .joined_path = std::move(joined),
    };
}

} // namespace dmc::rengine::profiles::dmc3
