#include "dmc_rengine/validation/l3_lifecycle_trace.hpp"

#include "dmc_rengine/core/json.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::validation {
namespace {

constexpr std::string_view kSchema =
    "dmc-rengine.gdspaces-l3-lifecycle-trace.v1";

[[nodiscard]] bool is_lower_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return (ch >= static_cast<unsigned char>('0') &&
                ch <= static_cast<unsigned char>('9')) ||
               (ch >= static_cast<unsigned char>('a') &&
                ch <= static_cast<unsigned char>('f'));
    });
}

[[nodiscard]] std::optional<L3LifecycleScope> scope_from_string(
    std::string_view value) noexcept {
    if (value == "V1") return L3LifecycleScope::v1_initial_load;
    if (value == "V2") return L3LifecycleScope::v2_transition;
    if (value == "V3") return L3LifecycleScope::v3_restart_reload;
    if (value == "V4") return L3LifecycleScope::v4_full_reset;
    if (value == "V5") return L3LifecycleScope::v5_inflight_cancellation;
    if (value == "V6") return L3LifecycleScope::v6_shutdown;
    return std::nullopt;
}

[[nodiscard]] std::optional<L3LifecycleStatus> status_from_string(
    std::string_view value) noexcept {
    if (value == "captured") return L3LifecycleStatus::captured;
    if (value == "aborted") return L3LifecycleStatus::aborted;
    return std::nullopt;
}

[[nodiscard]] std::optional<L3LifecycleEventKind> event_kind_from_string(
    std::string_view value) noexcept {
    using Kind = L3LifecycleEventKind;
    if (value == "resource_request") return Kind::resource_request;
    if (value == "materialization_submit") return Kind::materialization_submit;
    if (value == "state_write") return Kind::state_write;
    if (value == "materialization_complete") return Kind::materialization_complete;
    if (value == "typed_postload_enter") return Kind::typed_postload_enter;
    if (value == "typed_postload_exit") return Kind::typed_postload_exit;
    if (value == "ready_callback") return Kind::ready_callback;
    if (value == "consumer_visible") return Kind::consumer_visible;
    if (value == "loader_claim_inc") return Kind::loader_claim_inc;
    if (value == "loader_claim_dec") return Kind::loader_claim_dec;
    if (value == "zero_claim_release") return Kind::zero_claim_release;
    if (value == "cancel_mark") return Kind::cancel_mark;
    if (value == "quiescence_wait") return Kind::quiescence_wait;
    if (value == "quiescence_reached") return Kind::quiescence_reached;
    if (value == "group_reset") return Kind::group_reset;
    if (value == "full_reset") return Kind::full_reset;
    if (value == "backing_release") return Kind::backing_release;
    if (value == "scene_boundary") return Kind::scene_boundary;
    if (value == "shutdown_boundary") return Kind::shutdown_boundary;
    return std::nullopt;
}

class L3LifecycleImporter final {
public:
    L3LifecycleImporter(
        const core::json::Value& root,
        L3LifecycleImportLimits limits)
        : root_(root), limits_(limits) {}

