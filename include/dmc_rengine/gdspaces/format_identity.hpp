#pragma once

#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

class ResourceFormatIdentity final {
public:
    // Canonical presentation/export suffix for a proven semantic format.
    // This function never derives ResourceId or write authority.
    [[nodiscard]] static std::string canonical_extension(
        std::string_view semantic_format);
};

} // namespace dmc::rengine::gdspaces
