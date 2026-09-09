#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::codecs::dds_bc {

enum class Compression : std::uint8_t {
    dxt1,
    dxt5,
};

enum class Status : std::uint8_t {
    ok,
    truncated,
    invalid_magic,
    invalid_header,
    invalid_dimensions,
    invalid_mip_count,
    unsupported_compression,
    payload_overflow,
    payload_out_of_bounds,
};

[[nodiscard]] constexpr std::string_view to_string(Status status) noexcept {
    switch (status) {
    case Status::ok: return "ok";
    case Status::truncated: return "truncated";
    case Status::invalid_magic: return "invalid-magic";
    case Status::invalid_header: return "invalid-header";
    case Status::invalid_dimensions: return "invalid-dimensions";
    case Status::invalid_mip_count: return "invalid-mip-count";
    case Status::unsupported_compression: return "unsupported-compression";
    case Status::payload_overflow: return "payload-overflow";
    case Status::payload_out_of_bounds: return "payload-out-of-bounds";
    }
    return "invalid-header";
}

struct Document final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_count{};
    Compression compression{Compression::dxt1};
    std::uint32_t payload_size{};
    std::uint32_t total_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct ParseResult final {
    Status status{Status::invalid_header};
    Document document;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

struct RgbaImage final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint8_t> rgba8;

    [[nodiscard]] bool available() const noexcept;
};

struct DecodeResult final {
    bool ok{};
    RgbaImage image;
    std::string_view detail;
};

inline constexpr std::size_t header_size = 128U;

[[nodiscard]] std::uint32_t maximum_mip_count(
    std::uint32_t width,
    std::uint32_t height) noexcept;

[[nodiscard]] bool payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    Compression compression,
    std::uint32_t* output) noexcept;

// Parses a bounded DXT1/DXT5 DDS image beginning at bytes[0]. The parser
// validates only standard structural requirements needed by the shared reader
// layer. It intentionally does not require the exact DMC3 canonical header
// profile and it does not require the input span to end at DDS EOF; callers can
// decide whether any trailing bytes are an allowed carrier/alignment envelope.
[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes) noexcept;

// Decodes mip level 0 into RGBA8. The explicit pixel budget keeps all platform
// shells bounded when they open untrusted resources.
[[nodiscard]] DecodeResult decode_base_mip_rgba8(
    std::span<const std::byte> bytes,
    const Document& document,
    std::uint64_t max_pixels = 4ULL * 1024ULL * 1024ULL) noexcept;

[[nodiscard]] constexpr const char* compression_name(Compression compression) noexcept {
    return compression == Compression::dxt1 ? "DXT1" : "DXT5";
}

} // namespace dmc::rengine::codecs::dds_bc
