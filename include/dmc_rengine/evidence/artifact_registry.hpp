#pragma once

#include "dmc_rengine/evidence/artifact.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::evidence {

class ArtifactRegistry final {
public:
    [[nodiscard]] bool add(ArtifactIdentity artifact);
    [[nodiscard]] bool replace(ArtifactIdentity artifact);
    [[nodiscard]] const ArtifactIdentity* find(
        std::string_view id) const noexcept;
    [[nodiscard]] std::vector<const ArtifactIdentity*> by_sha256(
        std::string_view sha256) const;
    [[nodiscard]] std::vector<const ArtifactIdentity*> by_role(
        std::string_view role) const;
    [[nodiscard]] const std::map<
        std::string, ArtifactIdentity, std::less<>>& all() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::map<std::string, ArtifactIdentity, std::less<>> artifacts_;
};

} // namespace dmc::rengine::evidence
