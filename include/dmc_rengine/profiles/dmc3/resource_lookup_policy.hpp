#pragma once

#include "dmc_rengine/profiles/dmc3/open_game_resource_contract.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class ResourceProviderClass {
    archive,
    physical,
};

[[nodiscard]] constexpr std::string_view to_string(
    ResourceProviderClass provider) noexcept {
    switch (provider) {
    case ResourceProviderClass::archive: return "archive";
    case ResourceProviderClass::physical: return "physical";
    }
    return "archive";
}

struct ResourceLookupAttempt final {
    std::size_t attempt_index{};
    ResourceProviderClass provider{ResourceProviderClass::archive};
    std::uint32_t provider_mask{};
    std::size_t prefix_index{};
    std::string prefix;
    std::string basename;
    std::string candidate;

    [[nodiscard]] bool valid() const noexcept;
};

struct ResourceLookupPlan final {
    std::string original_request;
    std::string basename;
    std::vector<ResourceLookupAttempt> attempts;

    [[nodiscard]] bool valid() const noexcept;
};

class ResourceLookupPolicy final {
public:
    // Read from the recovered contract rather than restated. Two copies of a
    // recovered number drift the first time one of them is edited, and the one
    // that drifts is never the one anybody looks at.
    static constexpr std::size_t candidate_buffer_bytes =
        OpenGameResourceContract::candidate_buffer_bytes;
    static constexpr std::size_t attempts_per_request =
        OpenGameResourceContract::attempts_per_request;

    [[nodiscard]] static constexpr const std::array<std::string_view, 6>&
    namespace_prefixes() noexcept {
        return OpenGameResourceContract::namespace_prefixes;
    }

    [[nodiscard]] static std::string basename_of(std::string_view logical_path);

    [[nodiscard]] static ResourceLookupPlan plan(
        std::string_view logical_path);
};

} // namespace dmc::rengine::profiles::dmc3
