#include "dmc_rengine/exe/process_memory_window.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

int main() {
    using dmc::rengine::exe::ProcessMemoryWindowError;
    using dmc::rengine::exe::capture_main_module_window;
    using dmc::rengine::exe::k_max_process_memory_window_size;

    const auto invalid_pid = capture_main_module_window(0U, 0U, 2U);
    assert(!invalid_pid.ok());
    assert(invalid_pid.error == ProcessMemoryWindowError::invalid_request);

    const auto invalid_size = capture_main_module_window(1U, 0U, 0U);
    assert(!invalid_size.ok());
    assert(invalid_size.error == ProcessMemoryWindowError::invalid_request);

    const auto oversized = capture_main_module_window(
        1U, 0U, k_max_process_memory_window_size + 1U);
    assert(!oversized.ok());
    assert(oversized.error == ProcessMemoryWindowError::invalid_request);

#ifdef _WIN32
    const auto current_pid = static_cast<std::uint32_t>(GetCurrentProcessId());
    const auto self = capture_main_module_window(current_pid, 0U, 2U);
    assert(self.ok());
    assert(self.window->pid == current_pid);
    assert(self.window->process_creation_filetime != 0U);
    assert(!self.window->image_path.empty());
    assert(self.window->module_base != 0U);
    assert(self.window->rva == 0U);
    assert(self.window->runtime_va == self.window->module_base);
    assert(self.window->bytes.size() == 2U);
    assert(std::to_integer<unsigned char>(self.window->bytes[0]) == 'M');
    assert(std::to_integer<unsigned char>(self.window->bytes[1]) == 'Z');

    const auto self_again = capture_main_module_window(current_pid, 0U, 2U);
    assert(self_again.ok());
    assert(self_again.window->pid == self.window->pid);
    assert(self_again.window->module_base == self.window->module_base);
    assert(self_again.window->process_creation_filetime ==
           self.window->process_creation_filetime);
#else
    const auto unsupported = capture_main_module_window(1U, 0U, 2U);
    assert(!unsupported.ok());
    assert(unsupported.error == ProcessMemoryWindowError::platform_not_supported);
#endif

    return 0;
}
