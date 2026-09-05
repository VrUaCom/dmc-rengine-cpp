#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace dmc::rengine::formats::ptx {

inline constexpr std::size_t bundle_header_size =
    profiles::dmc3::TextureSlotFramingParser::k_bundle_header_size;
inline constexpr std::size_t descriptor_size =
    profiles::dmc3::TextureSlotFramingParser::k_descriptor_size;
inline constexpr std::size_t sector_size =
    profiles::dmc3::TextureSlotFramingParser::k_sector_size;

struct ScanResult final {
    bool recognized{false};
    profiles::dmc3::TextureSlotFramingResult framing;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Reader final {
public:
    // Native structural PTX reader. The implementation deliberately reuses
    // the evidence-backed DMC3 texture-slot framing parser and accepts only
    // the multi-texture bundle framing as PTX. Descriptor+single-DDS framing
    // remains a texture-slot representation, not a PTX bundle identity.
    [[nodiscard]] static ScanResult scan(
        std::span<const std::byte> bytes,
        profiles::dmc3::TextureSlotFramingSafety safety = {});

    // Materialize validated DDS children with stable byte provenance. This is
    // the modular Native Reader entry point over the existing canonical
    // TextureSlotExpander backend.
    [[nodiscard]] static gdspaces::ContainerExpansion expand_dds_children(
        const gdspaces::ResourcePayload& parent,
        profiles::dmc3::TextureSlotFramingSafety safety = {});
};

} // namespace dmc::rengine::formats::ptx
