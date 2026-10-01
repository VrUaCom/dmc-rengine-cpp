#pragma once

#include "dmc_rengine/exe/byte_window_receipt.hpp"
#include "dmc_rengine/spider/plan.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::spider {

inline constexpr std::string_view k_exe_window_packet_plan_schema =
    "dmc-rengine.exe-window-packet-plan.v1";
inline constexpr std::string_view k_exe_window_packet_validation_schema =
    "dmc-rengine.exe-window-packet-plan-validation.v1";
inline constexpr std::string_view k_exe_window_packet_receipt_schema =
    "dmc-rengine.exe-window-packet-receipt.v1";

enum class ExeWindowMode : std::uint8_t {
    probe,
    known_body,
};

struct ExeWindowRequest final {
    std::string id;
    std::uint64_t va{};
    std::size_t size{};
    ExeWindowMode mode{ExeWindowMode::probe};
    std::string body_sha256;
    std::vector<std::uint64_t> issues;
    std::string purpose;
};

struct ExeWindowPacketPlan final {
    std::string id;
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::string authority_role;
    std::string window_size_policy;
    std::vector<ExeWindowRequest> windows;
};

struct ExeWindowPacketSummary final {
    std::string plan_id;
    std::string plan_sha256;
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::string authority_role;
    std::size_t window_count{};
    std::size_t probe_count{};
    std::size_t known_body_count{};
    bool semantic_claim{false};
};

struct ExeWindowPacketProgram final {
    ExeWindowPacketPlan packet;
    ExeWindowPacketSummary summary;
    Plan execution;
};

struct ExeWindowPacketCompileResult final {
    std::optional<ExeWindowPacketProgram> program;
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const noexcept {
        return program.has_value() && errors.empty();
    }
};

struct ExeWindowAcquisition final {
    std::optional<exe::ExeByteWindowReceipt> receipt;
    std::string error;

    [[nodiscard]] bool ok() const noexcept {
        return receipt.has_value() && error.empty();
    }
};

using ExeWindowAcquireFn = ExeWindowAcquisition (*)(
    void* context,
    const ExeWindowRequest& request);

enum class ExeWindowPacketError : std::uint8_t {
    none,
    expected_sha_mismatch,
    acquisition_failed,
    invalid_receipt,
    known_body_mismatch,
};

struct ExeWindowPacketExecution final {
    std::vector<exe::ExeByteWindowReceipt> receipts;
    ExeWindowPacketError error{ExeWindowPacketError::none};
    std::size_t failed_window{};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return error == ExeWindowPacketError::none;
    }
};

// C++20 replacement for the validation/orchestration responsibilities of
// scripts/reverse/extract_exe_window_packet.py. It compiles JSON into a compact
// Spider Plan; it does not duplicate PE mapping or byte-window extraction.
[[nodiscard]] ExeWindowPacketCompileResult compile_exe_window_packet(
    std::string_view plan_json);

// Executes the compiled Spider plan through an injected native acquisition
// node. Production adapters can bind this to the canonical EXE acquisition
// authority; tests can inject deterministic fixtures without subprocesses.
[[nodiscard]] ExeWindowPacketExecution execute_exe_window_packet(
    const ExeWindowPacketProgram& program,
    std::string_view expected_artifact_sha256,
    ExeWindowAcquireFn acquire,
    void* context = nullptr);

[[nodiscard]] bool validate_exe_window_receipt(
    const exe::ExeByteWindowReceipt& receipt,
    const ExeWindowPacketPlan& plan,
    const ExeWindowRequest& request) noexcept;

} // namespace dmc::rengine::spider
