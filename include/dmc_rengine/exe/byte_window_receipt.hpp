#pragma once

#include "dmc_rengine/exe/byte_window.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::exe {

struct ExeByteWindowReceipt final {
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::uint64_t image_base{};
    std::uint64_t va{};
    std::uint32_t rva{};
    std::uint32_t file_offset{};
    std::uint64_t size{};
    std::string section_name;
    std::string window_sha256;

    [[nodiscard]] bool valid() const noexcept;
};

// Deterministic metadata receipt. Raw bytes are omitted by default. When
// bytes_hex is supplied it must contain exactly size*2 hexadecimal digits.
[[nodiscard]] std::string byte_window_receipt_to_json(
    const ExeByteWindowReceipt& receipt,
    std::string_view bytes_hex = {});

} // namespace dmc::rengine::exe