    [[nodiscard]] L3LifecycleImportResult run() {
        L3LifecycleImportResult result;
        const auto* root_object = root_.as_object();
        if (root_object == nullptr) {
            fail("$", "Lifecycle trace root must be a JSON object.");
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        validate_keys(
            *root_object,
            {"schema", "scope", "status", "authority", "resource", "observer",
             "run", "family_tags", "events"},
            "$");

        L3LifecycleTrace trace;
        const auto schema = required_string(*root_object, "schema", "$", 128U);
        const auto scope_text = required_string(*root_object, "scope", "$", 8U);
        const auto status_text = required_string(*root_object, "status", "$", 32U);
        const auto* authority = required(*root_object, "authority", "$");
        const auto* resource = required(*root_object, "resource", "$");
        const auto* observer = required(*root_object, "observer", "$");
        const auto* run = required(*root_object, "run", "$");
        const auto* family_tags = required(*root_object, "family_tags", "$");
        const auto* events = required(*root_object, "events", "$");

        if (!schema.has_value() || !scope_text.has_value() ||
            !status_text.has_value() || authority == nullptr || resource == nullptr ||
            observer == nullptr || run == nullptr || family_tags == nullptr ||
            events == nullptr) {
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        if (*schema != kSchema) {
            fail("$.schema", "Unsupported Layer-3 lifecycle trace schema.");
        }
        const auto scope = scope_from_string(*scope_text);
        if (!scope.has_value()) {
            fail("$.scope", "Scope must be one of V1, V2, V3, V4, V5 or V6.");
        }
        const auto status = status_from_string(*status_text);
        if (!status.has_value()) {
            fail("$.status", "Status must be 'captured' or 'aborted'.");
        }
        if (!diagnostics_.empty()) {
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        trace.schema = *schema;
        trace.scope = *scope;
        trace.status = *status;
        parse_authority(*authority, trace);
        parse_resource(*resource, trace);
        parse_observer(*observer, trace);
        parse_run(*run, trace);
        parse_family_tags(*family_tags, trace);
        parse_events(*events, trace);

        if (diagnostics_.empty()) {
            validate_cross_fields(trace);
        }
        if (diagnostics_.empty() && trace.status == L3LifecycleStatus::captured) {
            validate_scope_sequence(trace);
        }

        if (!diagnostics_.empty()) {
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        result.promotion_eligible =
            trace.status == L3LifecycleStatus::captured &&
            trace.run.original_process &&
            trace.run.completed_cleanly &&
            trace.observer.dropped_events == 0U &&
            !trace.observer.overflow_detected &&
            !trace.observer.semantic_intrusion_detected &&
            (!trace.run.overlay_published || trace.run.rollback_verified);
        result.trace = std::move(trace);
        return result;
    }

private:
    void parse_authority(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* object = value.as_object();
        if (object == nullptr) {
            fail("$.authority", "Authority must be a JSON object.");
            return;
        }
        validate_keys(*object, {"exe_sha256", "exe_size", "role"}, "$.authority");
        const auto sha = required_string(*object, "exe_sha256", "$.authority", 64U);
        const auto size = required_u64(*object, "exe_size", "$.authority");
        const auto role = required_string(
            *object, "role", "$.authority", limits_.max_id_bytes);
        if (!sha.has_value() || !size.has_value() || !role.has_value()) return;
        if (!is_lower_sha256(*sha)) {
            fail("$.authority.exe_sha256", "Executable SHA-256 must be canonical lowercase hex.");
        }
        if (*size == 0U) {
            fail("$.authority.exe_size", "Executable size must be non-zero.");
        }
        trace.authority = L3LifecycleAuthority{*sha, *size, *role};
    }

    void parse_resource(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* object = value.as_object();
        if (object == nullptr) {
            fail("$.resource", "Resource identity must be a JSON object.");
            return;
        }
        validate_keys(
            *object,
            {"logical_identity", "selected_provider_identity", "materialized_sha256",
             "materialized_size", "materialized_provenance"},
            "$.resource");
        const auto logical = required_string(
            *object, "logical_identity", "$.resource", limits_.max_id_bytes);
        const auto provider = required_string(
            *object, "selected_provider_identity", "$.resource", limits_.max_id_bytes);
        const auto sha = required_string(
            *object, "materialized_sha256", "$.resource", 64U);
        const auto size = required_u64(*object, "materialized_size", "$.resource");
        const auto provenance = required_string(
            *object, "materialized_provenance", "$.resource", limits_.max_id_bytes);
        if (!logical.has_value() || !provider.has_value() || !sha.has_value() ||
            !size.has_value() || !provenance.has_value()) return;
        if (!is_lower_sha256(*sha)) {
            fail("$.resource.materialized_sha256", "Materialized SHA-256 must be canonical lowercase hex.");
        }
        trace.resource = L3LifecycleResourceIdentity{
            *logical, *provider, *sha, *size, *provenance};
    }

    void parse_observer(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* object = value.as_object();
        if (object == nullptr) {
            fail("$.observer", "Observer must be a JSON object.");
            return;
        }
        validate_keys(
            *object,
            {"name", "version", "config_sha256", "dropped_events",
             "overflow_detected", "semantic_intrusion_detected"},
            "$.observer");
        const auto name = required_string(
            *object, "name", "$.observer", limits_.max_id_bytes);
        const auto version = required_string(
            *object, "version", "$.observer", limits_.max_id_bytes);
        const auto sha = required_string(*object, "config_sha256", "$.observer", 64U);
        const auto dropped = required_u64(*object, "dropped_events", "$.observer");
        const auto overflow = required_bool(*object, "overflow_detected", "$.observer");
        const auto intrusion = required_bool(
            *object, "semantic_intrusion_detected", "$.observer");
        if (!name.has_value() || !version.has_value() || !sha.has_value() ||
            !dropped.has_value() || !overflow.has_value() || !intrusion.has_value()) return;
        if (!is_lower_sha256(*sha)) {
            fail("$.observer.config_sha256", "Observer config SHA-256 must be canonical lowercase hex.");
        }
        trace.observer = L3LifecycleObserver{
            *name, *version, *sha, *dropped, *overflow, *intrusion};
    }

    void parse_run(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* object = value.as_object();
        if (object == nullptr) {
            fail("$.run", "Run metadata must be a JSON object.");
            return;
        }
        validate_keys(
            *object,
            {"id", "original_process", "completed_cleanly", "overlay_published",
             "rollback_verified", "overlay_sha256"},
            "$.run");
        const auto id = required_string(*object, "id", "$.run", limits_.max_id_bytes);
        const auto original = required_bool(*object, "original_process", "$.run");
        const auto clean = required_bool(*object, "completed_cleanly", "$.run");
        const auto overlay = required_bool(*object, "overlay_published", "$.run");
        const auto rollback = required_bool(*object, "rollback_verified", "$.run");
        bool overlay_sha_valid = true;
        const auto overlay_sha = optional_string(
            *object, "overlay_sha256", "$.run", 64U, overlay_sha_valid);
        if (!id.has_value() || !original.has_value() || !clean.has_value() ||
            !overlay.has_value() || !rollback.has_value() || !overlay_sha_valid) return;
        if (overlay_sha.has_value() && !is_lower_sha256(*overlay_sha)) {
            fail("$.run.overlay_sha256", "Overlay SHA-256 must be canonical lowercase hex.");
        }
        trace.run = L3LifecycleRun{
            *id, *original, *clean, *overlay, *rollback, overlay_sha};
    }

    void parse_family_tags(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail("$.family_tags", "Family tags must be a JSON array.");
            return;
        }
        if (array->size() > limits_.max_family_tags) {
            fail("$.family_tags", "Family tag count exceeds configured limit.");
            return;
        }
        std::set<std::string> seen;
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto path = "$.family_tags[" + std::to_string(index) + "]";
            const auto* text = (*array)[index].as_string();
            if (text == nullptr || text->empty() || text->size() > limits_.max_id_bytes) {
                fail(path, "Family tag must be a bounded non-empty string.");
                continue;
            }
            if (!seen.insert(*text).second) {
                fail(path, "Duplicate family tag.");
                continue;
            }
            trace.family_tags.push_back(*text);
        }
    }

    void parse_events(
        const core::json::Value& value,
        L3LifecycleTrace& trace) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail("$.events", "Events must be a JSON array.");
            return;
        }
        if (array->empty()) {
            fail("$.events", "At least one lifecycle event is required.");
            return;
        }
        if (array->size() > limits_.max_events) {
            fail("$.events", "Event count exceeds configured limit.");
            return;
        }

        trace.events.reserve(array->size());
        std::optional<std::uint64_t> previous_sequence;
        for (std::size_t event_index = 0; event_index < array->size(); ++event_index) {
            const auto path = "$.events[" + std::to_string(event_index) + "]";
            const auto* object = (*array)[event_index].as_object();
            if (object == nullptr) {
                fail(path, "Lifecycle event must be a JSON object.");
                continue;
            }
            validate_keys(
                *object,
                {"sequence", "kind", "record_identity", "group", "index",
                 "state_from", "state_to", "writer", "raw_numeric"},
                path);

            const auto sequence = required_u64(*object, "sequence", path);
            const auto kind_text = required_string(*object, "kind", path, 64U);
            if (!sequence.has_value() || !kind_text.has_value()) continue;
            const auto kind = event_kind_from_string(*kind_text);
            if (!kind.has_value()) {
                fail(path + ".kind", "Unknown lifecycle event kind.");
                continue;
            }
            if (previous_sequence.has_value() && *sequence <= *previous_sequence) {
                fail(path + ".sequence", "Event sequence must be strictly increasing.");
            }
            previous_sequence = *sequence;

            bool optional_valid = true;
            const auto record = optional_u64(
                *object, "record_identity", path, optional_valid);
            const auto group = optional_u32(*object, "group", path, optional_valid);
            const auto index = optional_u32(*object, "index", path, optional_valid);
            const auto state_from = optional_u32(
                *object, "state_from", path, optional_valid);
            const auto state_to = optional_u32(
                *object, "state_to", path, optional_valid);
            const auto writer = optional_string(
                *object, "writer", path, limits_.max_writer_bytes, optional_valid);
            auto raw_numeric = parse_raw_numeric(*object, path, optional_valid);
            if (!optional_valid) continue;

            if (*kind == L3LifecycleEventKind::state_write) {
                if (!state_from.has_value() || !state_to.has_value()) {
                    fail(path, "state_write requires state_from and state_to.");
                    continue;
                }
                if (*state_from > 4U || *state_to > 4U) {
                    fail(path, "Known LoadedResource states must remain in the recovered 0..4 domain.");
                    continue;
                }
            } else if (state_from.has_value() || state_to.has_value()) {
                fail(path, "state_from/state_to are only valid on state_write events.");
                continue;
            }

            trace.events.push_back(L3LifecycleEvent{
                .sequence = *sequence,
                .kind = *kind,
                .record_identity = record,
                .group = group,
                .index = index,
                .state_from = state_from,
                .state_to = state_to,
                .writer = writer.value_or(std::string{}),
                .raw_numeric = std::move(raw_numeric),
            });
        }
    }

