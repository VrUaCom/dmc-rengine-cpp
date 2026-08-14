#include "runtime/resources/resource_lifecycle.hpp"
#include "runtime/resources/resource_manager.hpp"
#include "recovered_dmc3_wave3_execution_cases.hpp"

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

    static_assert(sizeof(ResourceRuntimeEntry) == 0x48U);
    static_assert(offsetof(ResourceRuntimeEntry, group_index) == 0x00U);
    static_assert(offsetof(ResourceRuntimeEntry, state) == 0x04U);
    static_assert(offsetof(ResourceRuntimeEntry, subtype_index) == 0x08U);
    static_assert(offsetof(ResourceRuntimeEntry, source_descriptor) == 0x18U);
    static_assert(offsetof(ResourceRuntimeEntry, loaded_payload) == 0x20U);
    static_assert(offsetof(ResourceRuntimeEntry, owned_state) == 0x28U);

    // Exercise only transitions directly supported by Wave-2 evidence. Known
    // non-state fields are seeded independently to prove that these transition
    // helpers do not invent writer ownership or field-clearing behavior.
    ResourceRuntimeEntry entry{};
    entry.group_index = 5U;
    entry.subtype_index = 12U;
    entry.source_descriptor = 0x11110000ULL;
    entry.loaded_payload = 0x33330000ULL;
    entry.owned_state = 0x44440000ULL;

    assert(transition_free_to_loading(entry) ==
        ResourceTransitionResult::applied);
    assert(entry.state == ResourceLoadState::io_scheduled_or_loading);
    assert(entry.subtype_index == 12U);
    assert(entry.source_descriptor == 0x11110000ULL);
    assert(entry.loaded_payload == 0x33330000ULL);
    assert(entry.owned_state == 0x44440000ULL);

    assert(transition_free_to_loading(entry) ==
        ResourceTransitionResult::wrong_state);

    assert(transition_loading_to_io_complete(entry) ==
        ResourceTransitionResult::applied);
    assert(entry.state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(entry.loaded_payload == 0x33330000ULL);
    assert(entry.owned_state == 0x44440000ULL);

    std::array<std::byte, 8> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{0},
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4},
    };
    RecordingPostLoadBackend backend;
    backend.succeed = false;
    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::postload_failed);
    assert(entry.state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(backend.call_count == 1U);
    assert(backend.last_format == PostLoadFormat::mod);

    backend.succeed = true;
    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::applied);
    assert(entry.state == ResourceLoadState::ready_postprocessed);
    assert(backend.call_count == 2U);
    assert(bytes[3] == std::byte{0x7F});

    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::wrong_state);

    // All four confirmed helpers can cross the same explicit typed-postload
    // boundary. This does not claim how the original dispatcher selected them.
    constexpr std::array<PostLoadFormat, 4> formats{
        PostLoadFormat::mod,
        PostLoadFormat::efm,
        PostLoadFormat::scm,
        PostLoadFormat::shw,
    };
    for (const auto format : formats) {
        ResourceRuntimeEntry typed{};
        typed.state = ResourceLoadState::io_complete_pending_postprocess;
        assert(run_confirmed_postload(typed, format, bytes, backend) ==
            ResourceTransitionResult::applied);
        assert(typed.state == ResourceLoadState::ready_postprocessed);
    }

    // State-4 entry is intentionally seeded rather than synthesized because the
    // source-state domain entering teardown remains unresolved.
    ResourceRuntimeEntry teardown_entry{};
    teardown_entry.group_index = 6U;
    teardown_entry.state = ResourceLoadState::teardown_or_cancel_pending;
    teardown_entry.loaded_payload = 0x77770000ULL;
    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::applied);
    assert(teardown_entry.state == ResourceLoadState::free_or_unstarted);
    assert(teardown_entry.loaded_payload == 0x77770000ULL);

    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::wrong_state);

    // Wave-3 executable slices are compiled and executed by this already
    // registered recovered-runtime CTest target.
    dmc::recovered::dmc3::tests::wave3::run();

    return 0;
}
