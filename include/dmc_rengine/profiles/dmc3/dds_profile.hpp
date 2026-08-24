#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class Dmc3DdsCompression : std::uint8_t {
    dxt1,
    dxt5,
};

enum class Dmc3DdsStatus : std::uint8_t {
    ok,
    truncated,
    invalid_magic,
    invalid_header,
    unsupported_compression,
    unsupported_dimensions,
    invalid_mip_chain,
    invalid_payload_size,
};

[[nodiscard]] constexpr std::string_view to_string(Dmc3DdsStatus status) noexcept {
    switch (status) {
    case Dmc3DdsStatus::ok: return "ok";
    case Dmc3DdsStatus::truncated: return "truncated";
    case Dmc3DdsStatus::invalid_magic: return "invalid-magic";
    case Dmc3DdsStatus::invalid_header: return "invalid-header";
    case Dmc3DdsStatus::unsupported_compression: return "unsupported-compression";
    case Dmc3DdsStatus::unsupported_dimensions: return "unsupported-dimensions";
    case Dmc3DdsStatus::invalid_mip_chain: return "invalid-mip-chain";
    case Dmc3DdsStatus::invalid_payload_size: return "invalid-payload-size";
    }
    return "invalid-header";
}

struct Dmc3DdsSafety final {
    // Product authoring envelope derived from the preserved descriptor-backed
    // corpus. This is not claimed as an original-runtime maximum.
    std::uint32_t min_dimension{64U};
    std::uint32_t max_dimension{1024U};
};

struct Dmc3DdsDocument final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_map_count{};
    Dmc3DdsCompression compression{Dmc3DdsCompression::dxt1};
    std::uint32_t payload_size{};
    std::uint32_t total_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct Dmc3DdsParseResult final {
    Dmc3DdsStatus status{Dmc3DdsStatus::invalid_header};
    Dmc3DdsDocument document;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

struct Dmc3DdsBuildResult final {
    Dmc3DdsStatus status{Dmc3DdsStatus::invalid_header};
    Dmc3DdsDocument document;
    std::vector<std::byte> bytes;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

class Dmc3DdsProfile final {
public:
    static constexpr std::size_t k_header_size = 128U;

    // Validates the exact DMC3 DDS header profile observed in every one of the
    // 154 descriptor-backed real-corpus DDS images used by L1 Passes 78-81.
    [[nodiscard]] static Dmc3DdsParseResult parse(
        std::span<const std::byte> bytes,
        Dmc3DdsSafety safety = {});

    // Builds the canonical 128-byte DMC3 DDS header plus an exact full-chain
    // DXT payload. The emitted header is deterministic from width, height and
    // compression; Pass 81 corpus reconstruction matched 154/154 source
    // headers byte-for-byte.
    [[nodiscard]] static Dmc3DdsBuildResult build(
        std::uint32_t width,
        std::uint32_t height,
        Dmc3DdsCompression compression,
        std::span<const std::byte> payload,
        Dmc3DdsSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
