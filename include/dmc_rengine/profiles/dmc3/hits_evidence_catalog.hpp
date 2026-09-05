#pragma once

// Canonical DMC3 HITS reverse-evidence aggregation.
//
// Keep this layer separate from formats/hits.hpp: the format parser is shared
// and address-free, while this catalog is tied to one verified DMC3 HD build.

#include "dmc_rengine/profiles/dmc3/hits_collision_triplet.hpp"
#include "dmc_rengine/profiles/dmc3/hits_contact_normal_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_dynamic_update_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_primitive_descriptor_ownership_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_primitive_shape_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_primitive_type01_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_query_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_source1_query_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/hits_stage_cfg_collision_tables.hpp"
#include "dmc_rengine/profiles/dmc3/hits_stage_cfg_pac_evidence.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_catalog {

inline constexpr std::string_view canonical_executable_sha256 =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

// Decompilation depth is independent from evidence confidence. A DL number
// answers "how deep is the recovered model?" rather than "how certain is it?".
enum class DecompilationLayer : std::uint8_t {
    raw_binary = 0,
    machine_decode = 1,
    abi_layout_type = 2,
    local_algorithm = 3,
    object_ownership = 4,
    subsystem_architecture = 5,
    runtime_integration = 6,
    game_domain_semantics = 7,
    in_game_behavioral_parity = 8,
};

struct Coverage final {
    DecompilationLayer format_layout{DecompilationLayer::abi_layout_type};
    DecompilationLayer static_query_pipeline{DecompilationLayer::runtime_integration};
    DecompilationLayer dynamic_query_pipeline{DecompilationLayer::runtime_integration};
    DecompilationLayer stage_cfg_binding{DecompilationLayer::runtime_integration};
    DecompilationLayer flag_semantics{DecompilationLayer::abi_layout_type};
    DecompilationLayer original_game_writer_acceptance{DecompilationLayer::runtime_integration};
};

[[nodiscard]] constexpr Coverage current_coverage() noexcept {
    return {};
}

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return hits_evidence::matches_canonical_target(sha256) &&
           hits_stage_cfg_collision_tables::matches_canonical_target(sha256);
}

[[nodiscard]] inline bool canonical_catalog_consistent() noexcept {
    if (!matches_canonical_target(canonical_executable_sha256)) {
        return false;
    }

    const auto combined = hits_evidence::combined_query_abi_evidence(
        canonical_executable_sha256);
    if (!combined || combined->function_va != 0x14005E7A0ULL ||
        combined->argument_count != 6U || !combined->returns_any_hit_in_al) {
        return false;
    }

    const auto source0 = hits_evidence::wrapper_source_evidence(
        canonical_executable_sha256,
        hits_evidence::WrapperSourceIndex::stage_member_3);
    const auto source1 = hits_evidence::wrapper_source_evidence(
        canonical_executable_sha256,
        hits_evidence::WrapperSourceIndex::stage_member_6);
    if (!source0 || !source1 || !source0->stage_pac_backed_confirmed ||
        !source1->stage_pac_backed_confirmed) {
        return false;
    }

    const auto modern = hits_stage_cfg_collision_tables::slots_for_generation(
        hits_stage_cfg_collision_tables::SlotGeneration::modern_cem_stage_cfg);
    const auto legacy = hits_stage_cfg_collision_tables::slots_for_generation(
        hits_stage_cfg_collision_tables::SlotGeneration::legacy_cem008_stage_cfg);
    return modern.entry_table_slot == 39U &&
           modern.primitive_descriptor_table_slot == 40U &&
           legacy.entry_table_slot == 22U &&
           legacy.primitive_descriptor_table_slot == 23U;
}

} // namespace dmc::rengine::profiles::dmc3::hits_catalog
