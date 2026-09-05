#pragma once

#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::integration {

using NativeReaderAnalyzeFn = void (*)(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report);

struct NativeReaderModule final {
    std::string parser_id;
    gdspaces::ToolTarget consumer{gdspaces::ToolTarget::binary_inspector};
    bool link_format_evidence{true};
    NativeReaderAnalyzeFn analyze{nullptr};

    [[nodiscard]] bool valid() const noexcept {
        return !parser_id.empty() && analyze != nullptr;
    }
};

} // namespace dmc::rengine::integration
