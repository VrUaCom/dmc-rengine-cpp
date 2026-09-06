#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <vector>

namespace dmc::rengine::formats::shw {

struct Vec4f final {
    float x{};
    float y{};
    float z{};
    float w{};
};

struct Triangle final {
    std::array<std::uint32_t, 3> vertices{};
    std::uint32_t reserved{};
};

struct Adjacency final {
    std::array<std::uint16_t, 3> neighbors{};
    std::uint16_t reserved{};
};

struct Hull final {
    std::uint64_t record_offset{};
    std::uint16_t vertex_count{};
    std::uint16_t triangle_count{};

    std::uint64_t triangle_offset{};
    std::uint64_t adjacency_offset{};
    std::uint64_t vertex_offset{};
    std::uint64_t selector_offset{};

    std::vector<Triangle> triangles;
    std::vector<Adjacency> adjacency;
    std::vector<Vec4f> vertices;
    std::vector<std::uint8_t> transform_selectors;
};

struct Header final {
    float version{};
    std::uint8_t hull_count{};
};

struct Document final {
    Header header;
    std::vector<Hull> hulls;

    // Read-only preservation authority for all unresolved header/record bytes.
    std::vector<std::byte> source_bytes;
};

struct ParseResult final {
    bool recognized{false};
    Document document;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Parser final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::shw
