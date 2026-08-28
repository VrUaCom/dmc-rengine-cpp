#include "dmc_rengine/gdspaces/container_index_probe.hpp"

#include "dmc_rengine/formats/effect_pack.hpp"
#include "dmc_rengine/formats/relative_slot_container.hpp"
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <optional>

namespace dmc::rengine::gdspaces {
namespace {

using Walk = profiles::dmc3::RelativeSlotWalkContract;
using EffectContract = profiles::dmc3::EffectPackContract;

[[nodiscard]] formats::RelativeSlotContainerSpec spec_for(
    std::span<const std::byte> bytes) {
    formats::RelativeSlotContainerSpec spec;
    const auto magic = bytes.size() >= 4U &&
            std::to_integer<char>(bytes[1]) == 'N'
        ? Walk::pnst_magic
        : Walk::pac_magic;
    for (std::size_t index = 0U; index < magic.size(); ++index) {
        spec.magic[index] = static_cast<std::byte>(magic[index]);
    }
    spec.magic_bytes = magic.size();
    spec.document_format = magic == Walk::pnst_magic ? "pnst" : "pac";
    return spec;
}

} // namespace

ContainerIndexProbeResult ContainerIndexProbe::probe(
    std::span<const std::byte> container_bytes) {
    ContainerIndexProbeResult result;
    if (container_bytes.size() < Walk::offset_table_offset) {
        return result;
    }

    // The effect dialect first, because it is the stricter test: it requires
    // the whole container to be a two-slot pack whose line count equals its
    // sibling's populated slot count. A container that satisfies that is not
    // going to be mistaken for anything else.
    if (formats::EffectPackParser::structurally_valid(container_bytes)) {
        const auto parsed = formats::EffectPackParser::parse(container_bytes);
        if (parsed.ok()) {
            result.dialect = ContainerIndexDialect::kind_and_identifier;
            result.index_slot_index =
                static_cast<std::uint32_t>(EffectContract::manifest_slot_index);
            result.named_slot_count = parsed.document->record_slot_count;
            result.names_a_sibling_container = true;
            result.named_sibling_slot_index =
                static_cast<std::uint32_t>(EffectContract::records_slot_index);
            return result;
        }
    }

    const auto parsed = parse_relative_slot_container(
        container_bytes, spec_for(container_bytes));
    if (!parsed.ok() || parsed.document->entries.empty()) {
        return result;
    }
    const auto& first = parsed.document->entries.front();
    if (first.slot_index != SlotNameManifest::k_manifest_slot ||
        !first.populated) {
        return result;
    }
    const auto total = static_cast<std::uint64_t>(container_bytes.size());
    if (first.offset > total || first.size > total - first.offset) {
        return result;
    }

    const auto names = SlotNameManifest::parse(container_bytes.subspan(
        static_cast<std::size_t>(first.offset),
        static_cast<std::size_t>(first.size)));
    if (names.empty()) {
        // Text that names nothing is not an index. `# END`, `# GAME` and
        // `# DOOR 0` all land here, and they are scene and config blocks.
        return result;
    }

    result.dialect = ContainerIndexDialect::filename_list;
    result.index_slot_index = SlotNameManifest::k_manifest_slot;
    result.named_slot_count = static_cast<std::uint32_t>(names.size());
    result.names_a_sibling_container = false;
    return result;
}

} // namespace dmc::rengine::gdspaces
