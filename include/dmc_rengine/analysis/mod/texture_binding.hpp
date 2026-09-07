#pragma once

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/model_texture_companion.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::analysis::mod {

struct OutOfRangeTextureSlot final {
    std::size_t outer_index{};
    std::size_t mesh_index{};
    std::uint16_t texture_slot{};
};

// Cross-resource validation result for serialized MOD texture references.
// Runtime texture-table authority comes from the external companion; the MOD
// header +0x12 value is retained only as a serialized mirror/consistency signal.
struct TextureBindingAnalysis final {
    bool companion_valid{false};
    std::uint8_t header_texture_slot_count{};
    std::uint32_t companion_texture_count{};
    bool header_mirror_matches_companion{false};
    std::size_t mesh_count{};
    std::vector<OutOfRangeTextureSlot> out_of_range_meshes;

    [[nodiscard]] bool runtime_bindings_valid() const noexcept {
        return companion_valid && out_of_range_meshes.empty();
    }
};

[[nodiscard]] inline TextureBindingAnalysis analyze_texture_binding(
    const dmc::rengine::formats::mod::Document& document,
    const dmc::rengine::formats::model_family::TextureCompanionParseResult&
        companion) {
    TextureBindingAnalysis out;
    out.companion_valid = companion.ok();
    out.header_texture_slot_count = document.header.texture_slot_count;
    out.companion_texture_count = companion.texture_count;
    out.header_mirror_matches_companion =
        companion.ok() &&
        static_cast<std::uint32_t>(document.header.texture_slot_count) ==
            companion.texture_count;

    for (std::size_t outer_index = 0U;
         outer_index < document.outer_models.size();
         ++outer_index) {
        const auto& outer = document.outer_models[outer_index];
        for (std::size_t mesh_index = 0U;
             mesh_index < outer.meshes.size();
             ++mesh_index) {
            ++out.mesh_count;
            if (!companion.ok()) {
                continue;
            }

            const auto texture_slot = outer.meshes[mesh_index].texture_slot;
            if (static_cast<std::uint32_t>(texture_slot) >=
                companion.texture_count) {
                out.out_of_range_meshes.push_back(OutOfRangeTextureSlot{
                    .outer_index = outer_index,
                    .mesh_index = mesh_index,
                    .texture_slot = texture_slot,
                });
            }
        }
    }

    return out;
}

} // namespace dmc::rengine::analysis::mod
