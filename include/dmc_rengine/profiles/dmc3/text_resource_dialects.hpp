#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

/**
 * The DMC3 authoring formats that happen to be serialized as text.
 *
 * "Text" is a property of the encoding, not an identity. A cloth definition
 * and a scroll table are both readable ASCII and are not the same resource,
 * and calling either one `txt` throws away what the payload says about itself
 * in its own first line.
 *
 * Recovered 2026-09-08 from a complete `em000` extraction: 8 CLT payloads and
 * 1 TSC, every one of which announces its dialect in its opening bytes.
 */
enum class TextResourceDialect : std::uint8_t {
    unknown,
    /// Cloth/deformation definition. Opens `;<name>.clt`.
    clt,
    /// Motion/scroll control table. Opens with a `.TSC` tag line.
    tsc,
    /// The effect pack's `<kind> <id>` record manifest.
    effect_manifest,
};

struct TextResourceIdentity final {
    TextResourceDialect dialect{TextResourceDialect::unknown};

    /**
     * The original filename the payload states about itself, when it does.
     *
     * A CLT opens with `;em002_01.clt` — a comment carrying the name the file
     * had before it was packed. That is an *embedded original name*: not a
     * placeholder this project synthesized, not a line an extraction tool
     * wrote beside it, but the resource naming itself in its own bytes.
     *
     * It is worth keeping separate from the slot it was found in, because the
     * two disagree. In the recovered corpus `em000`'s slots 6, 9, 11, 14, 16,
     * 20 and 22 hold cloth belonging to `em001`, `em002`, `em003` and `em005`
     * — the same definitions shared across enemies. A name taken from the
     * enclosing container would have been wrong for seven of eight.
     */
    std::optional<std::string> embedded_original_name;

    [[nodiscard]] bool recognized() const noexcept {
        return dialect != TextResourceDialect::unknown;
    }
};

[[nodiscard]] constexpr std::string_view to_string(
    TextResourceDialect dialect) noexcept {
    switch (dialect) {
    case TextResourceDialect::clt: return "clt";
    case TextResourceDialect::tsc: return "tsc";
    case TextResourceDialect::effect_manifest: return "effect-manifest";
    case TextResourceDialect::unknown: return "txt";
    }
    return "txt";
}

class TextResourceDialects final {
public:
    /// How far in a dialect marker may sit. All observed markers are at 0 or 2.
    static constexpr std::size_t k_probe_bytes = 64U;

    [[nodiscard]] static TextResourceIdentity identify(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::profiles::dmc3
