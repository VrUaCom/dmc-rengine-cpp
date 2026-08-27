#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

namespace dmc::rengine::profiles::dmc3 {

class TextureSlotExpander final {
public:
    // Expands an evidence-backed DMC3 wrapped-DDS or texture-bundle physical
    // slot into virtual DDS children. Child identity is physical and stable for
    // a layout-preserving image: parent resource + TEXTURE[index] + exact DDS
    // byte span. Presentation/index labels are intentionally not used here.
    [[nodiscard]] static gdspaces::ContainerExpansion expand(
        const gdspaces::ResourcePayload& parent,
        TextureSlotFramingSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
