#include "dmc_rengine/patch/guarded_patch.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace dmc::rengine::patch {

GuardedPatchPlan::GuardedPatchPlan(std::string id, std::string target_sha256)
    : id_(std::move(id)), target_sha256_(std::move(target_sha256)) {}

const std::string& GuardedPatchPlan::id() const noexcept {
    return id_;
}

const std::string& GuardedPatchPlan::target_sha256() const noexcept {
    return target_sha256_;
}

bool GuardedPatchPlan::add(BytePatch patch) {
    if (!patch.valid() ||
        patch.offset > std::numeric_limits<std::uint64_t>::max() -
            static_cast<std::uint64_t>(patch.expected.size())) {
        return false;
    }

    if (std::any_of(
            patches_.begin(), patches_.end(),
            [&patch](const BytePatch& existing) {
                return overlaps(existing, patch);
            })) {
        return false;
    }

    patches_.push_back(std::move(patch));
    std::sort(
        patches_.begin(), patches_.end(),
        [](const BytePatch& left, const BytePatch& right) {
            return left.offset < right.offset;
        });
    return true;
}

const std::vector<BytePatch>& GuardedPatchPlan::patches() const noexcept {
    return patches_;
}

PatchResult GuardedPatchPlan::apply(std::span<const std::byte> source) const {
    PatchResult result;
    result.output.assign(source.begin(), source.end());

    if (id_.empty()) {
        result.errors.emplace_back("Patch plan ID is empty.");
    }
    if (target_sha256_.size() != 64U) {
        result.errors.emplace_back("Patch plan target SHA-256 is invalid.");
    } else {
        const auto actual = core::Sha256::compute(source).hex();
        if (actual != target_sha256_) {
            result.errors.emplace_back("Source SHA-256 does not match the patch plan.");
        }
    }

    for (const auto& patch : patches_) {
        const auto size = static_cast<std::uint64_t>(patch.expected.size());
        if (patch.offset > static_cast<std::uint64_t>(source.size()) ||
            size > static_cast<std::uint64_t>(source.size()) - patch.offset) {
            result.errors.emplace_back(
                "Patch range is outside the source: " + patch.description);
            continue;
        }

        const auto begin = static_cast<std::size_t>(patch.offset);
        if (!std::equal(
                patch.expected.begin(), patch.expected.end(),
                source.begin() + static_cast<std::ptrdiff_t>(begin))) {
            result.errors.emplace_back(
                "Expected bytes do not match: " + patch.description);
        }
    }

    if (!result.errors.empty()) {
        result.output.clear();
        return result;
    }

    for (const auto& patch : patches_) {
        const auto begin = static_cast<std::size_t>(patch.offset);
        std::copy(
            patch.replacement.begin(), patch.replacement.end(),
            result.output.begin() + static_cast<std::ptrdiff_t>(begin));
    }

    result.applied = true;
    return result;
}

bool GuardedPatchPlan::overlaps(
    const BytePatch& left,
    const BytePatch& right) noexcept {
    const auto left_end = left.offset +
        static_cast<std::uint64_t>(left.expected.size());
    const auto right_end = right.offset +
        static_cast<std::uint64_t>(right.expected.size());
    return left.offset < right_end && right.offset < left_end;
}

} // namespace dmc::rengine::patch
