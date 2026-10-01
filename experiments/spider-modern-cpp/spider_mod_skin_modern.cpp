#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/spider/crusader.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

#if __has_include(<expected>)
#include <expected>
#endif

#if __cplusplus > 202302L && __has_include(<simd>)
#include <simd>
#define DMC_RENGINE_EXPERIMENT_HAS_SIMD_HEADER 1
#else
#define DMC_RENGINE_EXPERIMENT_HAS_SIMD_HEADER 0
#endif

namespace {

using dmc::rengine::formats::mod::SkinDecodeStatus;
using dmc::rengine::formats::mod::decode_vertex_skin;
namespace crusader = dmc::rengine::spider::crusader;

struct BatchResult final {
    std::uint64_t checksum{};
    std::size_t decoded{};
};

struct ExperimentState final {
    std::span<const std::array<std::uint8_t, 4>> blend_indices;
    std::span<const std::uint16_t> packed_weights;
    std::uint8_t node_count{64U};
    std::uint32_t repeats{1U};
    std::size_t cursor{};
    BatchResult result{};
};

[[nodiscard]] BatchResult run_direct_batch(
    std::span<const std::array<std::uint8_t, 4>> blend_indices,
    std::span<const std::uint16_t> packed_weights,
    std::uint8_t node_count,
    std::uint32_t repeats) noexcept {
    BatchResult result;
    for (std::uint32_t repeat = 0U; repeat < repeats; ++repeat) {
        for (std::size_t index = 0U; index < blend_indices.size(); ++index) {
            const auto decoded = decode_vertex_skin(
                blend_indices[index], packed_weights[index], node_count);
            if (!decoded.ok()) {
                continue;
            }
            result.checksum += decoded.skin.influence_count;
            if (decoded.skin.influence_count != 0U) {
                result.checksum += decoded.skin.influences[0].bone_index;
            }
            ++result.decoded;
        }
    }
    return result;
}

[[nodiscard]] bool validate_batch_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    const auto& state = *static_cast<const ExperimentState*>(opaque_state);
    return state.blend_indices.size() == state.packed_weights.size();
}

[[nodiscard]] bool decode_batch_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    auto& state = *static_cast<ExperimentState*>(opaque_state);
    state.result = run_direct_batch(
        state.blend_indices,
        state.packed_weights,
        state.node_count,
        state.repeats);
    return state.result.decoded ==
           state.blend_indices.size() * static_cast<std::size_t>(state.repeats);
}

[[nodiscard]] bool decode_one_operation(
    void* opaque_state,
    std::uint32_t) noexcept {
    auto& state = *static_cast<ExperimentState*>(opaque_state);
    if (state.cursor >= state.blend_indices.size()) {
        return false;
    }
    const auto decoded = decode_vertex_skin(
        state.blend_indices[state.cursor],
        state.packed_weights[state.cursor],
        state.node_count);
    if (!decoded.ok()) {
        return false;
    }
    state.result.checksum += decoded.skin.influence_count;
    if (decoded.skin.influence_count != 0U) {
        state.result.checksum += decoded.skin.influences[0].bone_index;
    }
    ++state.result.decoded;
    return true;
}

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
[[nodiscard]] std::expected<BatchResult, SkinDecodeStatus> run_expected_batch(
    std::span<const std::array<std::uint8_t, 4>> blend_indices,
    std::span<const std::uint16_t> packed_weights,
    std::uint8_t node_count,
    std::uint32_t repeats) noexcept {
    if (blend_indices.size() != packed_weights.size()) {
        return std::unexpected(SkinDecodeStatus::quantized_sum_mismatch);
    }

    BatchResult result;
    for (std::uint32_t repeat = 0U; repeat < repeats; ++repeat) {
        for (std::size_t index = 0U; index < blend_indices.size(); ++index) {
            const auto decoded = decode_vertex_skin(
                blend_indices[index], packed_weights[index], node_count);
            if (!decoded.ok()) {
                return std::unexpected(decoded.status);
            }
            result.checksum += decoded.skin.influence_count;
            if (decoded.skin.influence_count != 0U) {
                result.checksum += decoded.skin.influences[0].bone_index;
            }
            ++result.decoded;
        }
    }
    return result;
}
#endif

template <typename Function>
[[nodiscard]] double measure_ms(Function&& function) {
    const auto begin = std::chrono::steady_clock::now();
    function();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

[[nodiscard]] std::size_t parse_size_or(
    const char* text,
    std::size_t fallback) noexcept {
    if (text == nullptr) {
        return fallback;
    }
    char* end = nullptr;
    const auto parsed = std::strtoull(text, &end, 10);
    if (end == text || *end != '\0' || parsed == 0ULL) {
        return fallback;
    }
    return static_cast<std::size_t>(parsed);
}

[[nodiscard]] std::uint32_t parse_u32_or(
    const char* text,
    std::uint32_t fallback) noexcept {
    const auto parsed = parse_size_or(text, fallback);
    if (parsed > 0xFFFFFFFFULL) {
        return fallback;
    }
    return static_cast<std::uint32_t>(parsed);
}

void print_feature_report() {
    std::cout << "cplusplus=" << __cplusplus << '\n';
#if defined(__cpp_lib_expected)
    std::cout << "cpp_lib_expected=" << __cpp_lib_expected << '\n';
#else
    std::cout << "cpp_lib_expected=0\n";
#endif
    std::cout << "simd_header=" << DMC_RENGINE_EXPERIMENT_HAS_SIMD_HEADER << '\n';
#if defined(__cpp_lib_simd)
    std::cout << "cpp_lib_simd=" << __cpp_lib_simd << '\n';
#else
    std::cout << "cpp_lib_simd=0\n";
#endif
}

} // namespace

