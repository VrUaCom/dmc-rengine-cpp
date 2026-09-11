#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/spider/crusader.hpp"
#include "dmc_rengine/spider/family.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace {

namespace spider = dmc::rengine::spider;
namespace crusader = dmc::rengine::spider::crusader;
using dmc::rengine::formats::mod::decode_vertex_skin;

struct ModSkinCorpus final {
    std::span<const std::array<std::uint8_t, 4>> blend_indices;
    std::span<const std::uint16_t> packed_weights;
    std::uint8_t node_count{64U};
};

struct ModSkinWorkflowReceipt final {
    std::size_t requested_vertices{};
    std::size_t decoded_vertices{};
    std::uint64_t checksum{};
    bool blend_x_observed_zero{};
    bool semantic_parity{};
};

enum class WorkflowError : std::uint8_t {
    none,
    input_size_mismatch,
    native_plan_failed,
    canonical_decode_failed,
};

[[nodiscard]] constexpr std::string_view to_string(WorkflowError error) noexcept {
    switch (error) {
    case WorkflowError::none: return "none";
    case WorkflowError::input_size_mismatch: return "input-size-mismatch";
    case WorkflowError::native_plan_failed: return "native-plan-failed";
    case WorkflowError::canonical_decode_failed: return "canonical-decode-failed";
    }
    return "native-plan-failed";
}

struct TarantulaWorkflowResult final {
    std::optional<ModSkinWorkflowReceipt> receipt;
    WorkflowError error{WorkflowError::none};

    [[nodiscard]] bool ok() const noexcept {
        return receipt.has_value() && error == WorkflowError::none;
    }
};

struct TarantulaState final {
    ModSkinCorpus corpus;
    ModSkinWorkflowReceipt receipt{};
    WorkflowError error{WorkflowError::none};
};

[[nodiscard]] bool validate_input_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    auto& state = *static_cast<TarantulaState*>(opaque_state);
    if (state.corpus.blend_indices.size() != state.corpus.packed_weights.size()) {
        state.error = WorkflowError::input_size_mismatch;
        return false;
    }
    state.receipt.requested_vertices = state.corpus.blend_indices.size();
    return true;
}

[[nodiscard]] bool decode_corpus_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    auto& state = *static_cast<TarantulaState*>(opaque_state);
    bool x_zero = true;

    for (std::size_t index = 0U;
         index < state.corpus.blend_indices.size();
         ++index) {
        const auto& blend = state.corpus.blend_indices[index];
        x_zero = x_zero && blend[0] == 0U;

        const auto decoded = decode_vertex_skin(
            blend,
            state.corpus.packed_weights[index],
            state.corpus.node_count);
        if (!decoded.ok()) {
            state.error = WorkflowError::canonical_decode_failed;
            return false;
        }

        ++state.receipt.decoded_vertices;
        state.receipt.checksum += decoded.skin.influence_count;
        if (decoded.skin.influence_count != 0U) {
            state.receipt.checksum += decoded.skin.influences[0].bone_index;
        }
    }

    state.receipt.blend_x_observed_zero = x_zero;
    return true;
}

[[nodiscard]] bool verify_receipt_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    auto& state = *static_cast<TarantulaState*>(opaque_state);
    state.receipt.semantic_parity =
        state.receipt.requested_vertices == state.receipt.decoded_vertices;
    if (!state.receipt.semantic_parity) {
        state.error = WorkflowError::canonical_decode_failed;
        return false;
    }
    return true;
}

// Tarantula owns the high-level native workflow. It compiles that workflow onto
// the existing Crusader executor instead of implementing another executor.
[[nodiscard]] TarantulaWorkflowResult run_tarantula_mod_skin_workflow(
    ModSkinCorpus corpus) noexcept {
    TarantulaState state{.corpus = corpus};

    crusader::Plan plan;
    plan.instructions = {
        crusader::Instruction{
            1U, 0U, 0U, 0U, crusader::Domain::cpu, 0U},
        crusader::Instruction{
            2U, 0U, 0U, 1U, crusader::Domain::cpu, 0U},
        crusader::Instruction{
            3U, 0U, 1U, 1U, crusader::Domain::cpu, 0U},
    };
    plan.dependencies = {0U, 1U};

    const std::array<crusader::OperationBinding, 3> bindings{{
        {1U, &validate_input_operation},
        {2U, &decode_corpus_operation},
        {3U, &verify_receipt_operation},
    }};

    const auto report = crusader::execute(plan, bindings, &state);
    if (!report.ok()) {
        if (state.error == WorkflowError::none) {
            state.error = WorkflowError::native_plan_failed;
        }
        return {.receipt = std::nullopt, .error = state.error};
    }

    return {.receipt = state.receipt, .error = WorkflowError::none};
}

