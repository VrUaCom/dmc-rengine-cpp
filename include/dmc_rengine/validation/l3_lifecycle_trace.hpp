#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::validation {

enum class L3LifecycleScope {
    v1_initial_load,
    v2_transition,
    v3_restart_reload,
    v4_full_reset,
    v5_inflight_cancellation,
    v6_shutdown,
};

enum class L3LifecycleStatus {
    captured,
    aborted,
};

enum class L3LifecycleEventKind {
    resource_request,
    materialization_submit,
    state_write,
    materialization_complete,
    typed_postload_enter,
    typed_postload_exit,
    ready_callback,
    consumer_visible,
    loader_claim_inc,
    loader_claim_dec,
    zero_claim_release,
    cancel_mark,
    quiescence_wait,
    quiescence_reached,
    group_reset,
    full_reset,
    backing_release,
    scene_boundary,
    shutdown_boundary,
};

struct L3LifecycleAuthority final {
    std::string exe_sha256;
    std::uint64_t exe_size{};
    std::string role;
};

struct L3LifecycleResourceIdentity final {
    std::string logical_identity;
    std::string selected_provider_identity;
    std::string materialized_sha256;
    std::uint64_t materialized_size{};
    std::string materialized_provenance;
};

struct L3LifecycleObserver final {
    std::string name;
    std::string version;
    std::string config_sha256;
    std::uint64_t dropped_events{};
    bool overflow_detected{};
    bool semantic_intrusion_detected{};
};

struct L3LifecycleRun final {
    std::string id;
    bool original_process{};
    bool completed_cleanly{};
    bool overlay_published{};
    bool rollback_verified{};
    std::optional<std::string> overlay_sha256;
};

struct L3LifecycleEvent final {
    std::uint64_t sequence{};
    L3LifecycleEventKind kind{L3LifecycleEventKind::resource_request};
    std::optional<std::uint64_t> record_identity;
    std::optional<std::uint32_t> group;
    std::optional<std::uint32_t> index;
    std::optional<std::uint32_t> state_from;
    std::optional<std::uint32_t> state_to;
    std::string writer;
    std::map<std::string, std::uint64_t, std::less<>> raw_numeric;
};

struct L3LifecycleTrace final {
    std::string schema;
    L3LifecycleScope scope{L3LifecycleScope::v1_initial_load};
    L3LifecycleStatus status{L3LifecycleStatus::aborted};
    L3LifecycleAuthority authority;
    L3LifecycleResourceIdentity resource;
    L3LifecycleObserver observer;
    L3LifecycleRun run;
    std::vector<std::string> family_tags;
    std::vector<L3LifecycleEvent> events;
};

struct L3LifecycleDiagnostic final {
    std::string path;
    std::string message;
};

struct L3LifecycleImportLimits final {
    std::size_t max_input_bytes{16U * 1024U * 1024U};
    std::size_t max_events{100000U};
    std::size_t max_family_tags{64U};
    std::size_t max_raw_numeric_fields{64U};
    std::size_t max_id_bytes{512U};
    std::size_t max_writer_bytes{512U};
};

struct L3LifecycleImportResult final {
    std::optional<L3LifecycleTrace> trace;
    std::vector<L3LifecycleDiagnostic> diagnostics;
    bool promotion_eligible{};

    [[nodiscard]] bool ok() const noexcept {
        return trace.has_value() && diagnostics.empty();
    }
};

[[nodiscard]] std::string_view to_string(L3LifecycleScope scope) noexcept;
[[nodiscard]] std::string_view to_string(L3LifecycleStatus status) noexcept;
[[nodiscard]] std::string_view to_string(L3LifecycleEventKind kind) noexcept;

[[nodiscard]] L3LifecycleImportResult l3_lifecycle_trace_from_json(
    std::string_view json_text,
    L3LifecycleImportLimits limits = {});

} // namespace dmc::rengine::validation
