#include "runtime/resources/resource_lifecycle.hpp"
#include "runtime/resources/resource_manager.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace {

class RecordingPostLoadBackend final
    : public dmc::recovered::dmc3::runtime::resources::IConfirmedPostLoadBackend {
public:
    bool succeed{true};
    std::size_t call_count{};
    std::optional<dmc::recovered::dmc3::runtime::resources::PostLoadFormat>
        last_format;

    [[nodiscard]] bool apply(
        dmc::recovered::dmc3::runtime::resources::PostLoadFormat format,
        std::span<std::byte> bytes) override {
        ++call_count;
        last_format = format;
        if (bytes.size() > 3U) {
            bytes[3] = std::byte{0x7F};
        }
        return succeed;
    }
};

[[nodiscard]] std::array<std::byte, 8> make_magic(
    char a,
    char b,
    char c) {
    return {
        static_cast<std::byte>(a),
        static_cast<std::byte>(b),
        static_cast<std::byte>(c),
        std::byte{0},
        std::byte{1},
        std::byte{2},
        std::byte{3},
        std::byte{4},
    };
}

} // namespace

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

    // The recovered entry now has the exact known x64 ABI, not merely a layout
    // description stored beside an unrelated C++ object.
    static_assert(sizeof(ResourceRuntimeEntry) == 0x48U);
    static_assert(offsetof(ResourceRuntimeEntry, group_index) == 0x00U);
    static_assert(offsetof(ResourceRuntimeEntry, state) == 0x04U);
    static_assert(offsetof(ResourceRuntimeEntry, subtype_index) == 0x08U);
    static_assert(offsetof(ResourceRuntimeEntry, source_descriptor) == 0x18U);
    static_assert(offsetof(ResourceRuntimeEntry, loaded_payload) == 0x20U);
    static_assert(offsetof(ResourceRuntimeEntry, owned_state) == 0x28U);

    ResourceRuntimeManager manager;
    assert(manager.group_for_slot(0U) == 0U);
    assert(manager.group_for_slot(3U) == 0U);
    assert(manager.group_for_slot(4U) == 1U);
    assert(manager.group_for_slot(139U) == 1U);
    assert(manager.group_for_slot(140U) == 2U);
    assert(manager.group_for_slot(199U) == 2U);
    assert(manager.group_for_slot(200U) == 3U);
    assert(manager.group_for_slot(227U) == 3U);
    assert(manager.group_for_slot(228U) == 4U);
    assert(manager.group_for_slot(229U) == 5U);
    assert(manager.group_for_slot(356U) == 5U);
    assert(manager.group_for_slot(357U) == 6U);
    assert(manager.group_for_slot(362U) == 6U);
    assert(!manager.group_for_slot(363U).has_value());

    // Execute the confirmed successful lifecycle on one real pool slot:
    // 0 -> 1 -> 2 -> typed post-load -> 3.
    constexpr std::size_t enemy_object_slot = 229U;
    assert(manager.start_loading(
        enemy_object_slot, 12U, static_cast<std::uintptr_t>(0x11110000ULL)) ==
        ResourceTransitionResult::applied);
    auto* entry = manager.entry(enemy_object_slot);
    assert(entry != nullptr);
    assert(entry->state == ResourceLoadState::io_scheduled_or_loading);
    assert(entry->subtype_index == 12U);
    assert(entry->source_descriptor == 0x11110000ULL);

    // A second 0->1 start is rejected instead of silently rewriting an active slot.
    assert(manager.start_loading(enemy_object_slot, 13U, 0x22220000ULL) ==
        ResourceTransitionResult::wrong_state);

    // A null payload cannot advance state 1 -> 2.
    assert(manager.mark_io_complete(enemy_object_slot, 0U) ==
        ResourceTransitionResult::payload_missing);
    assert(entry->state == ResourceLoadState::io_scheduled_or_loading);

    assert(manager.mark_io_complete(
        enemy_object_slot,
        static_cast<std::uintptr_t>(0x33330000ULL),
        static_cast<std::uintptr_t>(0x44440000ULL)) ==
        ResourceTransitionResult::applied);
    assert(entry->state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(entry->loaded_payload == 0x33330000ULL);
    assert(entry->owned_state == 0x44440000ULL);

    auto mod_bytes = make_magic('M', 'O', 'D');
    RecordingPostLoadBackend backend;
    backend.succeed = false;
    assert(manager.run_confirmed_postload(
        enemy_object_slot, mod_bytes, backend) ==
        ResourceTransitionResult::postload_failed);
    assert(entry->state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(backend.call_count == 1U);
    assert(backend.last_format == PostLoadFormat::mod);

    backend.succeed = true;
    assert(manager.run_confirmed_postload(
        enemy_object_slot, mod_bytes, backend) ==
        ResourceTransitionResult::applied);
    assert(entry->state == ResourceLoadState::ready_postprocessed);
    assert(backend.call_count == 2U);
    assert(mod_bytes[3] == std::byte{0x7F});

    // The same post-load cannot be applied twice after state 3 is reached.
    assert(manager.run_confirmed_postload(
        enemy_object_slot, mod_bytes, backend) ==
        ResourceTransitionResult::wrong_state);

    // Unknown formats are fail-closed and do not promote state 2 to READY.
    constexpr std::size_t enemy_sound_slot = 357U;
    assert(manager.start_loading(enemy_sound_slot, 2U, 0x55550000ULL) ==
        ResourceTransitionResult::applied);
    assert(manager.mark_io_complete(enemy_sound_slot, 0x66660000ULL) ==
        ResourceTransitionResult::applied);
    auto unknown_bytes = make_magic('X', 'Y', 'Z');
    assert(manager.run_confirmed_postload(
        enemy_sound_slot, unknown_bytes, backend) ==
        ResourceTransitionResult::unsupported_postload_format);
    const auto* sound_entry = manager.entry(enemy_sound_slot);
    assert(sound_entry != nullptr);
    assert(sound_entry->state ==
        ResourceLoadState::io_complete_pending_postprocess);

    // Magic dispatch is executable for all four directly confirmed fixup families.
    auto efm_bytes = make_magic('E', 'F', 'M');
    auto scm_bytes = make_magic('S', 'C', 'M');
    auto shw_bytes = make_magic('S', 'H', 'W');
    assert(confirmed_postload_format(efm_bytes) == PostLoadFormat::efm);
    assert(confirmed_postload_format(scm_bytes) == PostLoadFormat::scm);
    assert(confirmed_postload_format(shw_bytes) == PostLoadFormat::shw);
    assert(!confirmed_postload_format(unknown_bytes).has_value());

    // State-4 entry is intentionally not synthesized by the manager because the
    // complete source-state domain is still unresolved. The directly observed
    // cleanup edge itself is executable and independently tested.
    ResourceRuntimeEntry teardown_entry{};
    teardown_entry.group_index = 6U;
    teardown_entry.state = ResourceLoadState::teardown_or_cancel_pending;
    teardown_entry.loaded_payload = 0x77770000ULL;
    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::applied);
    assert(teardown_entry.state == ResourceLoadState::free_or_unstarted);
    // Other fields are deliberately not cleared because that destructor/cleanup
    // behavior has not yet been recovered.
    assert(teardown_entry.loaded_payload == 0x77770000ULL);

    assert(manager.start_loading(363U, 0U, 0U) ==
        ResourceTransitionResult::slot_out_of_range);

    return 0;
}
