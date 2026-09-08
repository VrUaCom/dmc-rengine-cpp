#pragma once

#include "dmc_rengine/formats/clt/ir.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::formats::clt {

enum class ParseError : std::uint8_t {
    none,
    empty,
    non_text_byte,
    nonzero_tail,
    missing_clt_identity,
    missing_terminator,
    invalid_integer_directive,
    invalid_bone_directive,
    duplicate_singleton_directive,
};

struct ParseResult final {
    std::optional<Document> document;
    ParseError error{ParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ParseError::none;
    }
};

class Parser final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats::clt
