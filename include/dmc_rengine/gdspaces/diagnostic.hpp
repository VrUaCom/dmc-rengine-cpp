#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

enum class DiagnosticSeverity {
    info,
    warning,
    error,
};

[[nodiscard]] constexpr std::string_view to_string(
    DiagnosticSeverity severity) noexcept {
    switch (severity) {
    case DiagnosticSeverity::info: return "info";
    case DiagnosticSeverity::warning: return "warning";
    case DiagnosticSeverity::error: return "error";
    }
    return "error";
}

struct Diagnostic final {
    DiagnosticSeverity severity{DiagnosticSeverity::info};
    std::string code;
    std::string message;
    std::optional<ResourceId> resource;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

} // namespace dmc::rengine::gdspaces
