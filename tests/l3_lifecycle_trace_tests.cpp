#include "dmc_rengine/validation/l3_lifecycle_trace.hpp"

#include <cassert>
#include <cstdint>
#include <string>

namespace {

constexpr const char* kSha =
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
constexpr const char* kMaterializedSha =
    "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
constexpr const char* kConfigSha =
    "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";

[[nodiscard]] std::string prefix(
    std::string scope,
    bool original_process,
    bool completed_cleanly = true,
    std::uint64_t dropped_events = 0U,
    bool overflow = false,
    bool intrusion = false) {
    return std::string{R"json({
  "schema": "dmc-rengine.gdspaces-l3-lifecycle-trace.v1",
  "scope": ")json"} + scope + R"json(",
  "status": "captured",
  "authority": {
    "exe_sha256": ")json" + kSha + R"json(",
    "exe_size": 6356432,
    "role": "synthetic-unit-test-authority"
  },
  "resource": {
    "logical_identity": "unit/test/resource.pac",
    "selected_provider_identity": "archive:DMC3-0.nbz/member:unit/test/resource.pac",
    "materialized_sha256": ")json" + kMaterializedSha + R"json(",
    "materialized_size": 4096,
    "materialized_provenance": "unit-test-l1-receipt"
  },
  "observer": {
    "name": "unit-test-observer",
    "version": "1",
    "config_sha256": ")json" + kConfigSha + R"json(",
    "dropped_events": )json" + std::to_string(dropped_events) + R"json(,
    "overflow_detected": )json" + (overflow ? "true" : "false") + R"json(,
    "semantic_intrusion_detected": )json" + (intrusion ? "true" : "false") + R"json(
  },
  "run": {
    "id": "synthetic-unit-test-run",
    "original_process": )json" + (original_process ? "true" : "false") + R"json(,
    "completed_cleanly": )json" + (completed_cleanly ? "true" : "false") + R"json(,
    "overlay_published": false,
    "rollback_verified": false
  },
  "family_tags": ["synthetic-test"],
  "events": [
)json";
}

[[nodiscard]] std::string suffix() {
    return R"json(
  ]
})json";
}

[[nodiscard]] std::string v1_trace(bool original_process) {
    return prefix("V1", original_process) + R"json(
    {"sequence": 10, "kind": "resource_request"},
    {"sequence": 20, "kind": "materialization_submit"},
    {"sequence": 30, "kind": "state_write", "state_from": 0, "state_to": 1, "writer": "0x1401B84E0"},
    {"sequence": 40, "kind": "materialization_complete"},
    {"sequence": 50, "kind": "state_write", "state_from": 1, "state_to": 2, "writer": "0x1401B8DC0"},
    {"sequence": 60, "kind": "typed_postload_enter"},
    {"sequence": 70, "kind": "typed_postload_exit"},
    {"sequence": 80, "kind": "ready_callback"},
    {"sequence": 90, "kind": "state_write", "state_from": 2, "state_to": 3, "writer": "0x1401B92D0"},
    {"sequence": 100, "kind": "consumer_visible"}
)json" + suffix();
}

[[nodiscard]] std::string v5_trace(bool include_ready_to_cancel = false) {
    auto events = std::string{R"json(
    {"sequence": 10, "kind": "cancel_mark"},
    {"sequence": 20, "kind": "state_write", "state_from": 2, "state_to": 4, "writer": "0x1401B8430"},
)json"};
    if (include_ready_to_cancel) {
        events += R"json(    {"sequence": 25, "kind": "state_write", "state_from": 3, "state_to": 4, "writer": "invalid-test-writer"},
)json";
    }
    events += R"json(    {"sequence": 30, "kind": "quiescence_wait"},
    {"sequence": 40, "kind": "state_write", "state_from": 4, "state_to": 0, "writer": "0x1401B8F00"},
    {"sequence": 50, "kind": "quiescence_reached"}
)json";
    return prefix("V5", false) + events + suffix();
}

} // namespace

