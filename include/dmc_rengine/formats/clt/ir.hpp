#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::formats::clt {

struct SourceLine final {
    std::size_t source_line{};
    std::string raw;
    std::string key;
    std::string value;
    bool comment{false};
};

struct BoneDirective final {
    std::size_t source_line{};
    std::uint32_t node_index{};
    std::string axis_token;
};

struct Document final {
    // Preservation authority for unknown syntax, line endings and alignment.
    std::vector<std::byte> source_bytes;

    std::size_t physical_size{};
    std::size_t text_size{};
    std::size_t zero_padding_size{};
    std::string embedded_name;
    std::vector<SourceLine> lines;

    // Typed projections are deliberately limited to integer/reference fields
    // needed by current corpus binding analysis. Float-valued solver fields
    // remain available losslessly through SourceLine until EXE semantics land.
    std::optional<std::uint32_t> cloth_num;
    std::optional<std::uint32_t> cloth_no;
    std::optional<std::uint32_t> cloth_id;
    std::optional<std::uint32_t> wind_local_raw;
    std::optional<std::uint32_t> wind_parent_raw;
    std::optional<std::uint32_t> wind_type_raw;
    std::optional<std::uint32_t> limit_length_raw;
    std::vector<BoneDirective> bones;

    [[nodiscard]] bool valid() const noexcept;
};

} // namespace dmc::rengine::formats::clt