    [[nodiscard]] std::map<std::string, std::uint64_t, std::less<>> parse_raw_numeric(
        const core::json::Value::Object& object,
        const std::string& path,
        bool& valid) {
        std::map<std::string, std::uint64_t, std::less<>> result;
        const auto iterator = object.find("raw_numeric");
        if (iterator == object.end()) return result;
        const auto* raw = iterator->second.as_object();
        if (raw == nullptr) {
            fail(path + ".raw_numeric", "raw_numeric must be a JSON object.");
            valid = false;
            return result;
        }
        if (raw->size() > limits_.max_raw_numeric_fields) {
            fail(path + ".raw_numeric", "raw_numeric field count exceeds configured limit.");
            valid = false;
            return result;
        }
        for (const auto& [key, value] : *raw) {
            if (key.empty() || key.size() > limits_.max_id_bytes) {
                fail(path + ".raw_numeric", "raw_numeric keys must be bounded non-empty strings.");
                valid = false;
                continue;
            }
            const auto number = nonnegative_u64(value);
            if (!number.has_value()) {
                fail(path + ".raw_numeric." + key, "raw_numeric values must be non-negative integers.");
                valid = false;
                continue;
            }
            result.emplace(key, *number);
        }
        return result;
    }

    void validate_cross_fields(const L3LifecycleTrace& trace) {
        if (trace.run.overlay_published) {
            if (!trace.run.overlay_sha256.has_value()) {
                fail("$.run.overlay_sha256", "Published overlays require an exact SHA-256 identity.");
            }
            if (trace.run.original_process && trace.status == L3LifecycleStatus::captured &&
                !trace.run.rollback_verified) {
                fail("$.run.rollback_verified", "Captured original-process overlay runs require verified rollback.");
            }
        } else {
            if (trace.run.overlay_sha256.has_value()) {
                fail("$.run.overlay_sha256", "overlay_sha256 is invalid when no overlay was published.");
            }
            if (trace.run.rollback_verified) {
                fail("$.run.rollback_verified", "rollback_verified must be false when no overlay was published.");
            }
        }

        if (trace.run.original_process && trace.status == L3LifecycleStatus::captured) {
            if (trace.observer.dropped_events != 0U) {
                fail("$.observer.dropped_events", "Captured original-process traces with dropped events are invalid.");
            }
            if (trace.observer.overflow_detected) {
                fail("$.observer.overflow_detected", "Captured original-process traces with observer overflow are invalid.");
            }
            if (trace.observer.semantic_intrusion_detected) {
                fail("$.observer.semantic_intrusion_detected", "Instrumentation that changes lifecycle semantics invalidates the trace.");
            }
        }
    }

