#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::evidence {

enum class RuntimeTracePhase : std::uint8_t {
    function_entry,
    before_call,
    after_call,
    function_exit,
};

struct RuntimeTraceTarget final {
    std::uint64_t function_va{};
    std::vector<std::uint8_t> expected_bytes;
    std::size_t capture_window_bytes{};

    [[nodiscard]] bool valid() const noexcept {
        return function_va != 0 && !expected_bytes.empty() && capture_window_bytes != 0;
    }

    friend bool operator==(const RuntimeTraceTarget&, const RuntimeTraceTarget&) = default;
};

struct RuntimeTracePlan final {
    std::string target_sha256;
    std::vector<RuntimeTraceTarget> targets;

    [[nodiscard]] bool valid() const noexcept {
        if (!is_sha256_hex(target_sha256) || targets.empty()) {
            return false;
        }

        for (const auto& target : targets) {
            if (!target.valid()) {
                return false;
            }
        }

        return true;
    }

private:
    [[nodiscard]] static bool is_sha256_hex(std::string_view value) noexcept {
        if (value.size() != 64) {
            return false;
        }

        for (const char ch : value) {
            const bool digit = ch >= '0' && ch <= '9';
            const bool lower = ch >= 'a' && ch <= 'f';
            const bool upper = ch >= 'A' && ch <= 'F';
            if (!digit && !lower && !upper) {
                return false;
            }
        }

        return true;
    }
};

struct RuntimeMemorySnapshot final {
    std::uint64_t address{};
    std::vector<std::uint8_t> bytes;

    [[nodiscard]] bool valid() const noexcept {
        return address != 0 && !bytes.empty();
    }

    friend bool operator==(const RuntimeMemorySnapshot&, const RuntimeMemorySnapshot&) = default;
};

struct RuntimeTraceEvent final {
    std::uint64_t sequence{};
    RuntimeTracePhase phase{RuntimeTracePhase::function_entry};
    std::uint64_t function_va{};
    std::optional<std::uint64_t> caller_va;
    std::optional<std::uint32_t> selected_source;
    std::optional<std::uint32_t> reject_mask;
    std::optional<std::uint32_t> dynamic_category;
    std::optional<std::int64_t> result_code;
    std::vector<RuntimeMemorySnapshot> snapshots;

    [[nodiscard]] bool valid() const noexcept {
        if (function_va == 0) {
            return false;
        }

        for (const auto& snapshot : snapshots) {
            if (!snapshot.valid()) {
                return false;
            }
        }

        return true;
    }

    friend bool operator==(const RuntimeTraceEvent&, const RuntimeTraceEvent&) = default;
};

} // namespace dmc::rengine::evidence
