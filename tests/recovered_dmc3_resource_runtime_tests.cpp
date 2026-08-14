#include "runtime/media/hd_media_translation.hpp"
#include "runtime/resources/resource_lifecycle.hpp"
#include "runtime/resources/resource_manager.hpp"
#include "runtime/scenes/scene_manager.hpp"
#include "runtime/stage/stage_dependency_executor.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

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

class FixtureEnemyResolver final
    : public dmc::recovered::dmc3::runtime::stage::IEnemyResourceResolver {
public:
    [[nodiscard]] std::optional<
        dmc::recovered::dmc3::runtime::stage::EnemyResourceSelection>
    resolve(std::uint32_t external_enemy_id) override {
        using dmc::recovered::dmc3::runtime::stage::EnemyResourceSelection;
        switch (external_enemy_id) {
        case 10U:
            return EnemyResourceSelection{
                .external_enemy_id = 10U,
                .class_selector = 1U,
                .resource_set_selector = 12U,
            };
        case 11U:
            return EnemyResourceSelection{
                .external_enemy_id = 11U,
                .class_selector = 2U,
                .resource_set_selector = 12U,
            };
        case 20U:
            return EnemyResourceSelection{
                .external_enemy_id = 20U,
                .class_selector = 3U,
                .resource_set_selector = 20U,
            };
        default:
            return std::nullopt;
        }
    }
};

class RecordingStageDependencySink final
    : public dmc::recovered::dmc3::runtime::stage::IStageDependencySink {
public:
    std::vector<std::uint32_t> events;
    std::optional<std::uint32_t> reject_event;

    [[nodiscard]] bool preload_enemy_object(
        const dmc::recovered::dmc3::runtime::stage::EnemyResourceSelection&
            selection) override {
        return record(1000U + selection.resource_set_selector);
    }

    [[nodiscard]] bool preload_enemy_sound(
        const dmc::recovered::dmc3::runtime::stage::EnemyResourceSelection&
            selection) override {
        return record(2000U + selection.resource_set_selector);
    }

    [[nodiscard]] bool request_stage_script() override {
        return record(3000U);
    }

    [[nodiscard]] bool request_stage_effect() override {
        return record(3001U);
    }

    [[nodiscard]] bool wait_pending_dependencies() override {
        return record(3002U);
    }

private:
    [[nodiscard]] bool record(std::uint32_t event) {
        events.push_back(event);
        return !reject_event.has_value() || *reject_event != event;
    }
};

class RecordingScene final
    : public dmc::recovered::dmc3::runtime::scenes::IScene {
public:
    RecordingScene(
        std::uint32_t id,
        std::vector<std::uint32_t>& events) noexcept
        : id_{id}, events_{events} {}

    ~RecordingScene() override {
        events_.push_back(300U + id_);
    }

    void enter() override {
        events_.push_back(100U + id_);
    }

    void exit() override {
        events_.push_back(200U + id_);
    }

private:
    std::uint32_t id_{};
    std::vector<std::uint32_t>& events_;
};

