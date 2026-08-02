#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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

struct ContainerEntry final {
    std::uint32_t slot_index{};
    std::uint64_t offset{};
    std::uint64_t size{};
    std::string logical_name;
    bool populated{false};
    bool synthetic_name{false};

    [[nodiscard]] bool valid(std::uint64_t container_size) const noexcept;
};

struct ContainerDocument final {
    std::string format;
    std::uint32_t schema_version{};
    std::uint32_t declared_slot_count{};
    std::uint64_t container_size{};
    std::vector<ContainerEntry> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct ContainerParseResult final {
    ContainerDocument document;
    std::vector<ParseDiagnostic> diagnostics;
    bool recognized{false};

    [[nodiscard]] bool ok() const noexcept;
};

} // namespace dmc::rengine::formats
