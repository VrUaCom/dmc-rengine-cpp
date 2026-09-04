#pragma once

#include "dmc_rengine/formats/scm.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::formats::scm {

enum class WriteMode {
    // Same-size authoring over a parsed source image. Existing offsets,
    // padding, unknown bytes and index-workspace contents remain source-bound.
    preserve_layout,

    // Deterministic product rebuild from the typed IR. This is not a claim of
    // Capcom offline-packer byte equivalence; acceptance remains gated on
    // corpus round-trip and original-game consumption.
    canonical_rebuild,
};

struct WriteResult final {
    bool wrote{false};
    bool reparse_ok{false};
    bool bit_identical_to_source{false};
    std::vector<std::byte> bytes;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Writer final {
public:
    [[nodiscard]] static WriteResult write(
        const Document& document,
        WriteMode mode);
};

} // namespace dmc::rengine::formats::scm
