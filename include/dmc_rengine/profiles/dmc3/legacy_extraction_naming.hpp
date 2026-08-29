#pragma once

#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace dmc::rengine::profiles::dmc3 {

enum class LegacyExtractionRepresentation : std::uint8_t {
    unavailable,
    file,
    expanded_directory,
};

// DMC3 HDC legacy-extractor representation derived only from exact external
// .index evidence. This is a replay/presentation plan, never a physical locator.
struct LegacyExtractionNamingPlan final {
    gdspaces::ResourceId resource_id;
    std::uint32_t physical_slot_index{};
    std::optional<std::size_t> extracted_ordinal;

    LegacyExtractionRepresentation representation{
        LegacyExtractionRepresentation::unavailable};

    // Exact parent-manifest line and normalized extraction leaf/directory name.
    std::optional<std::string> manifest_entry_raw;
    std::optional<std::string> extraction_name;

    // For `S_NNN folder`, retained corpus convention is S_NNN/S_NNN.index.
    std::optional<std::string> nested_index_name;

    // Separate namespace: e.g. st001.ptx. Not claimed to be the historical
    // extracted directory name and not used as a write/slot locator.
    std::optional<std::string> embedded_semantic_alias;

    std::string canonical_display_name;
    bool exact_from_external_index{false};

    [[nodiscard]] bool valid() const noexcept;
};

class LegacyExtractionNamingPlanner final {
public:
    [[nodiscard]] static LegacyExtractionNamingPlan build(
        const gdspaces::ResourceNamingIdentity& identity);
};

} // namespace dmc::rengine::profiles::dmc3
