#pragma once

#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/spider/exe_window_packet.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace dmc::rengine::spider {

// A native Spider anchor over the canonical executable byte-window authority.
// The executable is loaded, SHA-gated, and PE-parsed once; every window in a
// Spider Build reuses that immutable session instead of launching a subprocess.
class NativeExeWindowSource final {
public:
    [[nodiscard]] static std::unique_ptr<NativeExeWindowSource> open(
        const std::filesystem::path& path,
        std::string_view expected_sha256,
        std::string& error);

    [[nodiscard]] ExeWindowAcquisition acquire(
        const ExeWindowRequest& request) const;

    [[nodiscard]] const std::string& artifact_sha256() const noexcept {
        return artifact_sha256_;
    }

    [[nodiscard]] std::uint64_t artifact_size() const noexcept {
        return static_cast<std::uint64_t>(bytes_.size());
    }

private:
    NativeExeWindowSource(
        std::vector<std::byte> bytes,
        exe::PeImage image,
        std::string artifact_sha256);

    std::vector<std::byte> bytes_;
    exe::PeImage image_;
    std::string artifact_sha256_;
};

// Function-pointer adapter used by execute_exe_window_packet. The executor
// stays allocation-light and does not require std::function or virtual dispatch.
[[nodiscard]] ExeWindowAcquisition acquire_native_exe_window(
    void* context,
    const ExeWindowRequest& request);

} // namespace dmc::rengine::spider
