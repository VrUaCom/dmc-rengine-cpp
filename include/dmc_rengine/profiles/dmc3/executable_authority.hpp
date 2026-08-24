#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class ExecutableAuthorityRole {
    analysis_reverse,
    protected_distribution,
};

[[nodiscard]] constexpr std::string_view to_string(
    ExecutableAuthorityRole role) noexcept {
    switch (role) {
    case ExecutableAuthorityRole::analysis_reverse:
        return "analysis-reverse";
    case ExecutableAuthorityRole::protected_distribution:
        return "protected-distribution";
    }
    return "analysis-reverse";
}

struct ExecutableAuthority final {
    std::string_view id;
    std::string_view display_name;
    std::string_view sha256;
    std::uint64_t file_size{};
    ExecutableAuthorityRole role{ExecutableAuthorityRole::analysis_reverse};
    bool instruction_reverse_authority{};
    bool distribution_provenance_authority{};
    bool original_execution_candidate{};

    [[nodiscard]] bool valid() const noexcept;
};

enum class ExecutableAuthorityMatchStatus {
    recognized,
    unknown,
    known_hash_size_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    ExecutableAuthorityMatchStatus status) noexcept {
    switch (status) {
    case ExecutableAuthorityMatchStatus::recognized:
        return "recognized";
    case ExecutableAuthorityMatchStatus::unknown:
        return "unknown";
    case ExecutableAuthorityMatchStatus::known_hash_size_mismatch:
        return "known-hash-size-mismatch";
    }
    return "unknown";
}

struct ExecutableAuthorityMatch final {
    ExecutableAuthorityMatchStatus status{ExecutableAuthorityMatchStatus::unknown};
    const ExecutableAuthority* authority{};

    [[nodiscard]] bool recognized() const noexcept {
        return status == ExecutableAuthorityMatchStatus::recognized && authority != nullptr;
    }
};

[[nodiscard]] const ExecutableAuthority& canonical_analysis_executable() noexcept;
[[nodiscard]] const ExecutableAuthority& protected_distribution_executable() noexcept;

[[nodiscard]] ExecutableAuthorityMatch classify_executable_authority(
    std::string_view sha256,
    std::uint64_t file_size) noexcept;

} // namespace dmc::rengine::profiles::dmc3
