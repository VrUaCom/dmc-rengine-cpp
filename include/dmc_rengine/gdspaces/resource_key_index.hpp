#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourceKeyIndexBuildReceipt final {
    std::string source_id;
    std::uint32_t normalization_flags{};
    std::size_t input_count{};
    std::size_t indexed_count{};
    std::size_t rejected_invalid_resource_count{};
    std::size_t rejected_source_mismatch_count{};
    std::size_t rejected_c_string_count{};
    std::size_t rejected_empty_key_count{};
    std::size_t duplicate_identity_count{};
    std::size_t conflict_key_count{};

    [[nodiscard]] bool valid() const noexcept;
};

struct ResourceKeyMatchReport final {
    std::string provider_key;
    bool index_valid{false};
    bool key_valid{false};
    std::vector<ResourceRef> matches;

    [[nodiscard]] bool found() const noexcept {
        return index_valid && key_valid && !matches.empty();
    }

    [[nodiscard]] bool unique() const noexcept {
        return index_valid && key_valid && matches.size() == 1U;
    }

    [[nodiscard]] bool ambiguous() const noexcept {
        return index_valid && key_valid && matches.size() > 1U;
    }
};

// Derived product index over immutable ResourceRef values from exactly one
// source. It owns provider-normalized lookup policy without changing ISource's
// exact enumerate/read/materialization authority.
class ResourceKeyIndex final {
public:
    [[nodiscard]] static ResourceKeyIndex build(
        std::string source_id,
        std::uint32_t normalization_flags,
        std::span<const ResourceRef> resources);

    [[nodiscard]] ResourceKeyMatchReport lookup(
        std::string_view provider_key) const;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::string_view source_id() const noexcept;
    [[nodiscard]] std::uint32_t normalization_flags() const noexcept;
    [[nodiscard]] const ResourceKeyIndexBuildReceipt& receipt() const noexcept;

private:
    struct Entry final {
        std::string key;
        std::string identity_key;
        ResourceRef resource;
    };

    std::string source_id_;
    std::uint32_t normalization_flags_{};
    std::vector<Entry> entries_;
    ResourceKeyIndexBuildReceipt receipt_;
};

} // namespace dmc::rengine::gdspaces
