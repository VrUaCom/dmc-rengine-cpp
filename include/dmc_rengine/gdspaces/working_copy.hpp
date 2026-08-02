#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

struct EditOperation final {
    std::string id;
    std::uint64_t base_revision{};
    std::uint64_t offset{};
    std::vector<std::byte> expected;
    std::vector<std::byte> replacement;
    std::string description;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !description.empty();
    }
};

struct EditResult final {
    bool applied{false};
    std::uint64_t revision{};
    std::string error;
};

class WorkingCopy final {
public:
    explicit WorkingCopy(ResourcePayload source);

    [[nodiscard]] const ResourceRef& resource() const noexcept;
    [[nodiscard]] const std::string& source_sha256() const noexcept;
    [[nodiscard]] std::uint64_t revision() const noexcept;
    [[nodiscard]] const std::vector<std::byte>& bytes() const noexcept;
    [[nodiscard]] const std::vector<EditOperation>& history() const noexcept;
    [[nodiscard]] bool dirty() const noexcept;

    [[nodiscard]] EditResult apply(EditOperation operation);
    [[nodiscard]] bool undo_last();
    void reset();

private:
    struct AppliedOperation final {
        EditOperation operation;
        std::vector<std::byte> previous;
    };

    ResourceRef resource_;
    std::vector<std::byte> original_;
    std::vector<std::byte> current_;
    std::string source_sha256_;
    std::uint64_t revision_{};
    std::vector<AppliedOperation> applied_;
    std::vector<EditOperation> public_history_;
};

} // namespace dmc::rengine::gdspaces
