#pragma once

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cstdint>
#include <span>

namespace dmc::rengine::profiles::dmc3 {

// Read-side variants accepted in addition to the canonical full-mip framing.
// These names describe only observed physical encodings. They do not assign
// legacy engine semantics to the enclosing .ptx/.tm2 resource names.
enum class TextureSlotReadVariant : std::uint8_t {
    canonical,
    legacy_single_mip_bundle_dxt5,
    legacy_single_mip_wrapped_dxt5,
};

struct TextureSlotFramingReadResult final {
    TextureSlotFramingResult framing;
    TextureSlotReadVariant variant{TextureSlotReadVariant::canonical};

    [[nodiscard]] bool ok() const noexcept {
        return framing.ok();
    }

    [[nodiscard]] bool compatibility_used() const noexcept {
        return variant != TextureSlotReadVariant::canonical;
    }
};

// Reader union for physical texture resources found in retail DMC3 GData.
//
// TextureSlotFramingParser remains the strict/canonical contract used by
// authoring and reflow code. This reader first delegates to that parser and
// only then admits narrowly bounded legacy single-level DXT5 layouts observed
// in hash-bound retail samples. Keeping the compatibility boundary read-only
// avoids silently granting writer authority to encodings we have not rebuilt
// and accepted in the canonical executable yet.
class TextureSlotFramingReader final {
public:
    [[nodiscard]] static TextureSlotFramingReadResult parse(
        std::span<const std::byte> bytes,
        TextureSlotFramingSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
