#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/source/custom_build.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dmc::rengine::source {

struct CustomBuildReopenDiagnostic final {
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct CustomBuildReopenContext final {
    gdspaces::ResourceId executable_resource;
    const CustomBuildRecord* build{nullptr};
    const IntegrationProject* integration_project{nullptr};
    std::vector<const SourceBinaryMapping*> mappings;
    std::vector<CustomBuildReopenDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return executable_resource.valid() && build != nullptr &&
               integration_project != nullptr && !mappings.empty() &&
               diagnostics.empty();
    }

    [[nodiscard]] const SourceBinaryMapping* mapping_for_rva(
        std::uint64_t rva) const noexcept;
};

[[nodiscard]] CustomBuildReopenContext reopen_custom_build(
    const integration::ProjectWorkspace& workspace,
    const gdspaces::ResourceId& executable_resource);

} // namespace dmc::rengine::source