// Black Widow owns typed product/session decisions only. It does not parse MOD,
// execute the workflow, or infer writer authority that this experiment did not
// prove.
struct ModSkinCapabilityState final {
    bool can_decode_skin{};
    bool can_visualize_weights{};
    bool can_edit_skin{};
    bool preserve_blend_x_raw{true};
};

[[nodiscard]] ModSkinCapabilityState evaluate_black_widow_state(
    const TarantulaWorkflowResult& workflow) noexcept {
    ModSkinCapabilityState state;
    state.can_decode_skin = workflow.ok() && workflow.receipt->semantic_parity;
    state.can_visualize_weights = state.can_decode_skin;

    // This experiment proves read/decode orchestration only. It must not promote
    // editing/writer authority merely because decoding succeeded.
    state.can_edit_skin = false;
    state.preserve_blend_x_raw = true;
    return state;
}

} // namespace

int main() {
    constexpr std::size_t vertex_count = 4096U;
    std::vector<std::array<std::uint8_t, 4>> blend_indices(vertex_count);
    std::vector<std::uint16_t> packed_weights(vertex_count);

    for (std::size_t index = 0U; index < vertex_count; ++index) {
        const auto bone0 = static_cast<std::uint8_t>(index % 50U);
        const auto bone1 = static_cast<std::uint8_t>((index + 7U) % 50U);
        const auto bone2 = static_cast<std::uint8_t>((index + 19U) % 50U);
        blend_indices[index] = {
            0U,
            static_cast<std::uint8_t>(bone0 * 4U),
            static_cast<std::uint8_t>(bone1 * 4U),
            static_cast<std::uint8_t>(bone2 * 4U),
        };

        constexpr std::uint16_t q0 = 12U;
        constexpr std::uint16_t q1 = 9U;
        constexpr std::uint16_t q2 = 10U;
        packed_weights[index] = static_cast<std::uint16_t>(
            q0 | (q1 << 5U) | (q2 << 10U) |
            ((index & 1U) != 0U ? 0x8000U : 0U));
    }

    std::cout << "tarantula_family="
              << spider::to_string(spider::Family::tarantula) << '\n';
    std::cout << "crusader_family="
              << spider::to_string(spider::Family::crusader) << '\n';
    std::cout << "black_widow_family="
              << spider::to_string(spider::Family::black_widow) << '\n';

    const auto workflow = run_tarantula_mod_skin_workflow({
        .blend_indices = blend_indices,
        .packed_weights = packed_weights,
        .node_count = 64U,
    });

    if (!workflow.ok()) {
        std::cerr << "workflow_error=" << to_string(workflow.error) << '\n';
        return 2;
    }

    const auto capabilities = evaluate_black_widow_state(workflow);
    const auto& receipt = *workflow.receipt;

    std::cout << "requested_vertices=" << receipt.requested_vertices << '\n';
    std::cout << "decoded_vertices=" << receipt.decoded_vertices << '\n';
    std::cout << "blend_x_observed_zero=" << receipt.blend_x_observed_zero << '\n';
    std::cout << "semantic_parity=" << receipt.semantic_parity << '\n';
    std::cout << "can_decode_skin=" << capabilities.can_decode_skin << '\n';
    std::cout << "can_visualize_weights="
              << capabilities.can_visualize_weights << '\n';
    std::cout << "can_edit_skin=" << capabilities.can_edit_skin << '\n';
    std::cout << "preserve_blend_x_raw="
              << capabilities.preserve_blend_x_raw << '\n';

    if (!capabilities.can_decode_skin ||
        !capabilities.can_visualize_weights ||
        capabilities.can_edit_skin ||
        !capabilities.preserve_blend_x_raw) {
        return 3;
    }

    return 0;
}
