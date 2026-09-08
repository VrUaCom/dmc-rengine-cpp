#include "dmc_rengine/spider/exe_window_packet.hpp"

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/byte_window.hpp"

#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::spider {
namespace {

using core::json::Value;

[[nodiscard]] const Value* member(
    const Value::Object& object,
    std::string_view key) noexcept {
    const auto it = object.find(key);
    return it == object.end() ? nullptr : &it->second;
}

[[nodiscard]] std::optional<std::uint64_t> parse_u64_text(
    std::string_view text) noexcept {
    if (text.empty()) {
        return std::nullopt;
    }

    int base = 10;
    if (text.size() > 2U && text[0] == '0' &&
        (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        text.remove_prefix(2U);
        if (text.empty()) {
            return std::nullopt;
        }
    }

    std::uint64_t result{};
    const auto parsed = std::from_chars(
        text.data(), text.data() + text.size(), result, base);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        return std::nullopt;
    }
    return result;
}

[[nodiscard]] std::optional<std::uint64_t> parse_u64_value(
    const Value* value) noexcept {
    if (value == nullptr) {
        return std::nullopt;
    }
    if (const auto* number = value->as_u64()) {
        return *number;
    }
    if (const auto* number = value->as_i64()) {
        if (*number < 0) {
            return std::nullopt;
        }
        return static_cast<std::uint64_t>(*number);
    }
    if (const auto* text = value->as_string()) {
        return parse_u64_text(*text);
    }
    return std::nullopt;
}

[[nodiscard]] bool is_hex_digit(char value) noexcept {
    return (value >= '0' && value <= '9') ||
        (value >= 'a' && value <= 'f') ||
        (value >= 'A' && value <= 'F');
}

[[nodiscard]] std::optional<std::string> canonical_sha256(
    const Value* value) {
    if (value == nullptr) {
        return std::nullopt;
    }
    const auto* text = value->as_string();
    if (text == nullptr || text->size() != 64U) {
        return std::nullopt;
    }

    std::string normalized;
    normalized.reserve(64U);
    for (const auto character : *text) {
        if (!is_hex_digit(character)) {
            return std::nullopt;
        }
        normalized.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))));
    }
    return normalized;
}

[[nodiscard]] std::optional<std::string> canonical_sha256_text(
    std::string_view text) {
    if (text.size() != 64U) {
        return std::nullopt;
    }
    std::string normalized;
    normalized.reserve(64U);
    for (const auto character : text) {
        if (!is_hex_digit(character)) {
            return std::nullopt;
        }
        normalized.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))));
    }
    return normalized;
}

[[nodiscard]] const std::string* non_empty_string(
    const Value* value) noexcept {
    if (value == nullptr) {
        return nullptr;
    }
    const auto* text = value->as_string();
    return text != nullptr && !text->empty() ? text : nullptr;
}

[[nodiscard]] bool safe_window_id(std::string_view value) noexcept {
    if (value.empty()) {
        return false;
    }
    for (const auto character : value) {
        const bool accepted =
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') ||
            character == '-' || character == '_' || character == '.';
        if (!accepted) {
            return false;
        }
    }
    return true;
}

void error(ExeWindowPacketCompileResult& result, std::string message) {
    result.errors.push_back(std::move(message));
}

[[nodiscard]] std::string plan_sha256(std::string_view source) {
    const auto characters = std::span<const char>{source.data(), source.size()};
    return core::Sha256::compute(std::as_bytes(characters)).hex();
}

[[nodiscard]] bool parse_issues(
    const Value* value,
    std::vector<std::uint64_t>& output) {
    if (value == nullptr) {
        return false;
    }
    const auto* array = value->as_array();
    if (array == nullptr) {
        return false;
    }

    output.clear();
    output.reserve(array->size());
    for (const auto& item : *array) {
        std::optional<std::uint64_t> issue;
        if (const auto* number = item.as_u64()) {
            issue = *number;
        } else if (const auto* number = item.as_i64(); number != nullptr && *number > 0) {
            issue = static_cast<std::uint64_t>(*number);
        }
        if (!issue || *issue == 0U) {
            return false;
        }
        output.push_back(*issue);
    }
    return true;
}

} // namespace

