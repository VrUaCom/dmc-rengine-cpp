#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::so {

inline constexpr std::size_t type6_header_size = 0x0EU;
inline constexpr std::size_t type8_header_size = 0x08U;
inline constexpr std::size_t link_record_size = 0x04U;
inline constexpr std::size_t volume_record_size = 0x50U;

struct Diagnostic final {
    std::string message;
};

struct IndexedBlock final {
    std::uint16_t type{};
    std::uint64_t base_offset{};
    std::uint64_t extent_size{};
    std::vector<std::uint16_t> header_words;
    std::vector<std::uint16_t> entry_offsets;
};

struct GraphParseResult final {
    bool recognized{false};
    std::vector<IndexedBlock> blocks;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct LinkRecord final {
    std::uint8_t field0{};
    std::uint8_t field1{};
    std::uint8_t field2{};
    std::uint8_t field3{};
};

struct LinkParseResult final {
    bool recognized{false};
    std::vector<LinkRecord> records;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct Vec4 final {
    float x{};
    float y{};
    float z{};
    float w{};
};

struct VolumeRecord final {
    std::uint32_t type{};
    std::array<std::byte, 12> prefix_unknown{};
    Vec4 vector0{};
    Vec4 vector1{};
    Vec4 vector2{};
    Vec4 vector3{};
};

struct VolumeParseResult final {
    bool recognized{false};
    std::vector<VolumeRecord> records;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct CompanionCorrelation final {
    bool one_header_plus_one_link_per_volume{false};
    std::size_t link_record_count{};
    std::size_t volume_record_count{};
};

[[nodiscard]] GraphParseResult parse_graph(std::span<const std::byte> bytes);
[[nodiscard]] LinkParseResult parse_links(std::span<const std::byte> bytes);
[[nodiscard]] VolumeParseResult parse_volumes(std::span<const std::byte> bytes);
[[nodiscard]] CompanionCorrelation correlate_companions(const LinkParseResult& links,
                                                        const VolumeParseResult& volumes) noexcept;

} // namespace dmc::rengine::formats::so
