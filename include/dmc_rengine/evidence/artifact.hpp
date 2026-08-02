#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

namespace dmc::rengine::evidence {

struct ArtifactIdentity final {
    std::string id;
    std::string role;
    std::string sha256;
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept {
        if (id.empty() || role.empty() || sha256.size() != 64U) {
            return false;
        }

        return std::all_of(
            sha256.begin(), sha256.end(),
            [](unsigned char character) {
                return std::isxdigit(character) != 0;
            });
    }

    friend bool operator==(const ArtifactIdentity&, const ArtifactIdentity&) = default;
};

} // namespace dmc::rengine::evidence
