#pragma once

#include "runtime/media/hd_media_translation.hpp"
#include "runtime/scenes/scene_transition_spine.hpp"
#include "runtime/stage/numeric_stage_resolver.hpp"
#include "runtime/stage/stage_dependency_execution.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace dmc::recovered::dmc3::tests::wave3 {

class FixtureEnemyResolver final
    : public runtime::stage::IEnemyResourceSelectionResolver {
public:
    [[nodiscard]] std::optional<runtime::stage::EnemyResourceSelection>
    resolve(std::uint32_t external_enemy_id) override {
        using runtime::stage::EnemyResourceSelection;
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

class RecordingDependencySink final
    : public runtime::stage::IStageDependencyExecutionSink {
public:
    std::vector<std::uint32_t> events;
    std::optional<std::uint32_t> reject_event;

    [[nodiscard]] bool preload_enemy_resource_pair(
        const runtime::stage::EnemyResourceSelection& selection) override {
        return record(1000U + selection.resource_set_selector);
    }

    [[nodiscard]] bool request_stage_core_resources() override {
        return record(2000U);
    }

    [[nodiscard]] bool wait_pending_dependencies() override {
        return record(3000U);
    }

private:
    [[nodiscard]] bool record(std::uint32_t event) {
        events.push_back(event);
        return !reject_event.has_value() || *reject_event != event;
    }
};

class FixtureAddressSpace final : public runtime::stage::IReadOnlyAddressSpace {
public:
    void write_u64(std::uint64_t address, std::uint64_t value) {
        values_[address] = value;
    }

    [[nodiscard]] std::optional<std::uint64_t> read_u64(
        std::uint64_t virtual_address) const override {
        const auto iterator = values_.find(virtual_address);
        if (iterator == values_.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }

private:
    std::unordered_map<std::uint64_t, std::uint64_t> values_;
};

class RecordingScene final : public runtime::scenes::IScene {
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

class RecordingSceneFactory final : public runtime::scenes::ISceneFactory {
public:
    explicit RecordingSceneFactory(
        std::vector<std::uint32_t>& events) noexcept
        : events_{events} {}

    [[nodiscard]] std::unique_ptr<runtime::scenes::IScene>
    create(runtime::scenes::SceneId id) override {
        const auto numeric = static_cast<std::uint32_t>(id);
        events_.push_back(400U + numeric);
        return std::make_unique<RecordingScene>(numeric, events_);
    }

private:
    std::vector<std::uint32_t>& events_;
};

inline void run() {
    using namespace runtime::stage;

    // Directly recovered numeric Stage algorithm. The fixture is only a memory
    // provider; the division, group lookup, selector lookup and bank/row
    // identification are executed by recovered-game code.
    FixtureAddressSpace address_space;
    constexpr std::uint64_t group3_selector_base = 0x140700000ULL;
    address_space.write_u64(
        stage_group_base_table_va + 3ULL * 8ULL,
        group3_selector_base);
    address_space.write_u64(
        group3_selector_base + 8ULL * 8ULL,
        stage_bank_b.base_va + 0ULL * stage_descriptor_stride);

    const auto stage308 = NumericStageResolver::resolve(308U, address_space);
    assert(stage308.resolved());
    assert(stage308.group_index == 3U);
    assert(stage308.remainder == 8U);
    assert(stage308.selector_base_va == group3_selector_base);
    assert(stage308.selector_slot_va == group3_selector_base + 8ULL * 8ULL);
    assert(stage308.descriptor_va == stage_bank_b.base_va);
    assert(stage308.bank == StageDescriptorBankId::bank_b);
    assert(stage308.source_row == 0U);

    // Group outside the ten recovered hundreds entries fails without memory I/O.
    const auto stage1000 = NumericStageResolver::resolve(1000U, address_space);
    assert(stage1000.status == NumericStageResolutionStatus::group_out_of_range);

    // Missing selector slot is fail-closed.
    address_space.write_u64(
        stage_group_base_table_va + 4ULL * 8ULL,
        0x140710000ULL);
    const auto stage400 = NumericStageResolver::resolve(400U, address_space);
    assert(stage400.status ==
        NumericStageResolutionStatus::unreadable_selector_slot);

    // Descriptor must belong exactly to one of the two recovered banks.
    constexpr std::uint64_t group9_selector_base = 0x140720000ULL;
    address_space.write_u64(
        stage_group_base_table_va + 9ULL * 8ULL,
        group9_selector_base);
    address_space.write_u64(group9_selector_base, 0x140123456ULL);
    const auto stage900 = NumericStageResolver::resolve(900U, address_space);
    assert(stage900.status ==
        NumericStageResolutionStatus::descriptor_outside_recovered_banks);

    // Provisional Wave-3 dependency execution begins after cfg enemy-ID
    // extraction and deliberately keeps lower-level object/sound and
    // script/effect sub-ordering outside the recovered contract.
    FixtureEnemyResolver enemy_resolver;
    RecordingDependencySink dependency_sink;
    constexpr std::array<std::uint32_t, 4> enemy_ids{10U, 11U, 20U, 10U};
    const auto dependency_report =
        StageDependencyExecutionCandidate::execute_after_config_scan(
            enemy_ids, enemy_resolver, dependency_sink);
    assert(dependency_report.complete());
    assert(dependency_report.unique_resource_sets.size() == 2U);
    assert(dependency_report.unique_resource_sets[0].resource_set_selector == 12U);
    assert(dependency_report.unique_resource_sets[1].resource_set_selector == 20U);
    assert(dependency_report.unique_resource_sets[0].external_enemy_id == 10U);

    const std::vector<std::uint32_t> expected_dependency_events{
        1012U, 1020U, 2000U, 3000U,
    };
    assert(dependency_sink.events == expected_dependency_events);

    RecordingDependencySink unresolved_sink;
    constexpr std::array<std::uint32_t, 2> unresolved_ids{10U, 99U};
    const auto unresolved_report =
        StageDependencyExecutionCandidate::execute_after_config_scan(
            unresolved_ids, enemy_resolver, unresolved_sink);
    assert(unresolved_report.status ==
        StageDependencyExecutionStatus::unresolved_enemy_id);
    assert(unresolved_report.unresolved_enemy_id == 99U);
    assert(unresolved_sink.events.empty());

    RecordingDependencySink rejecting_sink;
    rejecting_sink.reject_event = 1020U;
    const auto rejected_report =
        StageDependencyExecutionCandidate::execute_after_config_scan(
            enemy_ids, enemy_resolver, rejecting_sink);
    assert(rejected_report.status ==
        StageDependencyExecutionStatus::backend_rejected);
    const std::vector<std::uint32_t> expected_rejected_events{1012U, 1020U};
    assert(rejecting_sink.events == expected_rejected_events);

    using namespace runtime::media;

    const std::array<HdAudioDescriptor, 2> audio_descriptors{{
        HdAudioDescriptor{
            .physical_name = "battle_01.ogg",
            .loop_start_ms = 15253U,
            .loop_end_ms = 117653U,
        },
        HdAudioDescriptor{
            .physical_name = "system_00.ogg",
            .loop_start_ms = no_explicit_loop,
            .loop_end_ms = no_explicit_loop,
        },
    }};

    const auto ogg_key = make_ogg_lookup_key("afs\\sound\\BATTLE_01.ADX");
    assert(ogg_key.has_value());
    assert(*ogg_key == "battle_01.ogg");

    const auto* audio = find_hd_audio_descriptor(
        "afs\\sound\\BATTLE_01.ADX", audio_descriptors);
    assert(audio != nullptr);
    const auto loop = explicit_loop_points(*audio);
    assert(loop.has_value());
    assert(loop->start_ms == 15253U);
    assert(loop->end_ms == 117653U);
    assert(!explicit_loop_points(audio_descriptors[1]).has_value());
    assert(find_hd_audio_descriptor(
        "afs\\sound\\missing.adx", audio_descriptors) == nullptr);
    assert(!make_ogg_lookup_key("no-extension").has_value());

    const auto wmv = rewrite_legacy_video_to_wmv("Video\\m14_s00.sfd");
    assert(wmv.has_value());
    assert(*wmv == "Video\\m14_s00.wmv");
    assert(!rewrite_legacy_video_to_wmv("Video\\no-extension").has_value());

    using namespace runtime::scenes;

    std::vector<std::uint32_t> scene_events;
    RecordingSceneFactory scene_factory{scene_events};
    SceneTransitionSpine scenes{scene_factory};

    assert(scenes.transition_to(SceneId::boot) ==
        SceneTransitionStatus::applied);
    assert(scenes.current_id() == SceneId::boot);
    const std::vector<std::uint32_t> expected_boot_events{400U, 100U};
    assert(scene_events == expected_boot_events);

    assert(scenes.transition_to(SceneId::game) ==
        SceneTransitionStatus::applied);
    assert(scenes.current_id() == SceneId::game);
    const std::vector<std::uint32_t> expected_game_events{
        400U, 100U,
        200U, 300U,
        404U, 104U,
    };
    assert(scene_events == expected_game_events);
}

} // namespace dmc::recovered::dmc3::tests::wave3
