#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::exe {

inline constexpr std::size_t k_max_process_memory_window_size = 0x1000U;

enum class ProcessMemoryWindowError {
    none,
    invalid_request,
    platform_not_supported,
    open_process_failed,
    process_times_failed,
    image_path_failed,
    module_query_failed,
    module_identity_mismatch,
    address_overflow,
    read_failed,
    partial_read,
};

[[nodiscard]] constexpr std::string_view to_string(
    ProcessMemoryWindowError error) noexcept {
    switch (error) {
    case ProcessMemoryWindowError::none: return "none";
    case ProcessMemoryWindowError::invalid_request: return "invalid_request";
    case ProcessMemoryWindowError::platform_not_supported:
        return "platform_not_supported";
    case ProcessMemoryWindowError::open_process_failed:
        return "open_process_failed";
    case ProcessMemoryWindowError::process_times_failed:
        return "process_times_failed";
    case ProcessMemoryWindowError::image_path_failed: return "image_path_failed";
    case ProcessMemoryWindowError::module_query_failed: return "module_query_failed";
    case ProcessMemoryWindowError::module_identity_mismatch:
        return "module_identity_mismatch";
    case ProcessMemoryWindowError::address_overflow: return "address_overflow";
    case ProcessMemoryWindowError::read_failed: return "read_failed";
    case ProcessMemoryWindowError::partial_read: return "partial_read";
    }
    return "unknown";
}

struct ProcessMemoryWindow final {
    std::uint32_t pid{};
    // Exact Windows process creation FILETIME captured from the same process
    // handle used for QueryFullProcessImageNameW/ReadProcessMemory. This makes
    // PID reuse distinguishable across evidence acquired at different times.
    std::uint64_t process_creation_filetime{};
    std::filesystem::path image_path;
    std::uint64_t module_base{};
    std::uint64_t rva{};
    std::uint64_t runtime_va{};
    std::vector<std::byte> bytes;

    [[nodiscard]] bool valid() const noexcept;
};

struct ProcessMemoryWindowResult final {
    std::optional<ProcessMemoryWindow> window;
    ProcessMemoryWindowError error{ProcessMemoryWindowError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return window.has_value() && window->valid() &&
            error == ProcessMemoryWindowError::none;
    }
};

// Reads a bounded byte window from the main executable module of one explicit
// process. The caller supplies an RVA, never a preferred/static VA. On Windows
// the implementation opens exactly the requested PID, captures the process
// creation FILETIME from that same handle, verifies that the first Toolhelp
// module path matches QueryFullProcessImageNameW for that process, and requires
// ReadProcessMemory to return the complete requested range. Other platforms
// fail closed with platform_not_supported.
[[nodiscard]] ProcessMemoryWindowResult capture_main_module_window(
    std::uint32_t pid,
    std::uint64_t rva,
    std::size_t size);

} // namespace dmc::rengine::exe