    [[nodiscard]] std::optional<std::size_t> first_kind(
        const L3LifecycleTrace& trace,
        L3LifecycleEventKind kind,
        std::size_t start = 0U) const noexcept {
        for (std::size_t index = start; index < trace.events.size(); ++index) {
            if (trace.events[index].kind == kind) return index;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::size_t> first_state(
        const L3LifecycleTrace& trace,
        std::uint32_t from,
        std::uint32_t to,
        std::size_t start = 0U) const noexcept {
        for (std::size_t index = start; index < trace.events.size(); ++index) {
            const auto& event = trace.events[index];
            if (event.kind == L3LifecycleEventKind::state_write &&
                event.state_from == from && event.state_to == to) {
                return index;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::size_t> first_cancel_state4(
        const L3LifecycleTrace& trace,
        std::size_t start = 0U) const noexcept {
        for (std::size_t index = start; index < trace.events.size(); ++index) {
            const auto& event = trace.events[index];
            if (event.kind != L3LifecycleEventKind::state_write ||
                event.state_to != 4U || !event.state_from.has_value()) {
                continue;
            }
            if (*event.state_from == 1U || *event.state_from == 2U) return index;
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::size_t after(std::size_t index) noexcept {
        return index + 1U;
    }

    void require_event(
        const std::optional<std::size_t>& index,
        std::string path,
        std::string message) {
        if (!index.has_value()) fail(std::move(path), std::move(message));
    }

    void validate_scope_sequence(const L3LifecycleTrace& trace) {
        switch (trace.scope) {
        case L3LifecycleScope::v1_initial_load:
            validate_v1(trace);
            break;
        case L3LifecycleScope::v2_transition:
            validate_v2(trace);
            break;
        case L3LifecycleScope::v3_restart_reload:
            validate_v3(trace);
            break;
        case L3LifecycleScope::v4_full_reset:
            validate_v4(trace);
            break;
        case L3LifecycleScope::v5_inflight_cancellation:
            validate_v5(trace);
            break;
        case L3LifecycleScope::v6_shutdown:
            validate_v6(trace);
            break;
        }
    }

    void validate_v1(const L3LifecycleTrace& trace) {
        const auto request = first_kind(trace, L3LifecycleEventKind::resource_request);
        require_event(request, "$.events", "V1 requires resource_request.");
        if (!request.has_value()) return;
        const auto submit = first_kind(
            trace, L3LifecycleEventKind::materialization_submit, after(*request));
        require_event(submit, "$.events", "V1 requires materialization_submit after resource_request.");
        if (!submit.has_value()) return;
        const auto state01 = first_state(trace, 0U, 1U, after(*submit));
        require_event(state01, "$.events", "V1 requires state 0 -> 1 after materialization submission.");
        if (!state01.has_value()) return;
        const auto complete = first_kind(
            trace, L3LifecycleEventKind::materialization_complete, after(*state01));
        require_event(complete, "$.events", "V1 requires materialization_complete after state 1 publication.");
        if (!complete.has_value()) return;
        const auto state12 = first_state(trace, 1U, 2U, after(*complete));
        require_event(state12, "$.events", "V1 requires state 1 -> 2 after materialization completion.");
        if (!state12.has_value()) return;
        const auto typed_enter = first_kind(
            trace, L3LifecycleEventKind::typed_postload_enter, after(*state12));
        require_event(typed_enter, "$.events", "V1 requires typed_postload_enter after state 2.");
        if (!typed_enter.has_value()) return;
        const auto typed_exit = first_kind(
            trace, L3LifecycleEventKind::typed_postload_exit, after(*typed_enter));
        require_event(typed_exit, "$.events", "V1 requires typed_postload_exit after typed_postload_enter.");
        if (!typed_exit.has_value()) return;
        const auto state23 = first_state(trace, 2U, 3U, after(*typed_exit));
        require_event(state23, "$.events", "V1 requires state 2 -> 3 after typed post-load.");
        if (!state23.has_value()) return;

        const auto ready_callback = first_kind(trace, L3LifecycleEventKind::ready_callback);
        if (ready_callback.has_value() &&
            (*ready_callback <= *typed_exit || *ready_callback >= *state23)) {
            fail("$.events", "V1 ready_callback, when present, must occur after typed post-load and before state 3 publication.");
        }

        const auto visible = first_kind(
            trace, L3LifecycleEventKind::consumer_visible, after(*state23));
        require_event(visible, "$.events", "V1 requires state-3-gated consumer_visible after readiness publication.");
    }

    void validate_v2(const L3LifecycleTrace& trace) {
        const auto boundary = first_kind(trace, L3LifecycleEventKind::scene_boundary);
        require_event(boundary, "$.events", "V2 requires a scene_boundary event.");
        if (!boundary.has_value()) return;
        const auto claim_dec = first_kind(
            trace, L3LifecycleEventKind::loader_claim_dec, after(*boundary));
        require_event(claim_dec, "$.events", "V2 requires outgoing loader_claim_dec after the transition boundary.");
        if (!claim_dec.has_value()) return;
        const auto release = first_kind(
            trace, L3LifecycleEventKind::zero_claim_release, after(*claim_dec));
        require_event(release, "$.events", "V2 requires zero_claim_release for an outgoing component.");
        if (!release.has_value()) return;
        const auto submit = first_kind(
            trace, L3LifecycleEventKind::materialization_submit, after(*release));
        require_event(submit, "$.events", "V2 requires incoming materialization_submit after outgoing release.");
        if (!submit.has_value()) return;
        const auto ready = first_state(trace, 2U, 3U, after(*submit));
        require_event(ready, "$.events", "V2 requires incoming state 2 -> 3 readiness.");
        if (!ready.has_value()) return;
        const auto visible = first_kind(
            trace, L3LifecycleEventKind::consumer_visible, after(*ready));
        require_event(visible, "$.events", "V2 requires incoming consumer visibility after state 3.");
    }

    void validate_v3(const L3LifecycleTrace& trace) {
        const auto cancel = first_kind(trace, L3LifecycleEventKind::cancel_mark);
        require_event(cancel, "$.events", "V3 requires cancel_mark before replacement.");
        if (!cancel.has_value()) return;
        const auto state4 = first_cancel_state4(trace, after(*cancel));
        require_event(state4, "$.events", "V3 requires unfinished state 1/2 -> 4 after cancel_mark.");
        if (!state4.has_value()) return;
        const auto wait = first_kind(
            trace, L3LifecycleEventKind::quiescence_wait, after(*state4));
        require_event(wait, "$.events", "V3 requires quiescence_wait after cancellation.");
        if (!wait.has_value()) return;
        const auto cleanup = first_state(trace, 4U, 0U, after(*state4));
        require_event(cleanup, "$.events", "V3 requires deferred state 4 -> 0 cleanup.");
        if (!cleanup.has_value()) return;
        const auto reached_start = std::max(after(*wait), after(*cleanup));
        const auto reached = first_kind(
            trace, L3LifecycleEventKind::quiescence_reached, reached_start);
        require_event(reached, "$.events", "V3 requires quiescence_reached after wait and deferred cleanup.");
        if (!reached.has_value()) return;
        const auto submit = first_kind(
            trace, L3LifecycleEventKind::materialization_submit, after(*reached));
        require_event(submit, "$.events", "V3 replacement materialization must begin only after quiescence.");
        if (!submit.has_value()) return;
        const auto ready = first_state(trace, 2U, 3U, after(*submit));
        require_event(ready, "$.events", "V3 replacement must reach state 3.");
        if (!ready.has_value()) return;
        const auto visible = first_kind(
            trace, L3LifecycleEventKind::consumer_visible, after(*ready));
        require_event(visible, "$.events", "V3 requires replacement consumer visibility after state 3.");
    }

    void validate_v4(const L3LifecycleTrace& trace) {
        const auto boundary = first_kind(trace, L3LifecycleEventKind::scene_boundary);
        require_event(boundary, "$.events", "V4 requires a scene_boundary event.");
        if (!boundary.has_value()) return;
        const auto reset = first_kind(
            trace, L3LifecycleEventKind::full_reset, after(*boundary));
        require_event(reset, "$.events", "V4 requires full_reset after the broader scene boundary.");
        if (!reset.has_value()) return;
        const auto stale_visibility = first_kind(
            trace, L3LifecycleEventKind::consumer_visible, after(*reset));
        if (stale_visibility.has_value()) {
            fail("$.events", "V4 forbids consumer_visible after full_reset in the same trace.");
        }
    }

    void validate_v5(const L3LifecycleTrace& trace) {
        for (const auto& event : trace.events) {
            if (event.kind == L3LifecycleEventKind::state_write &&
                event.state_from == 3U && event.state_to == 4U) {
                fail("$.events", "V5 canonical cancellation must not relabel ready state 3 as state 4.");
                return;
            }
        }
        const auto cancel = first_kind(trace, L3LifecycleEventKind::cancel_mark);
        require_event(cancel, "$.events", "V5 requires cancel_mark.");
        if (!cancel.has_value()) return;
        const auto state4 = first_cancel_state4(trace, after(*cancel));
        require_event(state4, "$.events", "V5 requires unfinished state 1/2 -> 4.");
        if (!state4.has_value()) return;
        const auto wait = first_kind(
            trace, L3LifecycleEventKind::quiescence_wait, after(*cancel));
        require_event(wait, "$.events", "V5 requires quiescence_wait.");
        if (!wait.has_value()) return;
        const auto cleanup = first_state(trace, 4U, 0U, after(*state4));
        require_event(cleanup, "$.events", "V5 requires deferred state 4 -> 0 cleanup.");
        if (!cleanup.has_value()) return;
        const auto reached = first_kind(
            trace,
            L3LifecycleEventKind::quiescence_reached,
            std::max(after(*wait), after(*cleanup)));
        require_event(reached, "$.events", "V5 requires quiescence_reached after cleanup completes.");
    }

    void validate_v6(const L3LifecycleTrace& trace) {
        const auto shutdown = first_kind(trace, L3LifecycleEventKind::shutdown_boundary);
        require_event(shutdown, "$.events", "V6 requires shutdown_boundary.");
        if (!shutdown.has_value()) return;
        const auto release = first_kind(
            trace, L3LifecycleEventKind::backing_release, after(*shutdown));
        require_event(release, "$.events", "V6 requires backing_release after shutdown begins.");
        const auto stale_visibility = first_kind(
            trace, L3LifecycleEventKind::consumer_visible, after(*shutdown));
        if (stale_visibility.has_value()) {
            fail("$.events", "V6 forbids consumer_visible after shutdown_boundary.");
        }
    }

    [[nodiscard]] const core::json::Value* required(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) {
            fail(std::string{path} + "." + std::string{key}, "Required field is missing.");
            return nullptr;
        }
        return &iterator->second;
    }

    [[nodiscard]] std::optional<std::string> required_string(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path,
        std::size_t max_bytes) {
        const auto* value = required(object, key, path);
        if (value == nullptr) return std::nullopt;
        const auto* text = value->as_string();
        if (text == nullptr || text->empty() || text->size() > max_bytes) {
            fail(std::string{path} + "." + std::string{key}, "Expected a bounded non-empty string.");
            return std::nullopt;
        }
        return *text;
    }

    [[nodiscard]] std::optional<std::uint64_t> required_u64(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path) {
        const auto* value = required(object, key, path);
        if (value == nullptr) return std::nullopt;
        const auto number = nonnegative_u64(*value);
        if (!number.has_value()) {
            fail(std::string{path} + "." + std::string{key}, "Expected a non-negative integer.");
        }
        return number;
    }

    [[nodiscard]] std::optional<bool> required_bool(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path) {
        const auto* value = required(object, key, path);
        if (value == nullptr) return std::nullopt;
        const auto* boolean = value->as_bool();
        if (boolean == nullptr) {
            fail(std::string{path} + "." + std::string{key}, "Expected a boolean.");
            return std::nullopt;
        }
        return *boolean;
    }

    [[nodiscard]] std::optional<std::uint64_t> optional_u64(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path,
        bool& valid) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) return std::nullopt;
        const auto number = nonnegative_u64(iterator->second);
        if (!number.has_value()) {
            fail(std::string{path} + "." + std::string{key}, "Expected a non-negative integer.");
            valid = false;
        }
        return number;
    }

    [[nodiscard]] std::optional<std::uint32_t> optional_u32(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path,
        bool& valid) {
        const auto value = optional_u64(object, key, path, valid);
        if (!value.has_value()) return std::nullopt;
        if (*value > std::numeric_limits<std::uint32_t>::max()) {
            fail(std::string{path} + "." + std::string{key}, "Integer exceeds uint32 range.");
            valid = false;
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(*value);
    }

    [[nodiscard]] std::optional<std::string> optional_string(
        const core::json::Value::Object& object,
        std::string_view key,
        std::string_view path,
        std::size_t max_bytes,
        bool& valid) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) return std::nullopt;
        const auto* text = iterator->second.as_string();
        if (text == nullptr || text->empty() || text->size() > max_bytes) {
            fail(std::string{path} + "." + std::string{key}, "Expected a bounded non-empty string.");
            valid = false;
            return std::nullopt;
        }
        return *text;
    }

    [[nodiscard]] static std::optional<std::uint64_t> nonnegative_u64(
        const core::json::Value& value) noexcept {
        if (const auto* number = value.as_u64(); number != nullptr) return *number;
        if (const auto* number = value.as_i64(); number != nullptr && *number >= 0) {
            return static_cast<std::uint64_t>(*number);
        }
        return std::nullopt;
    }

    void validate_keys(
        const core::json::Value::Object& object,
        std::initializer_list<std::string_view> allowed,
        std::string_view path) {
        for (const auto& [key, unused] : object) {
            static_cast<void>(unused);
            const auto recognized = std::any_of(
                allowed.begin(), allowed.end(),
                [&key](std::string_view candidate) { return candidate == key; });
            if (!recognized) {
                fail(std::string{path} + "." + key, "Unknown field is not allowed by this schema.");
            }
        }
    }

    void fail(std::string path, std::string message) {
        diagnostics_.push_back(L3LifecycleDiagnostic{
            std::move(path), std::move(message)});
    }

    const core::json::Value& root_;
    L3LifecycleImportLimits limits_;
    std::vector<L3LifecycleDiagnostic> diagnostics_;
};

} // namespace

std::string_view to_string(L3LifecycleScope scope) noexcept {
    switch (scope) {
    case L3LifecycleScope::v1_initial_load: return "V1";
    case L3LifecycleScope::v2_transition: return "V2";
    case L3LifecycleScope::v3_restart_reload: return "V3";
    case L3LifecycleScope::v4_full_reset: return "V4";
    case L3LifecycleScope::v5_inflight_cancellation: return "V5";
    case L3LifecycleScope::v6_shutdown: return "V6";
    }
    return "unknown";
}

std::string_view to_string(L3LifecycleStatus status) noexcept {
    switch (status) {
    case L3LifecycleStatus::captured: return "captured";
    case L3LifecycleStatus::aborted: return "aborted";
    }
    return "unknown";
}

std::string_view to_string(L3LifecycleEventKind kind) noexcept {
    using Kind = L3LifecycleEventKind;
    switch (kind) {
    case Kind::resource_request: return "resource_request";
    case Kind::materialization_submit: return "materialization_submit";
    case Kind::state_write: return "state_write";
    case Kind::materialization_complete: return "materialization_complete";
    case Kind::typed_postload_enter: return "typed_postload_enter";
    case Kind::typed_postload_exit: return "typed_postload_exit";
    case Kind::ready_callback: return "ready_callback";
    case Kind::consumer_visible: return "consumer_visible";
    case Kind::loader_claim_inc: return "loader_claim_inc";
    case Kind::loader_claim_dec: return "loader_claim_dec";
    case Kind::zero_claim_release: return "zero_claim_release";
    case Kind::cancel_mark: return "cancel_mark";
    case Kind::quiescence_wait: return "quiescence_wait";
    case Kind::quiescence_reached: return "quiescence_reached";
    case Kind::group_reset: return "group_reset";
    case Kind::full_reset: return "full_reset";
    case Kind::backing_release: return "backing_release";
    case Kind::scene_boundary: return "scene_boundary";
    case Kind::shutdown_boundary: return "shutdown_boundary";
    }
    return "unknown";
}

L3LifecycleImportResult l3_lifecycle_trace_from_json(
    std::string_view json_text,
    L3LifecycleImportLimits limits) {
    core::json::ParseLimits parse_limits;
    parse_limits.max_input_bytes = limits.max_input_bytes;
    parse_limits.max_depth = 32U;
    parse_limits.max_values = std::max<std::size_t>(
        4096U, limits.max_events * (limits.max_raw_numeric_fields + 12U));
    parse_limits.max_array_items = std::max(limits.max_events, limits.max_family_tags);
    parse_limits.max_object_members = std::max<std::size_t>(
        128U, limits.max_raw_numeric_fields + 16U);
    parse_limits.max_string_bytes = std::max(limits.max_id_bytes, limits.max_writer_bytes);

    const auto parsed = core::json::Parser::parse(json_text, parse_limits);
    if (!parsed.ok()) {
        L3LifecycleImportResult result;
        for (const auto& error : parsed.errors) {
            result.diagnostics.push_back(L3LifecycleDiagnostic{
                "$@" + std::to_string(error.offset), error.message});
        }
        return result;
    }
    return L3LifecycleImporter{*parsed.value, limits}.run();
}

} // namespace dmc::rengine::validation
