#pragma once

#include "dmc_rengine/spider/python_json.hpp"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/**
 * The L2 original-selection evidence chain, natively (Tarantula).
 *
 * Replaces scripts/reverse/normalize_l2_original_selection_candidate.py and
 * scripts/reverse/verify_l2_original_selection_evidence.py with the same
 * guardrails, the same messages and the same output bytes. Neither step can
 * create trusted original-process evidence: the normalizer turns the legacy
 * selection JSON into a non-promotable candidate, and the binder hash-binds
 * that candidate to its mapping, observer and numbered NBZ artifacts.
 *
 * Where Python compared loosely the port is strict: a bool is never a number
 * (`True == 1` in Python) and a float is never an integer field.
 */
namespace dmc::rengine::spider {

inline constexpr std::string_view k_l2_legacy_selection_schema = "dmc-rengine.gdspaces-l2-original-selection.v1";
inline constexpr std::string_view k_l2_selection_candidate_schema =
    "dmc-rengine.gdspaces-l2-original-selection-candidate.v1";
inline constexpr std::string_view k_l2_selection_bound_schema = "dmc-rengine.gdspaces-l2-original-selection-bound.v1";

struct L2SelectionResult final {
    std::optional<OrderedJson> value;
    /// Why it was rejected, in the Python script's words.
    std::string error;

    [[nodiscard]] bool ok() const noexcept { return value.has_value(); }
};

/// The non-promotable candidate for one legacy selection JSON document.
[[nodiscard]] L2SelectionResult normalize_l2_selection_candidate(std::string_view legacy_json);

struct L2SelectionBindingInputs final {
    std::filesystem::path mapping;
    std::filesystem::path selection;
    std::vector<std::filesystem::path> mapping_children;
    std::filesystem::path observer_artifact;
    /// Exact numbered NBZ artifacts by runtime volume index.
    std::map<std::uint32_t, std::filesystem::path> archive_artifacts;
};

/// The bound candidate packet, after every artifact has been hashed and checked.
[[nodiscard]] L2SelectionResult bind_l2_selection_candidate(const L2SelectionBindingInputs& inputs);

/// `INDEX=PATH` values for --archive-artifact; empty error when they all parse.
[[nodiscard]] std::string parse_l2_archive_artifacts(
    const std::vector<std::string>& values, std::map<std::uint32_t, std::filesystem::path>& out);

} // namespace dmc::rengine::spider
