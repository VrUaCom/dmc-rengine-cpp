#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class RecoveredSourceKind {
    subsystem,
    class_type,
    struct_type,
    registry,
    global,
    table,
    function,
    method,
    state_machine,
    policy,
};

[[nodiscard]] constexpr std::string_view to_string(
    RecoveredSourceKind kind) noexcept {
    switch (kind) {
    case RecoveredSourceKind::subsystem: return "subsystem";
    case RecoveredSourceKind::class_type: return "class";
    case RecoveredSourceKind::struct_type: return "struct";
    case RecoveredSourceKind::registry: return "registry";
    case RecoveredSourceKind::global: return "global";
    case RecoveredSourceKind::table: return "table";
    case RecoveredSourceKind::function: return "function";
    case RecoveredSourceKind::method: return "method";
    case RecoveredSourceKind::state_machine: return "state-machine";
    case RecoveredSourceKind::policy: return "policy";
    }
    return "function";
}

enum class RecoveredSourceStatus {
    confirmed,
    very_high,
    high,
    partial,
    open,
    rejected,
};

[[nodiscard]] constexpr std::string_view to_string(
    RecoveredSourceStatus status) noexcept {
    switch (status) {
    case RecoveredSourceStatus::confirmed: return "confirmed";
    case RecoveredSourceStatus::very_high: return "very-high";
    case RecoveredSourceStatus::high: return "high";
    case RecoveredSourceStatus::partial: return "partial";
    case RecoveredSourceStatus::open: return "open";
    case RecoveredSourceStatus::rejected: return "rejected";
    }
    return "open";
}

struct RecoveredSourceSymbol final {
    std::string id;
    std::string parent_id;
    RecoveredSourceKind kind{RecoveredSourceKind::function};
    std::string name;
    std::string summary;
    RecoveredSourceStatus status{RecoveredSourceStatus::open};
    std::optional<std::uint64_t> va;
    std::optional<std::uint64_t> size;
    std::vector<std::uint32_t> evidence_passes;
    std::map<std::string, std::string, std::less<>> attributes;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !name.empty();
    }
};

[[nodiscard]] const std::vector<RecoveredSourceSymbol>&
recovered_source_tree() noexcept;

[[nodiscard]] const RecoveredSourceSymbol* find_recovered_source_symbol(
    std::string_view id) noexcept;

[[nodiscard]] std::vector<const RecoveredSourceSymbol*>
recovered_source_children(std::string_view parent_id);

} // namespace dmc::rengine::profiles::dmc3