ExeWindowPacketCompileResult compile_exe_window_packet(
    std::string_view plan_json) {
    ExeWindowPacketCompileResult result;
    const auto parsed = core::json::Parser::parse(plan_json);
    if (!parsed.ok()) {
        error(result, "plan must be valid JSON");
        return result;
    }

    const auto* root = parsed.value->as_object();
    if (root == nullptr) {
        error(result, "plan root must be an object");
        return result;
    }

    const auto* schema = non_empty_string(member(*root, "schema"));
    if (schema == nullptr || *schema != k_exe_window_packet_plan_schema) {
        error(result, "unsupported or missing plan schema");
        return result;
    }

    ExeWindowPacketProgram program;
    const auto* id = non_empty_string(member(*root, "id"));
    if (id == nullptr) {
        error(result, "plan id must be a non-empty string");
        return result;
    }
    program.packet.id = *id;

    const auto artifact_sha = canonical_sha256(member(*root, "artifact_sha256"));
    if (!artifact_sha) {
        error(result, "artifact_sha256 must be exactly 64 hexadecimal characters");
        return result;
    }
    program.packet.artifact_sha256 = *artifact_sha;

    const auto artifact_size = parse_u64_value(member(*root, "artifact_size"));
    if (!artifact_size || *artifact_size == 0U) {
        error(result, "artifact_size must be a non-zero uint64 value");
        return result;
    }
    program.packet.artifact_size = *artifact_size;

    const auto* authority_role = non_empty_string(member(*root, "authority_role"));
    if (authority_role == nullptr) {
        error(result, "authority_role must be a non-empty string");
        return result;
    }
    program.packet.authority_role = *authority_role;

    const auto* size_policy = non_empty_string(member(*root, "window_size_policy"));
    if (size_policy == nullptr) {
        error(result, "window_size_policy must be a non-empty string");
        return result;
    }
    program.packet.window_size_policy = *size_policy;

    const auto* windows_value = member(*root, "windows");
    const auto* windows = windows_value != nullptr ? windows_value->as_array() : nullptr;
    if (windows == nullptr || windows->empty()) {
        error(result, "windows must be a non-empty array");
        return result;
    }

    std::set<std::string, std::less<>> ids;
    program.packet.windows.reserve(windows->size());
    for (std::size_t index = 0; index < windows->size(); ++index) {
        const auto* object = windows->at(index).as_object();
        const auto prefix = "windows[" + std::to_string(index) + "]";
        if (object == nullptr) {
            error(result, prefix + ": expected object");
            return result;
        }

        ExeWindowRequest request;
        const auto* window_id = non_empty_string(member(*object, "id"));
        if (window_id == nullptr || !safe_window_id(*window_id)) {
            error(result, prefix + ".id: expected a safe non-empty id");
            return result;
        }
        if (!ids.insert(*window_id).second) {
            error(result, prefix + ".id: duplicate id");
            return result;
        }
        request.id = *window_id;

        const auto va = parse_u64_value(member(*object, "va"));
        if (!va || *va == 0U) {
            error(result, prefix + ".va: zero or invalid VA");
            return result;
        }
        request.va = *va;

        const auto size = parse_u64_value(member(*object, "size"));
        if (!size || *size == 0U || *size > exe::k_max_exe_byte_window_size ||
            *size > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
            error(result, prefix + ".size: outside executable byte-window limit");
            return result;
        }
        request.size = static_cast<std::size_t>(*size);

        const auto* mode = non_empty_string(member(*object, "mode"));
        if (mode == nullptr) {
            error(result, prefix + ".mode: expected 'probe' or 'known-body'");
            return result;
        }
        if (*mode == "probe") {
            request.mode = ExeWindowMode::probe;
        } else if (*mode == "known-body") {
            request.mode = ExeWindowMode::known_body;
            const auto body_sha = canonical_sha256(member(*object, "body_sha256"));
            if (!body_sha) {
                error(result, prefix + ".body_sha256: invalid SHA-256");
                return result;
            }
            request.body_sha256 = *body_sha;
        } else {
            error(result, prefix + ".mode: expected 'probe' or 'known-body'");
            return result;
        }

        if (!parse_issues(member(*object, "issues"), request.issues)) {
            error(result, prefix + ".issues: expected positive integer array");
            return result;
        }

        const auto* purpose = non_empty_string(member(*object, "purpose"));
        if (purpose == nullptr) {
            error(result, prefix + ".purpose: expected non-empty string");
            return result;
        }
        request.purpose = *purpose;
        program.packet.windows.push_back(std::move(request));
    }

    program.summary = ExeWindowPacketSummary{
        .plan_id = program.packet.id,
        .plan_sha256 = plan_sha256(plan_json),
        .artifact_sha256 = program.packet.artifact_sha256,
        .artifact_size = program.packet.artifact_size,
        .authority_role = program.packet.authority_role,
        .window_count = program.packet.windows.size(),
        .probe_count = 0U,
        .known_body_count = 0U,
        .semantic_claim = false,
    };

    program.execution.instructions.push_back(Instruction{
        .op = OpCode::validate_plan,
        .domain = ExecutionDomain::cpu,
        .dependency_count = 0U,
        .operand = 0U,
    });
    for (std::size_t index = 0; index < program.packet.windows.size(); ++index) {
        const auto& window = program.packet.windows[index];
        if (window.mode == ExeWindowMode::probe) {
            ++program.summary.probe_count;
        } else {
            ++program.summary.known_body_count;
        }
        program.execution.instructions.push_back(Instruction{
            .op = OpCode::acquire_window,
            .domain = ExecutionDomain::io,
            .dependency_count = 1U,
            .operand = static_cast<std::uint32_t>(index),
        });
        program.execution.instructions.push_back(Instruction{
            .op = OpCode::validate_window,
            .domain = ExecutionDomain::cpu,
            .dependency_count = 1U,
            .operand = static_cast<std::uint32_t>(index),
        });
    }
    program.execution.instructions.push_back(Instruction{
        .op = OpCode::publish_packet,
        .domain = ExecutionDomain::io,
        .dependency_count = 1U,
        .operand = 0U,
    });

    result.program = std::move(program);
    return result;
}

