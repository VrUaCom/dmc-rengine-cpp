#pragma once

#include "dmc_rengine/spider/exe_window_packet.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace dmc::rengine::spider {

enum class ExeWindowPacketPublicationError {
    none,
    invalid_plan,
    execution_failed,
    output_unavailable,
    write_failed,
};

struct ExeWindowPacketPublication final {
    ExeWindowPacketPublicationError error{ExeWindowPacketPublicationError::none};
    ExeWindowPacketError execution_error{ExeWindowPacketError::none};
    std::size_t failed_window{};
    std::filesystem::path receipt_path;
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return error == ExeWindowPacketPublicationError::none;
    }
};

[[nodiscard]] std::string exe_window_packet_summary_to_json(
    const ExeWindowPacketSummary& summary);

// Metadata-only Tarantula workflow. Compiles exact plan bytes, acquires and
// validates all windows through the existing executor, then reserves a NEW
// output directory. Writes the exact plan, canonical child receipts and their
// hashes, and the packet receipt LAST. Existing outputs are never replaced.
// On failure only the directory reserved by this call is removed. Consumers
// must require packet.receipt.json; directory existence alone is not success.
// Raw-byte publication (--hex) remains on the Python path for now.
[[nodiscard]] ExeWindowPacketPublication publish_exe_window_packet(
    std::string_view plan_json,
    std::string_view expected_artifact_sha256,
    const std::filesystem::path& output_directory,
    ExeWindowAcquireFn acquire,
    void* context = nullptr);

} // namespace dmc::rengine::spider
