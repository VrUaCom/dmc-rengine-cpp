#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::formats {

enum class ParseSeverity {
    info,
    warning,
    error,
};

[[nodiscard]] constexpr std::string_view to_string(ParseSeverity severity) noexcept {
    switch (severity) {
    case ParseSeverity::info: return "info";
    case ParseSeverity::warning: return "warning";
    case ParseSeverity::error: return "error";
    }
    return "error";
}

struct ParseDiagnostic final {
    ParseSeverity severity{ParseSeverity::info};
    std::string code;
    std::string message;
    std::uint64_t offset{};

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

} // namespace dmc::rengine::formats
