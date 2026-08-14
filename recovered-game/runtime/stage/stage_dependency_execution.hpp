#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::recovered::dmc3::runtime::stage {

// Executable Wave-3 reconstruction candidate. The broad dependency relationship
// is supported by the current reverse, but remains provisional until its raw
// research authority is reconciled/promoted in-repo. Keeping that status in the
// type name prevents this code from being mistaken for canonical game equivalence.
struct EnemyResourceSelection final {
    std::uint32_t external_enemy_id{};
    std::uint32_t class_selector{};
    std::uint32_t resource_set_selector{};
};

class IEnemyResourceSelectionResolver {
public:
    virtual ~IEnemyResourceSelectionResolver() = default;

    [[nodiscard]] virtual std::optional<EnemyResourceSelection> resolve(
        std::uint32_t external_enemy_id) = 0;
};

class IStageDependencyExecutionSink {
public:
    virtual ~IStageDependencyExecutionSink() = default;

    // The reverse currently establishes that each selected enemy resource set
    // contributes both object/model and sound dependencies before stage core
    // resources finish. Their lower-level call/interleave ABI remains a separate
    // target, so this boundary intentionally treats the pair atomically.
    [[nodiscard]] virtual bool preload_enemy_resource_pair(
        const EnemyResourceSelection& selection) = 0;

    // Script and effect are both scheduled after enemy dependency discovery.
    // Their exact lower-level call ordering is not encoded here.
    [[nodiscard]] virtual bool request_stage_core_resources() = 0;

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

class StageDependencyExecutionCandidate final {
public:
    // Input begins AFTER cfg-record parsing has already produced external enemy
    // IDs. The exact cfg binary record ABI is still unresolved and is therefore
    // not fabricated by this executable slice.
    [[nodiscard]] static StageDependencyExecutionReport execute_after_config_scan(
        std::span<const std::uint32_t> enemy_ids,
        IEnemyResourceSelectionResolver& resolver,
        IStageDependencyExecutionSink& sink) {
        StageDependencyExecutionReport report{};

        // Resolve first and fail before side effects if any ID is still unknown.
        for (const auto enemy_id : enemy_ids) {
            const auto selection = resolver.resolve(enemy_id);
            if (!selection.has_value()) {
                report.status =
                    StageDependencyExecutionStatus::unresolved_enemy_id;
                report.unresolved_enemy_id = enemy_id;
                return report;
            }

            const auto duplicate = std::find_if(
                report.unique_resource_sets.begin(),
                report.unique_resource_sets.end(),
                [&selection](const EnemyResourceSelection& existing) {
                    return existing.resource_set_selector ==
                        selection->resource_set_selector;
                });
            if (duplicate == report.unique_resource_sets.end()) {
                report.unique_resource_sets.push_back(*selection);
            }
        }

        for (const auto& selection : report.unique_resource_sets) {
            if (!sink.preload_enemy_resource_pair(selection)) {
                report.status = StageDependencyExecutionStatus::backend_rejected;
                return report;
            }
        }

        if (!sink.request_stage_core_resources() ||
            !sink.wait_pending_dependencies()) {
            report.status = StageDependencyExecutionStatus::backend_rejected;
            return report;
        }

        report.status = StageDependencyExecutionStatus::complete;
        return report;
    }
};

} // namespace dmc::recovered::dmc3::runtime::stage