int main(int argc, char** argv) {
    const std::size_t vertex_count =
        argc > 1 ? parse_size_or(argv[1], 250000U) : 250000U;
    const std::uint32_t repeats =
        argc > 2 ? parse_u32_or(argv[2], 8U) : 8U;

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

        const auto q0 = static_cast<std::uint16_t>(10U + (index % 5U));
        const auto q1 = static_cast<std::uint16_t>(7U + ((index / 5U) % 4U));
        const auto q2 = static_cast<std::uint16_t>(31U - q0 - q1);
        packed_weights[index] = static_cast<std::uint16_t>(
            q0 | (q1 << 5U) | (q2 << 10U) |
            ((index & 1U) != 0U ? 0x8000U : 0U));
    }

    print_feature_report();
    std::cout << "vertices=" << vertex_count << '\n';
    std::cout << "repeats=" << repeats << '\n';

    BatchResult direct_result;
    const auto direct_ms = measure_ms([&] {
        direct_result = run_direct_batch(
            blend_indices, packed_weights, 64U, repeats);
    });

    ExperimentState batch_state{
        .blend_indices = blend_indices,
        .packed_weights = packed_weights,
        .node_count = 64U,
        .repeats = repeats,
    };

    crusader::Plan batch_plan;
    batch_plan.instructions = {
        crusader::Instruction{
            1U, 0U, 0U, 0U, crusader::Domain::cpu, 0U},
        crusader::Instruction{
            2U, 0U, 0U, 1U, crusader::Domain::cpu, 0U},
    };
    batch_plan.dependencies = {0U};

    const std::array<crusader::OperationBinding, 2> batch_bindings{{
        {1U, &validate_batch_operation},
        {2U, &decode_batch_operation},
    }};

    crusader::ExecutionReport batch_report;
    const auto crusader_batch_ms = measure_ms([&] {
        batch_report = crusader::execute(batch_plan, batch_bindings, &batch_state);
    });

    ExperimentState per_vertex_state{
        .blend_indices = blend_indices,
        .packed_weights = packed_weights,
        .node_count = 64U,
        .repeats = repeats,
    };
    crusader::Plan per_vertex_plan;
    per_vertex_plan.instructions = {
        crusader::Instruction{
            3U, 0U, 0U, 0U, crusader::Domain::cpu, 0U},
    };
    const std::array<crusader::OperationBinding, 1> per_vertex_bindings{{
        {3U, &decode_one_operation},
    }};

    crusader::ExecutionReport per_vertex_report;
    const auto crusader_per_vertex_ms = measure_ms([&] {
        for (std::uint32_t repeat = 0U; repeat < repeats; ++repeat) {
            for (std::size_t index = 0U; index < vertex_count; ++index) {
                per_vertex_state.cursor = index;
                per_vertex_report = crusader::execute(
                    per_vertex_plan,
                    per_vertex_bindings,
                    &per_vertex_state);
                if (!per_vertex_report.ok()) {
                    return;
                }
            }
        }
    });

    std::cout << "direct_ms=" << direct_ms << '\n';
    std::cout << "crusader_batch_ms=" << crusader_batch_ms << '\n';
    std::cout << "crusader_per_vertex_ms=" << crusader_per_vertex_ms << '\n';
    std::cout << "crusader_batch_ratio="
              << (direct_ms == 0.0 ? 0.0 : crusader_batch_ms / direct_ms) << '\n';
    std::cout << "crusader_per_vertex_ratio="
              << (direct_ms == 0.0 ? 0.0 : crusader_per_vertex_ms / direct_ms)
              << '\n';

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
    BatchResult expected_result;
    SkinDecodeStatus expected_error = SkinDecodeStatus::ok;
    const auto expected_ms = measure_ms([&] {
        auto result = run_expected_batch(
            blend_indices, packed_weights, 64U, repeats);
        if (result) {
            expected_result = *result;
        } else {
            expected_error = result.error();
        }
    });
    std::cout << "expected_ms=" << expected_ms << '\n';
    std::cout << "expected_ratio="
              << (direct_ms == 0.0 ? 0.0 : expected_ms / direct_ms) << '\n';
    if (expected_error != SkinDecodeStatus::ok) {
        std::cerr << "expected_path_error="
                  << dmc::rengine::formats::mod::to_string(expected_error)
                  << '\n';
        return 4;
    }
    if (expected_result.checksum != direct_result.checksum ||
        expected_result.decoded != direct_result.decoded) {
        std::cerr << "expected_path_parity=failed\n";
        return 5;
    }
#else
    std::cout << "expected_ms=unavailable\n";
    std::cout << "expected_ratio=unavailable\n";
#endif

    if (!batch_report.ok()) {
        std::cerr << "crusader_batch_status="
                  << crusader::to_string(batch_report.status) << '\n';
        return 2;
    }
    if (!per_vertex_report.ok()) {
        std::cerr << "crusader_per_vertex_status="
                  << crusader::to_string(per_vertex_report.status) << '\n';
        return 3;
    }
    if (batch_state.result.checksum != direct_result.checksum ||
        batch_state.result.decoded != direct_result.decoded ||
        per_vertex_state.result.checksum != direct_result.checksum ||
        per_vertex_state.result.decoded != direct_result.decoded) {
        std::cerr << "semantic_parity=failed\n";
        return 6;
    }

    std::cout << "semantic_parity=ok\n";
    return 0;
}