class RecordingSceneFactory final
    : public dmc::recovered::dmc3::runtime::scenes::ISceneFactory {
public:
    explicit RecordingSceneFactory(
        std::vector<std::uint32_t>& events) noexcept
        : events_{events} {}

    std::optional<dmc::recovered::dmc3::runtime::scenes::SceneId> reject_id;

    [[nodiscard]] std::unique_ptr<
        dmc::recovered::dmc3::runtime::scenes::IScene>
    create(dmc::recovered::dmc3::runtime::scenes::SceneId id) override {
        const auto numeric = static_cast<std::uint32_t>(id);
        events_.push_back(400U + numeric);
        if (reject_id.has_value() && *reject_id == id) {
            return nullptr;
        }
        return std::make_unique<RecordingScene>(numeric, events_);
    }

private:
    std::vector<std::uint32_t>& events_;
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

    constexpr std::size_t enemy_object_slot = 229U;
    assert(manager.start_loading(
        enemy_object_slot, 12U, static_cast<std::uintptr_t>(0x11110000ULL)) ==
        ResourceTransitionResult::applied);
    auto* entry = manager.entry(enemy_object_slot);
    assert(entry != nullptr);
    assert(entry->state == ResourceLoadState::io_scheduled_or_loading);
    assert(entry->subtype_index == 12U);
    assert(entry->source_descriptor == 0x11110000ULL);

    assert(manager.start_loading(enemy_object_slot, 13U, 0x22220000ULL) ==
        ResourceTransitionResult::wrong_state);

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

    assert(manager.run_confirmed_postload(
        enemy_object_slot, mod_bytes, backend) ==
        ResourceTransitionResult::wrong_state);

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

    auto efm_bytes = make_magic('E', 'F', 'M');
    auto scm_bytes = make_magic('S', 'C', 'M');
    auto shw_bytes = make_magic('S', 'H', 'W');
    assert(confirmed_postload_format(efm_bytes) == PostLoadFormat::efm);
    assert(confirmed_postload_format(scm_bytes) == PostLoadFormat::scm);
    assert(confirmed_postload_format(shw_bytes) == PostLoadFormat::shw);
    assert(!confirmed_postload_format(unknown_bytes).has_value());

    ResourceRuntimeEntry teardown_entry{};
    teardown_entry.group_index = 6U;
    teardown_entry.state = ResourceLoadState::teardown_or_cancel_pending;
    teardown_entry.loaded_payload = 0x77770000ULL;
    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::applied);
    assert(teardown_entry.state == ResourceLoadState::free_or_unstarted);
    assert(teardown_entry.loaded_payload == 0x77770000ULL);

    assert(manager.start_loading(363U, 0U, 0U) ==
        ResourceTransitionResult::slot_out_of_range);

    using namespace dmc::recovered::dmc3::runtime::stage;
    FixtureEnemyResolver enemy_resolver;
    RecordingStageDependencySink dependency_sink;
    constexpr std::array<std::uint32_t, 4> enemy_ids{10U, 11U, 20U, 10U};
    const auto dependency_report = StageDependencyExecutor::execute_after_config_scan(
        enemy_ids, enemy_resolver, dependency_sink);
    assert(dependency_report.complete());
    assert(dependency_report.unique_resource_sets.size() == 2U);
    assert(dependency_report.unique_resource_sets[0].resource_set_selector == 12U);
    assert(dependency_report.unique_resource_sets[1].resource_set_selector == 20U);
    const std::vector<std::uint32_t> expected_dependency_events{
        1012U, 2012U, 1020U, 2020U, 3000U, 3001U, 3002U,
    };
    assert(dependency_sink.events == expected_dependency_events);
    assert(dependency_report.unique_resource_sets[0].external_enemy_id == 10U);

    RecordingStageDependencySink unresolved_sink;
    constexpr std::array<std::uint32_t, 2> unresolved_ids{10U, 99U};
    const auto unresolved_report = StageDependencyExecutor::execute_after_config_scan(
        unresolved_ids, enemy_resolver, unresolved_sink);
    assert(unresolved_report.status ==
        StageDependencyExecutionStatus::unresolved_enemy_id);
    assert(unresolved_report.unresolved_enemy_id == 99U);
    assert(unresolved_sink.events.empty());

    RecordingStageDependencySink rejecting_sink;
    rejecting_sink.reject_event = 2012U;
    const auto rejected_report = StageDependencyExecutor::execute_after_config_scan(
        enemy_ids, enemy_resolver, rejecting_sink);
    assert(rejected_report.status ==
        StageDependencyExecutionStatus::backend_rejected);
    const std::vector<std::uint32_t> expected_rejected_events{1012U, 2012U};
    assert(rejecting_sink.events == expected_rejected_events);

    // Directly recovered HD-port translation behavior now executes in the
    // recovered tree rather than existing only as catalog observations.
    using namespace dmc::recovered::dmc3::runtime::media;
    const std::array<HdAudioDescriptor, 2> audio_descriptors{{
        HdAudioDescriptor{
            .physical_name = "battle_01.ogg",
            .loop_start_ms = 15253U,
            .loop_end_ms = 117653U,
        },
        HdAudioDescriptor{
            .physical_name = "system_00.ogg",
            .loop_start_ms = kNoExplicitLoop,
            .loop_end_ms = kNoExplicitLoop,
        },
    }};
    const auto ogg_key = make_ogg_lookup_key("afs\\sound\\BATTLE_01.ADX");
    assert(ogg_key.has_value());
    assert(*ogg_key == "battle_01.ogg");
    const auto* audio = find_hd_audio_descriptor(
        "afs\\sound\\BATTLE_01.ADX", audio_descriptors);
    assert(audio != nullptr);
    assert(audio->has_explicit_loop());
    assert(audio->loop_start_ms == 15253U);
    assert(audio->loop_end_ms == 117653U);
    assert(!audio_descriptors[1].has_explicit_loop());
    assert(find_hd_audio_descriptor(
        "afs\\sound\\missing.adx", audio_descriptors) == nullptr);
    assert(!make_ogg_lookup_key("no-extension").has_value());

    const auto wmv = rewrite_sfd_to_wmv("Video\\m14_s00.sfd");
    assert(wmv.has_value());
    assert(*wmv == "Video\\m14_s00.wmv");
    assert(!rewrite_sfd_to_wmv("Video\\no-extension").has_value());

    // Recovered scene manager executes the observed transition order:
    // exit current -> destroy current -> create replacement -> enter replacement.
    using namespace dmc::recovered::dmc3::runtime::scenes;
    std::vector<std::uint32_t> scene_events;
    RecordingSceneFactory scene_factory{scene_events};
    SceneManager scene_manager{scene_factory};
    assert(scene_manager.transition_to(SceneId::boot) ==
        SceneTransitionResult::applied);
    assert(scene_manager.current_id() == SceneId::boot);
    const std::vector<std::uint32_t> expected_boot_events{400U, 100U};
    assert(scene_events == expected_boot_events);

    assert(scene_manager.transition_to(SceneId::game) ==
        SceneTransitionResult::applied);
    assert(scene_manager.current_id() == SceneId::game);
    const std::vector<std::uint32_t> expected_game_events{
        400U, 100U,
        200U, 300U,
        404U, 104U,
    };
    assert(scene_events == expected_game_events);

    scene_factory.reject_id = SceneId::result;
    assert(scene_manager.transition_to(SceneId::result) ==
        SceneTransitionResult::factory_failed);
    assert(!scene_manager.has_scene());
    assert(!scene_manager.current_id().has_value());
    const std::vector<std::uint32_t> expected_failed_transition_events{
        400U, 100U,
        200U, 300U,
        404U, 104U,
        204U, 304U,
        408U,
    };
    assert(scene_events == expected_failed_transition_events);

    return 0;
}
