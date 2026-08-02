#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::patch {

struct BytePatch final {
    std::uint64_t offset{};
    std::vector<std::byte> expected;
    std::vector<std::byte> replacement;
    std::string description;

    [[nodiscard]] bool valid() const noexcept {
        return !description.empty() && !expected.empty() &&
            expected.size() == replacement.size();
    }
};

struct PatchResult final {
    bool applied{false};
    std::vector<std::byte> output;
    std::vector<std::string> errors;
};

class GuardedPatchPlan final {
public:
    GuardedPatchPlan(std::string id, std::string target_sha256);

    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& target_sha256() const noexcept;
    [[nodiscard]] bool add(BytePatch patch);
    [[nodiscard]] const std::vector<BytePatch>& patches() const noexcept;
    [[nodiscard]] PatchResult apply(std::span<const std::byte> source) const;

private:
    [[nodiscard]] static bool overlaps(
        const BytePatch& left,
        const BytePatch& right) noexcept;

    std::string id_;
    std::string target_sha256_;
    std::vector<BytePatch> patches_;
};

} // namespace dmc::rengine::patch
