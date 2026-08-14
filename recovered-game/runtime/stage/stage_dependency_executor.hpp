#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::recovered::dmc3::runtime::stage {

struct EnemyResourceSelection final {
    std::uint32_t external_enemy_id{};
    std::uint32_t class_selector{};
    std::uint32_t resource_set_selector{};
};

class IEnemyResourceResolver {
public:
    virtual ~IEnemyResourceResolver() = default;

    [[nodiscard]] virtual std::optional<EnemyResourceSelection> resolve(
        std::uint32_t external_enemy_id) = 0;
};

class IStageDependencySink {
public:
    virtual ~IStageDependencySink() = default;

    [[nodiscard]] virtual bool preload_enemy_object(
        const EnemyResourceSelection& selection) = 0;
    [[nodiscard]] virtual bool preload_enemy_sound(
        const EnemyResourceSelection& selection) = 0;
    [[nodiscard]] virtual bool request_stage_script() = 0;
    [[nodiscard]] virtual bool request_stage_effect() = 0;
    [[nodiscard]] virtual bool wait_pending_dependencies() = 0;
};

enum class StageDependencyExecutionStatus {
    complete,
    unresolved_enemy_id,
    backend_rejected,
};

struct StageDependencyExecutionReport final {
    StageDependencyExecutionStatus status{
        StageDependencyExecutionStatus::complete};
    std::vector<EnemyResourceSelection> unique_resource_sets;
    std::optional<std::uint32_t> unresolved_enemy_id;

    [[nodiscard]] bool complete() const noexcept {
        return status == StageDependencyExecutionStatus::complete;
    }
};

class StageDependencyExecutor final {
public:
    // This executable slice begins after stage-config records have yielded enemy
    // IDs. The exact binary cfg-record parser is still a reverse target and is
    // deliberately not invented here.
    //
    // Recovered relative ordering implemented here:
    //   enemy IDs -> resource-set mapping -> dedupe -> enemy object+sound preload
    //   -> stage script -> stage effect -> wait pending dependencies.
    [[nodiscard]] static StageDependencyExecutionReport execute_after_config_scan(
        std::span<const std::uint32_t> enemy_ids,
        IEnemyResourceResolver& resolver,
        IStageDependencySink& sink) {
        StageDependencyExecutionReport report{};

        for (const auto enemy_id : enemy_ids) {
            const auto resolved = resolver.resolve(enemy_id);
            if (!resolved.has_value()) {
                report.status =
                    StageDependencyExecutionStatus::unresolved_enemy_id;
                report.unresolved_enemy_id = enemy_id;
                return report;
            }

            const auto duplicate = std::find_if(
                report.unique_resource_sets.begin(),
                report.unique_resource_sets.end(),
                [&resolved](const EnemyResourceSelection& existing) {
                    return existing.resource_set_selector ==
                        resolved->resource_set_selector;
                });
            if (duplicate == report.unique_resource_sets.end()) {
                report.unique_resource_sets.push_back(*resolved);
            }
        }

        for (const auto& selection : report.unique_resource_sets) {
            if (!sink.preload_enemy_object(selection) ||
                !sink.preload_enemy_sound(selection)) {
                report.status = StageDependencyExecutionStatus::backend_rejected;
                return report;
            }
        }

        if (!sink.request_stage_script() ||
            !sink.request_stage_effect() ||
            !sink.wait_pending_dependencies()) {
            report.status = StageDependencyExecutionStatus::backend_rejected;
            return report;
        }

        report.status = StageDependencyExecutionStatus::complete;
        return report;
    }
};

} // namespace dmc::recovered::dmc3::runtime::stage
