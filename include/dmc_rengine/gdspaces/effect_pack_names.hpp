#pragma once

#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::gdspaces {

// The names an effect container stores for its own records.
//
// `*_effect.pac` is the one container in this game that writes its slot names
// down: a two-slot `PNST` whose first slot is an ASCII manifest and whose
// second holds exactly one record per manifest line. Everywhere else a slot
// name is a decision this tool made.
//
// The awkward part is where those names live. They are in the *outer*
// container, and they name the slots of the *inner* one — so the expander,
// which sees one container at a time, cannot reach them from the container it
// is expanding. The caller that holds the outer bytes has to bring them.
//
// This is that bridge, and it deliberately refuses everything else: it returns
// an empty list unless the bytes really are an effect pack and the slot being
// expanded really is the record slot. A near miss gets nothing rather than a
// plausible-looking name list attached to the wrong container.
[[nodiscard]] std::vector<SlotNameAttribution> effect_pack_slot_names(
    std::span<const std::byte> effect_pack_bytes,
    std::uint32_t expanded_slot_index);

// True where these bytes are the outer container of an effect pack. Offered so
// a caller can ask the cheap question before materializing anything else.
[[nodiscard]] bool is_effect_pack(std::span<const std::byte> bytes) noexcept;

} // namespace dmc::rengine::gdspaces
