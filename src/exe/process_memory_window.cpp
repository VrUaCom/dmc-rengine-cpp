#include "dmc_rengine/exe/process_memory_window.hpp"

#include <algorithm>
#include <cwctype>
#include <limits>
#include <sstream>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] bool checked_runtime_va(
    std::uint64_t module_base,
    std::uint64_t rva,
    std::uint64_t& runtime_va) noexcept {
    if (rva > std::numeric_limits<std::uint64_t>::max() - module_base) {
        return false;
    }
    runtime_va = module_base + rva;
    return true;
}

#ifdef _WIN32
class UniqueHandle final {
public:
    explicit UniqueHandle(HANDLE handle = nullptr) noexcept : handle_(handle) {}
    ~UniqueHandle() {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    UniqueHandle(UniqueHandle&& other) noexcept
        : handle_(std::exchange(other.handle_, nullptr)) {}

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
                CloseHandle(handle_);
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    [[nodiscard]] HANDLE get() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept {
        return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
    }

private:
    HANDLE handle_{};
};

[[nodiscard]] std::wstring normalized_windows_path(
    const std::filesystem::path& path) {
    std::error_code error;
    auto normalized = std::filesystem::weakly_canonical(path, error);
    if (error) {
        normalized = path.lexically_normal();
    }

    auto value = normalized.native();
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
        if (ch == L'/') {
            return L'\\';
        }
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return value;
}

[[nodiscard]] std::string win32_error_message(
    std::string_view prefix,
    DWORD error_code) {
    std::ostringstream output;
    output << prefix << " (Win32 error " << error_code << ')';
    return output.str();
}

[[nodiscard]] std::pair<UniqueHandle, DWORD> capture_module_snapshot(
    DWORD pid) noexcept {
    // Microsoft documents ERROR_BAD_LENGTH as a transient module-list race for
    // TH32CS_SNAPMODULE snapshots and recommends retrying. Keep the retry
    // bounded so evidence acquisition remains deterministic/fail-closed.
    constexpr unsigned k_max_attempts = 8U;
    DWORD last_error = ERROR_SUCCESS;
    for (unsigned attempt = 0U; attempt < k_max_attempts; ++attempt) {
        UniqueHandle snapshot{CreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
            pid)};
        if (snapshot.valid()) {
            return {std::move(snapshot), ERROR_SUCCESS};
        }

        last_error = GetLastError();
        if (last_error != ERROR_BAD_LENGTH) {
            break;
        }
    }
    return {UniqueHandle{}, last_error};
}

[[nodiscard]] std::uint64_t filetime_u64(const FILETIME& value) noexcept {
    return (static_cast<std::uint64_t>(value.dwHighDateTime) << 32U) |
        static_cast<std::uint64_t>(value.dwLowDateTime);
}
#endif

} // namespace

bool ProcessMemoryWindow::valid() const noexcept {
    if (pid == 0U || process_creation_filetime == 0U || image_path.empty() ||
        module_base == 0U || bytes.empty() ||
        bytes.size() > k_max_process_memory_window_size) {
        return false;
    }

    std::uint64_t expected_runtime_va = 0U;
    return checked_runtime_va(module_base, rva, expected_runtime_va) &&
        expected_runtime_va == runtime_va;
}

ProcessMemoryWindowResult capture_main_module_window(
    std::uint32_t pid,
    std::uint64_t rva,
    std::size_t size) {
    if (pid == 0U || size == 0U || size > k_max_process_memory_window_size) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::invalid_request,
            .message = "PID must be non-zero and size must be within the bounded process-window limit.",
        };
    }

#ifdef _WIN32
    const UniqueHandle process{OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
        FALSE,
        static_cast<DWORD>(pid))};
    if (!process.valid()) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::open_process_failed,
            .message = win32_error_message(
                "OpenProcess failed", GetLastError()),
        };
    }

    FILETIME creation_time{};
    FILETIME exit_time{};
    FILETIME kernel_time{};
    FILETIME user_time{};
    if (GetProcessTimes(
            process.get(),
            &creation_time,
            &exit_time,
            &kernel_time,
            &user_time) == FALSE) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::process_times_failed,
            .message = win32_error_message(
                "GetProcessTimes failed", GetLastError()),
        };
    }
    const auto process_creation_filetime = filetime_u64(creation_time);
    if (process_creation_filetime == 0U) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::process_times_failed,
            .message = "GetProcessTimes returned a zero process creation FILETIME.",
        };
    }

    std::wstring image_buffer(32768U, L'\0');
    DWORD image_size = static_cast<DWORD>(image_buffer.size());
    if (QueryFullProcessImageNameW(
            process.get(), 0U, image_buffer.data(), &image_size) == FALSE ||
        image_size == 0U) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::image_path_failed,
            .message = win32_error_message(
                "QueryFullProcessImageNameW failed", GetLastError()),
        };
    }
    image_buffer.resize(static_cast<std::size_t>(image_size));
    const std::filesystem::path process_image_path{image_buffer};

    auto [snapshot, snapshot_error] = capture_module_snapshot(
        static_cast<DWORD>(pid));
    if (!snapshot.valid()) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::module_query_failed,
            .message = win32_error_message(
                "CreateToolhelp32Snapshot failed", snapshot_error),
        };
    }

    MODULEENTRY32W module{};
    module.dwSize = sizeof(module);
    if (Module32FirstW(snapshot.get(), &module) == FALSE ||
        module.modBaseAddr == nullptr) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::module_query_failed,
            .message = win32_error_message(
                "Module32FirstW failed", GetLastError()),
        };
    }

    const std::filesystem::path module_path{module.szExePath};
    if (normalized_windows_path(process_image_path) !=
        normalized_windows_path(module_path)) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::module_identity_mismatch,
            .message = "The Toolhelp main module does not match the process image path.",
        };
    }

    const auto module_base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(module.modBaseAddr));
    std::uint64_t runtime_va = 0U;
    if (!checked_runtime_va(module_base, rva, runtime_va) ||
        runtime_va > static_cast<std::uint64_t>(
            std::numeric_limits<std::uintptr_t>::max())) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::address_overflow,
            .message = "module_base + RVA cannot be represented as a process address.",
        };
    }

    std::vector<std::byte> bytes(size);
    SIZE_T bytes_read = 0U;
    if (ReadProcessMemory(
            process.get(),
            reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(runtime_va)),
            bytes.data(),
            static_cast<SIZE_T>(bytes.size()),
            &bytes_read) == FALSE) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::read_failed,
            .message = win32_error_message(
                "ReadProcessMemory failed", GetLastError()),
        };
    }
    if (bytes_read != static_cast<SIZE_T>(bytes.size())) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::partial_read,
            .message = "ReadProcessMemory returned a partial window.",
        };
    }

    ProcessMemoryWindow window{
        .pid = pid,
        .process_creation_filetime = process_creation_filetime,
        .image_path = process_image_path,
        .module_base = module_base,
        .rva = rva,
        .runtime_va = runtime_va,
        .bytes = std::move(bytes),
    };
    if (!window.valid()) {
        return {
            .window = std::nullopt,
            .error = ProcessMemoryWindowError::read_failed,
            .message = "Captured process window failed structural validation.",
        };
    }

    return {
        .window = std::move(window),
        .error = ProcessMemoryWindowError::none,
        .message = {},
    };
#else
    static_cast<void>(rva);
    return {
        .window = std::nullopt,
        .error = ProcessMemoryWindowError::platform_not_supported,
        .message = "Live process-memory acquisition is supported only on Windows.",
    };
#endif
}

} // namespace dmc::rengine::exe
