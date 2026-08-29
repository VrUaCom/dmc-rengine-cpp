#pragma once

#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// Reconstructs the logical content of a previously observed legacy .index
// manifest. Raw labels and directive come only from sealed naming evidence.
// The rendered CRLF text is a canonical replay representation; it is not a
// claim that original line-ending bytes are preserved byte-for-byte.
struct LegacyIndexReplayPlan final {
    gdspaces::ResourceId parent_resource;
    gdspaces::ResourceId manifest_resource;
    std::string manifest_sha256;
    std::string directive;
    std::vector<std::string> raw_entries;
    bool exact_labels_from_external_index{false};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::string render_crlf() const;
};

struct LegacyIndexReplayBuildResult final {
    LegacyIndexReplayPlan plan;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class LegacyIndexReplayPlanner final {
public:
    [[nodiscard]] static LegacyIndexReplayBuildResult build(
        const gdspaces::ContainerNamingIdentitySnapshot& snapshot);
};

} // namespace dmc::rengine::profiles::dmc3
