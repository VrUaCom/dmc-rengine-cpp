#include "runtime/resources/resource_lifecycle.hpp"

#include <array>
#include <cassert>
#include <cstdint>

int main() {
    using namespace dmc::recovered::dmc3::runtime::resources;

    const auto& model = wave2_resource_runtime_model();
    assert(model.valid());
    assert(model.artifact_sha256 ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(model.evidence_packet_id == "dmc3-stage-wave2");

    const auto& pool = model.pool;
    assert(pool.valid());
    assert(pool.base_va == 0x140C99D30ULL);
    assert(pool.entry_count == 363U);
    assert(pool.entry.stride == 0x48U);
    assert(pool.entry.group_index_offset == 0x00U);
    assert(pool.entry.state_offset == 0x04U);
    assert(pool.entry.subtype_index_offset == 0x08U);
    assert(pool.entry.source_descriptor_offset == 0x18U);
    assert(pool.entry.loaded_payload_offset == 0x20U);
    assert(pool.entry.owned_state_offset == 0x28U);

    constexpr std::array<std::uint32_t, 7> counts{
        4U, 136U, 60U, 28U, 1U, 128U, 6U,
    };
    constexpr std::array<std::uint32_t, 7> offsets{
        0U, 4U, 140U, 200U, 228U, 229U, 357U,
    };
    for (std::uint32_t index = 0U; index < counts.size(); ++index) {
        const auto* group = pool.group(index);
        assert(group != nullptr);
        assert(group->first_entry == offsets[index]);
        assert(group->entry_count == counts[index]);
    }
    assert(pool.group(7U) == nullptr);
    assert(pool.groups.back().end_entry() == 363U);

    constexpr std::array<ResourceLoadState, 4> expected_state_chain{
        ResourceLoadState::free_or_unstarted,
        ResourceLoadState::io_scheduled_or_loading,
        ResourceLoadState::io_complete_pending_postprocess,
        ResourceLoadState::ready_postprocessed,
    };
    assert(model.successful_state_chain == expected_state_chain);
    assert(to_string(ResourceLoadState::teardown_or_cancel_pending) ==
        "teardown-or-cancel-pending");

    assert(model.cleanup_transition.from ==
        ResourceLoadState::teardown_or_cancel_pending);
    assert(model.cleanup_transition.to ==
        ResourceLoadState::free_or_unstarted);
    assert(!model.teardown_source_state_domain_complete);

    const auto* mod = model.fixup(PostLoadFormat::mod);
    const auto* efm = model.fixup(PostLoadFormat::efm);
    const auto* scm = model.fixup(PostLoadFormat::scm);
    const auto* shw = model.fixup(PostLoadFormat::shw);
    assert(mod != nullptr && mod->function_va == 0x1402FE3B0ULL);
    assert(efm != nullptr && efm->function_va == 0x1402F7A90ULL);
    assert(scm != nullptr && scm->function_va == 0x1403051B0ULL);
    assert(shw != nullptr && shw->function_va == 0x1403204C0ULL);
    assert(to_string(mod->format) == "MOD");
    assert(to_string(efm->format) == "EFM");
    assert(to_string(scm->format) == "SCM");
    assert(to_string(shw->format) == "SHW");

    // Deliberate evidence boundary: this model does not invent the unknown
    // source-state domain for transitions into state 4, cache/refcount rules,
    // callback ABI, or actual pointer-fixup implementation.
    return 0;
}
