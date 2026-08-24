#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"

#include <array>
#include <cctype>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool valid_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    for (const unsigned char ch : value) {
        if (!std::isxdigit(ch)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool ascii_hex_equal(
    std::string_view left,
    std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < left.size(); ++index) {
        const auto a = static_cast<unsigned char>(left[index]);
        const auto b = static_cast<unsigned char>(right[index]);
        if (std::tolower(a) != std::tolower(b)) {
            return false;
        }
    }
    return true;
}

} // namespace

bool ExecutableAuthority::valid() const noexcept {
    if (id.empty() || display_name.empty() || !valid_sha256(sha256) || file_size == 0U) {
        return false;
    }

    switch (role) {
    case ExecutableAuthorityRole::analysis_reverse:
        return instruction_reverse_authority &&
            !distribution_provenance_authority &&
            !original_execution_candidate;
    case ExecutableAuthorityRole::protected_distribution:
        return !instruction_reverse_authority &&
            distribution_provenance_authority &&
            original_execution_candidate;
    }
    return false;
}

const ExecutableAuthority& canonical_analysis_executable() noexcept {
    static constexpr ExecutableAuthority authority{
        .id = "dmc3-hdc-unpacked-analysis-e454",
        .display_name = "DMC3 HD unpacked/decrypted canonical analysis executable",
        .sha256 = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .file_size = 6'356'432ULL,
        .role = ExecutableAuthorityRole::analysis_reverse,
        .instruction_reverse_authority = true,
        .distribution_provenance_authority = false,
        .original_execution_candidate = false,
    };
    return authority;
}

const ExecutableAuthority& protected_distribution_executable() noexcept {
    static constexpr ExecutableAuthority authority{
        .id = "dmc3-hdc-protected-distribution-81c7",
        .display_name = "DMC3 HD protected vanilla distribution executable",
        .sha256 = "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6",
        .file_size = 6'567'320ULL,
        .role = ExecutableAuthorityRole::protected_distribution,
        .instruction_reverse_authority = false,
        .distribution_provenance_authority = true,
        .original_execution_candidate = true,
    };
    return authority;
}

ExecutableAuthorityMatch classify_executable_authority(
    std::string_view sha256,
    std::uint64_t file_size) noexcept {
    constexpr std::array<const ExecutableAuthority* (*)(), 2> authorities{
        []() -> const ExecutableAuthority* { return &canonical_analysis_executable(); },
        []() -> const ExecutableAuthority* { return &protected_distribution_executable(); },
    };

    for (const auto factory : authorities) {
        const auto* authority = factory();
        if (!ascii_hex_equal(sha256, authority->sha256)) {
            continue;
        }
        if (file_size != authority->file_size) {
            return ExecutableAuthorityMatch{
                .status = ExecutableAuthorityMatchStatus::known_hash_size_mismatch,
                .authority = authority,
            };
        }
        return ExecutableAuthorityMatch{
            .status = ExecutableAuthorityMatchStatus::recognized,
            .authority = authority,
        };
    }

    return ExecutableAuthorityMatch{};
}

} // namespace dmc::rengine::profiles::dmc3