int main() {
    using dmc::rengine::validation::L3LifecycleScope;
    using dmc::rengine::validation::l3_lifecycle_trace_from_json;

    // Synthetic structural success is explicitly non-promotable.
    {
        const auto result = l3_lifecycle_trace_from_json(v1_trace(false));
        assert(result.ok());
        assert(result.trace->scope == L3LifecycleScope::v1_initial_load);
        assert(!result.promotion_eligible.content_candidate());
        assert(!result.promotion_eligible.eligible());
        assert(result.trace->events.size() == 10U);
    }

    // Even a syntactically perfect self-asserted original_process=true JSON can
    // only become a content candidate. Trusted origin is deliberately impossible
    // to manufacture through this importer.
    {
        const auto result = l3_lifecycle_trace_from_json(v1_trace(true));
        assert(result.ok());
        assert(result.promotion_eligible.content_candidate());
        assert(!result.promotion_eligible.eligible());
    }

    // Ready callback must stay between typed post-load and state-3 publication.
    {
        auto invalid = v1_trace(false);
        const auto callback = invalid.find(
            "{\"sequence\": 80, \"kind\": \"ready_callback\"},");
        assert(callback != std::string::npos);
        const auto line_end = invalid.find('\n', callback);
        invalid.erase(callback, line_end - callback + 1U);
        const auto visible = invalid.find(
            "{\"sequence\": 100, \"kind\": \"consumer_visible\"}");
        assert(visible != std::string::npos);
        invalid.insert(
            visible,
            "{\"sequence\": 95, \"kind\": \"ready_callback\"},\n    ");
        const auto result = l3_lifecycle_trace_from_json(invalid);
        assert(!result.ok());
    }

    // Captured original-process traces with loss are invalid, not merely
    // non-promotable.
    {
        auto invalid = prefix("V1", true, true, 1U) + R"json(
    {"sequence": 10, "kind": "resource_request"}
)json" + suffix();
        const auto result = l3_lifecycle_trace_from_json(invalid);
        assert(!result.ok());
    }

    // Canonical V5 cancellation accepts state 1/2 -> 4 -> 0 plus quiescence.
    {
        const auto result = l3_lifecycle_trace_from_json(v5_trace(false));
        assert(result.ok());
        assert(result.trace->scope == L3LifecycleScope::v5_inflight_cancellation);
        assert(!result.promotion_eligible.content_candidate());
        assert(!result.promotion_eligible.eligible());
    }

    // The recovered canonical cancellation writer does not mark ready state 3 as 4.
    {
        const auto result = l3_lifecycle_trace_from_json(v5_trace(true));
        assert(!result.ok());
    }

    // Aborted traces remain valid diagnostic artifacts even without a complete
    // scenario sequence, but can never become Level-E promotion evidence.
    {
        auto aborted = prefix("V1", true);
        const auto status = aborted.find("\"status\": \"captured\"");
        assert(status != std::string::npos);
        aborted.replace(status, std::string{"\"status\": \"captured\""}.size(),
                        "\"status\": \"aborted\"");
        aborted += R"json(    {"sequence": 10, "kind": "resource_request"}
)json";
        aborted += suffix();
        const auto result = l3_lifecycle_trace_from_json(aborted);
        assert(result.ok());
        assert(!result.promotion_eligible.content_candidate());
        assert(!result.promotion_eligible.eligible());
    }

    // V7 is intentionally not a single-trace scope. Breadth is aggregated across
    // accepted V1-V6 family-tagged receipts during final audit.
    {
        auto invalid = v1_trace(false);
        const auto scope = invalid.find("\"scope\": \"V1\"");
        assert(scope != std::string::npos);
        invalid.replace(scope, std::string{"\"scope\": \"V1\""}.size(),
                        "\"scope\": \"V7\"");
        const auto result = l3_lifecycle_trace_from_json(invalid);
        assert(!result.ok());
    }

    return 0;
}