bool validate_exe_window_receipt(
    const exe::ExeByteWindowReceipt& receipt,
    const ExeWindowPacketPlan& plan,
    const ExeWindowRequest& request) noexcept {
    return receipt.valid() &&
        receipt.artifact_sha256 == plan.artifact_sha256 &&
        receipt.artifact_size == plan.artifact_size &&
        receipt.va == request.va &&
        receipt.size == static_cast<std::uint64_t>(request.size);
}

ExeWindowPacketExecution execute_exe_window_packet(
    const ExeWindowPacketProgram& program,
    std::string_view expected_artifact_sha256,
    ExeWindowAcquireFn acquire,
    void* context) {
    ExeWindowPacketExecution output;
    const auto expected_sha = canonical_sha256_text(expected_artifact_sha256);
    if (!expected_sha || *expected_sha != program.packet.artifact_sha256) {
        output.error = ExeWindowPacketError::expected_sha_mismatch;
        output.message = "expected SHA does not match packet artifact authority";
        return output;
    }
    if (acquire == nullptr) {
        output.error = ExeWindowPacketError::acquisition_failed;
        output.message = "Spider acquisition node is not bound";
        return output;
    }

    output.receipts.reserve(program.packet.windows.size());
    for (std::size_t index = 0; index < program.packet.windows.size(); ++index) {
        const auto& request = program.packet.windows[index];
        auto acquired = acquire(context, request);
        if (!acquired.ok()) {
            output.error = ExeWindowPacketError::acquisition_failed;
            output.failed_window = index;
            output.message = acquired.error.empty()
                ? "native acquisition node failed"
                : std::move(acquired.error);
            return output;
        }

        const auto& receipt = *acquired.receipt;
        if (!validate_exe_window_receipt(receipt, program.packet, request)) {
            output.error = ExeWindowPacketError::invalid_receipt;
            output.failed_window = index;
            output.message = "native acquisition node returned a mismatched receipt";
            return output;
        }
        if (request.mode == ExeWindowMode::known_body &&
            receipt.window_sha256 != request.body_sha256) {
            output.error = ExeWindowPacketError::known_body_mismatch;
            output.failed_window = index;
            output.message = "known-body SHA mismatch";
            return output;
        }
        output.receipts.push_back(receipt);
    }

    return output;
}

} // namespace dmc::rengine::spider
