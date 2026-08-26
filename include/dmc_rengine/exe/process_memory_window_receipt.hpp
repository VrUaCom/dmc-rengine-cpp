#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::exe {

// Legacy metadata receipt retained for deterministic reacquisition/testing.
// It does not carry a Windows process-instance discriminator and therefore
// must not satisfy the final real R2B/R3 promotion gate once v2 is available.
struct ProcessMemoryWindowReceipt final {
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::string image_path;
    std::uint64_t preferred_image_base{};
    std::uint32_t pid{};
    std::uint64_t module_base{};
    std::uint64_t rva{};
    std::uint64_t runtime_va{};
    std::uint64_t size{};
    std::string section_name;
    std::string window_sha256;
    std::optional<std::string> expected_window_artifact_sha256;
    std::optional<std::string> expected_window_sha256;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool has_mapping_expectation() const noexcept;
    [[nodiscard]] bool matches_expected_window() const noexcept;
};

// Process-instance-bound receipt for real R2B/R3 promotion. The creation
// FILETIME is captured from the same Windows process HANDLE used for the
// bounded memory read. PID/module-base equality without this value is not a
// sufficient long-lived process-instance identity.
struct ProcessMemoryWindowReceiptV2 final {
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::string image_path;
    std::uint64_t preferred_image_base{};
    std::uint32_t pid{};
    std::uint64_t process_creation_filetime{};
    std::uint64_t module_base{};
    std::uint64_t rva{};
    std::uint64_t runtime_va{};
    std::uint64_t size{};
    std::string section_name;
    std::string window_sha256;
    std::optional<std::string> expected_window_artifact_sha256;
    std::optional<std::string> expected_window_sha256;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool has_mapping_expectation() const noexcept;
    [[nodiscard]] bool matches_expected_window() const noexcept;
};

// Deterministic legacy receipt for one bounded live main-module range. Raw
// bytes are omitted by default. A mapping expectation is valid only when both
// the canonical source artifact SHA and canonical window SHA are present.
[[nodiscard]] std::string process_memory_window_receipt_to_json(
    const ProcessMemoryWindowReceipt& receipt,
    std::string_view bytes_hex = {});

// Process-instance-bound v2 equivalent. Emits schema
// `dmc-rengine.exe-process-window.v2` and includes creation FILETIME as an
// unsigned decimal 64-bit value. It does not upgrade or reinterpret v1.
[[nodiscard]] std::string process_memory_window_receipt_v2_to_json(
    const ProcessMemoryWindowReceiptV2& receipt,
    std::string_view bytes_hex = {});

} // namespace dmc::rengine::exe
