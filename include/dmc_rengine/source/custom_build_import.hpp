#pragma once

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/source/custom_build.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::source {

/**
 * Reads a Custom Build Record from JSON.
 *
 * The counterpart to `custom_build_manifest_json`, which is a *report* about a
 * registered build rather than a save format: it carries workspace-derived
 * fields such as `graph_registered` and `sha256_reopen_lookup`. Those describe
 * the workspace that emitted it, not the build, so they are ignored here — a
 * record must not import a claim about a workspace it is entering.
 *
 * Everything else is read back, because `CustomBuildRecord::valid()` demands a
 * complete record and the registry refuses an incomplete one. That refusal is
 * the point: a build with no source-to-binary mappings, no toolchain identity
 * or no test results is not evidence of a recompilation, and importing it
 * would let the Source Map answer from nothing.
 */

struct CustomBuildImportLimits final {
    core::json::ParseLimits json{};
    std::size_t max_modifications{4096U};
    std::size_t max_mappings{200000U};
    std::size_t max_test_results{4096U};
    std::size_t max_string_list_items{4096U};
};

struct CustomBuildImportDiagnostic final {
    std::string path;
    std::string message;
};

struct CustomBuildImportResult final {
    std::optional<CustomBuildRecord> record;
    std::vector<CustomBuildImportDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return record.has_value() && diagnostics.empty();
    }
};

[[nodiscard]] CustomBuildImportResult custom_build_from_json(
    std::string_view input,
    CustomBuildImportLimits limits = {});

} // namespace dmc::rengine::source
