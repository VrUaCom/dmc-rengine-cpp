#include "dmc_rengine/gdspaces/effect_pack_names.hpp"

#include "dmc_rengine/formats/effect_pack.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"

namespace dmc::rengine::gdspaces {

bool is_effect_pack(std::span<const std::byte> bytes) noexcept {
    return formats::EffectPackParser::structurally_valid(bytes);
}

std::vector<SlotNameAttribution> effect_pack_slot_names(
    std::span<const std::byte> effect_pack_bytes,
    std::uint32_t expanded_slot_index) {
    using Contract = profiles::dmc3::EffectPackContract;

    // The manifest names the record slot and nothing else. Asked about the
    // manifest's own slot, or any other, the answer is nothing — not the list
    // shifted by one.
    if (expanded_slot_index !=
        static_cast<std::uint32_t>(Contract::records_slot_index)) {
        return {};
    }

    const auto parsed = formats::EffectPackParser::parse(effect_pack_bytes);
    if (!parsed.ok()) {
        return {};
    }

    std::vector<SlotNameAttribution> names;
    names.reserve(parsed.document->records.size());
    for (const auto& record : parsed.document->records) {
        names.push_back(SlotNameAttribution{
            .slot_index = record.slot_index,
            .name = record.name,
            .origin = SlotNameOrigin::container_manifest,
            // The corroboration available here is not an extension matching a
            // format — a manifest line carries no extension. It is that the
            // record's extent is the one its kind carries in both corpus
            // files. A kind whose extent varies corroborates nothing, and says
            // so rather than borrowing confidence from its neighbours.
            .corroborated_by_payload = record.extent_matches_kind,
        });
    }
    return names;
}

} // namespace dmc::rengine::gdspaces
