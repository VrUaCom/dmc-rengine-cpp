#pragma once

#include "dmc_rengine/formats/mod.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::formats::mod {

// First authoring gate for MOD. This mode intentionally preserves the physical
// source layout and refuses structural, companion-sensitive or undecoded-field
// edits. It is not a canonical rebuild-from-scratch authority.
enum class WriteMode : std::uint8_t {
    preserve_layout,
};

struct WriteResult final {
    bool success{false};
    std::vector<std::byte> bytes;
    ParseResult reparsed;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Writer final {
public:
    [[nodiscard]] static WriteResult write(
        const Document& document,
        WriteMode mode = WriteMode::preserve_layout);
};

} // namespace dmc::rengine::formats::mod
