#include "runtime/resources/resource_lifecycle.hpp"

#include <algorithm>
#include <limits>

namespace dmc::recovered::dmc3::runtime::resources {

bool ResourceEntryLayout::valid() const noexcept {
    return stride == 0x48U &&
        group_index_offset == 0x00U &&
        state_offset == 0x04U &&
        subtype_index_offset == 0x08U &&
        source_descriptor_offset == 0x18U &&
        loaded_payload_offset == 0x20U &&
        owned_state_offset == 0x28U &&
        owned_state_offset < stride;
}

bool ResourcePoolLayout::valid() const noexcept {
    if (base_va != 0x140C99D30ULL || entry_count != 363U || !entry.valid()) {
        return false;
    }

    std::uint32_t cursor = 0U;
    for (std::size_t index = 0U; index < groups.size(); ++index) {
        const auto& current = groups[index];
        if (current.index != index || current.first_entry != cursor ||
            current.entry_count == 0U ||
            current.entry_count > std::numeric_limits<std::uint32_t>::max() - cursor) {
            return false;
        }
        cursor += current.entry_count;
    }
    return cursor == entry_count;
}

const ResourcePoolGroup* ResourcePoolLayout::group(
    std::uint32_t index) const noexcept {
    const auto iterator = std::find_if(
        groups.begin(), groups.end(),
        [index](const ResourcePoolGroup& candidate) {
            return candidate.index == index;
        });
    return iterator == groups.end() ? nullptr : &*iterator;
}

const PostLoadFixup* ResourceRuntimeEvidenceModel::fixup(
    PostLoadFormat format) const noexcept {
    const auto iterator = std::find_if(
        typed_fixups.begin(), typed_fixups.end(),
        [format](const PostLoadFixup& candidate) {
            return candidate.format == format;
        });
    return iterator == typed_fixups.end() ? nullptr : &*iterator;
}

bool ResourceRuntimeEvidenceModel::valid() const noexcept {
    if (artifact_sha256 !=
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082" ||
        evidence_packet_id != "dmc3-stage-wave2" || !pool.valid() ||
        successful_state_chain != std::array<ResourceLoadState, 4>{
            ResourceLoadState::free_or_unstarted,
            ResourceLoadState::io_scheduled_or_loading,
            ResourceLoadState::io_complete_pending_postprocess,
            ResourceLoadState::ready_postprocessed,
        }) {
        return false;
    }

    const std::array<std::uint64_t, 4> expected_vas{
        0x1402FE3B0ULL,
        0x1402F7A90ULL,
        0x1403051B0ULL,
        0x1403204C0ULL,
    };
    for (std::size_t index = 0U; index < typed_fixups.size(); ++index) {
        if (typed_fixups[index].function_va != expected_vas[index] ||
            typed_fixups[index].symbol.empty()) {
            return false;
        }
    }
    return true;
}

const ResourceRuntimeEvidenceModel&
wave2_resource_runtime_model() noexcept {
    static const ResourceRuntimeEvidenceModel model{
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .evidence_packet_id = "dmc3-stage-wave2",
        .pool = ResourcePoolLayout{
            .base_va = 0x140C99D30ULL,
            .entry_count = 363U,
            .entry = ResourceEntryLayout{
                .stride = 0x48U,
                .group_index_offset = 0x00U,
                .state_offset = 0x04U,
                .subtype_index_offset = 0x08U,
                .source_descriptor_offset = 0x18U,
                .loaded_payload_offset = 0x20U,
                .owned_state_offset = 0x28U,
            },
            .groups = {
                ResourcePoolGroup{.index = 0U, .first_entry = 0U, .entry_count = 4U},
                ResourcePoolGroup{.index = 1U, .first_entry = 4U, .entry_count = 136U},
                ResourcePoolGroup{.index = 2U, .first_entry = 140U, .entry_count = 60U},
                ResourcePoolGroup{.index = 3U, .first_entry = 200U, .entry_count = 28U},
                ResourcePoolGroup{.index = 4U, .first_entry = 228U, .entry_count = 1U},
                ResourcePoolGroup{.index = 5U, .first_entry = 229U, .entry_count = 128U},
                ResourcePoolGroup{.index = 6U, .first_entry = 357U, .entry_count = 6U},
            },
        },
        .typed_fixups = {
            PostLoadFixup{
                .format = PostLoadFormat::mod,
                .function_va = 0x1402FE3B0ULL,
                .symbol = "MOD_postload_fixup",
            },
            PostLoadFixup{
                .format = PostLoadFormat::efm,
                .function_va = 0x1402F7A90ULL,
                .symbol = "EFM_postload_fixup",
            },
            PostLoadFixup{
                .format = PostLoadFormat::scm,
                .function_va = 0x1403051B0ULL,
                .symbol = "SCM_postload_fixup",
            },
            PostLoadFixup{
                .format = PostLoadFormat::shw,
                .function_va = 0x1403204C0ULL,
                .symbol = "SHW_postload_fixup",
            },
        },
        .successful_state_chain = {
            ResourceLoadState::free_or_unstarted,
            ResourceLoadState::io_scheduled_or_loading,
            ResourceLoadState::io_complete_pending_postprocess,
            ResourceLoadState::ready_postprocessed,
        },
    };
    return model;
}

} // namespace dmc::recovered::dmc3::runtime::resources
