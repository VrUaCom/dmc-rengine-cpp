#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::exe {

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

// Deterministic receipt for one bounded live main-module range. Raw bytes are
// omitted by default. A mapping expectation is valid only when both the
// canonical source artifact SHA and canonical window SHA are present.
[[nodiscard]] std::string process_memory_window_receipt_to_json(
    const ProcessMemoryWindowReceipt& receipt,
    std::string_view bytes_hex = {});

} // namespace dmc::rengine::exe
