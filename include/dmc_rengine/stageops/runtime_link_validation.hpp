#pragma once

#include "dmc_rengine/stageops/runtime_links.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace dmc::rengine::stageops {

struct StageRuntimeLinkValidationIssue final {
    std::string code;
    std::string link_id;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct StageRuntimeLinkValidationReport final {
    std::vector<StageRuntimeLinkValidationIssue> issues;
    std::size_t accepted_link_count{};

    [[nodiscard]] bool valid() const noexcept {
        return issues.empty();
    }
};

class StageRuntimeLinkValidator final {
public:
    [[nodiscard]] static StageRuntimeLinkValidationReport validate(
        const StageDomainWorkspace& domains,
        const std::vector<StageRuntimeLink>& links);
};

} // namespace dmc::rengine::stageops
