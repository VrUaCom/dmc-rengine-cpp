#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/patch/guarded_patch.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

int main() {
    const std::vector<std::byte> source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
    };

    const auto digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{source});

    dmc::rengine::patch::GuardedPatchPlan plan(
        "synthetic-patch", digest.hex());
    assert(plan.add(dmc::rengine::patch::BytePatch{
        .offset = 1,
        .expected = {std::byte{0x02}, std::byte{0x03}},
        .replacement = {std::byte{0x09}, std::byte{0x08}},
        .description = "Replace synthetic middle bytes",
    }));
    assert(!plan.add(dmc::rengine::patch::BytePatch{
        .offset = 2,
        .expected = {std::byte{0x03}},
        .replacement = {std::byte{0x07}},
        .description = "Overlapping patch",
    }));

    const auto result = plan.apply(std::span<const std::byte>{source});
    assert(result.applied);
    assert(result.errors.empty());
    assert(result.output.size() == source.size());
    assert(result.output[0] == std::byte{0x01});
    assert(result.output[1] == std::byte{0x09});
    assert(result.output[2] == std::byte{0x08});
    assert(result.output[3] == std::byte{0x04});

    dmc::rengine::patch::GuardedPatchPlan wrong_bytes(
        "wrong-bytes", digest.hex());
    assert(wrong_bytes.add(dmc::rengine::patch::BytePatch{
        .offset = 0,
        .expected = {std::byte{0xFF}},
        .replacement = {std::byte{0x00}},
        .description = "Expected-byte guard",
    }));
    const auto rejected = wrong_bytes.apply(std::span<const std::byte>{source});
    assert(!rejected.applied);
    assert(!rejected.errors.empty());
    assert(rejected.output.empty());

    dmc::rengine::patch::GuardedPatchPlan wrong_hash(
        "wrong-hash",
        "0000000000000000000000000000000000000000000000000000000000000000");
    assert(wrong_hash.add(dmc::rengine::patch::BytePatch{
        .offset = 0,
        .expected = {std::byte{0x01}},
        .replacement = {std::byte{0x02}},
        .description = "Hash guard",
    }));
    const auto hash_rejected = wrong_hash.apply(
        std::span<const std::byte>{source});
    assert(!hash_rejected.applied);

    return 0;
}
