#include "dmc_rengine/stageops/domain_queries.hpp"

#include "dmc_rengine/stageops/domain_source_span.hpp"

#include <algorithm>

namespace dmc::rengine::stageops {

std::vector<const StageDomainObject*> domain_objects_by_kind_source_order(
    const StageDomainWorkspace& workspace,
    StageDomainKind kind) {
    auto result = workspace.by_kind(kind);
    std::sort(
        result.begin(), result.end(),
        [](const StageDomainObject* left, const StageDomainObject* right) {
            const auto left_span = domain_source_span(*left);
            const auto right_span = domain_source_span(*right);
            if (left_span.has_value() && right_span.has_value()) {
                if (left_span->resource_id != right_span->resource_id) {
                    return left_span->resource_id < right_span->resource_id;
                }
                if (left_span->offset != right_span->offset) {
                    return left_span->offset < right_span->offset;
                }
                if (left_span->size != right_span->size) {
                    return left_span->size < right_span->size;
                }
            } else if (left_span.has_value() != right_span.has_value()) {
                // Byte-addressable objects are surfaced first for editor/source
                // navigation. Non-addressable aggregates remain deterministic.
                return left_span.has_value();
            }
            return left->id < right->id;
        });
    return result;
}

} // namespace dmc::rengine::stageops
