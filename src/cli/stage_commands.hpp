#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

namespace dmc::rengine::cli {

int run_build_stage_workspace(
    std::string_view stage_id,
    const std::filesystem::path& root,
    const std::optional<std::filesystem::path>& evidence_path);

} // namespace dmc::rengine::cli
