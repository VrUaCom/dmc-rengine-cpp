#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::binary {

struct ByteRange final {
    std::uint64_t offset{};
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint64_t end() const noexcept;
    [[nodiscard]] bool within(std::uint64_t total_size) const noexcept;
    [[nodiscard]] bool overlaps(const ByteRange& other) const noexcept;

    friend bool operator==(const ByteRange&, const ByteRange&) = default;
};

enum class RegionKind {
    header,
    table,
    record,
    payload,
    padding,
    unknown,
};

[[nodiscard]] constexpr std::string_view to_string(RegionKind kind) noexcept {
    switch (kind) {
    case RegionKind::header: return "header";
    case RegionKind::table: return "table";
    case RegionKind::record: return "record";
    case RegionKind::payload: return "payload";
    case RegionKind::padding: return "padding";
    case RegionKind::unknown: return "unknown";
    }
    return "unknown";
}

struct Region final {
    std::string id;
    std::string name;
    ByteRange range;
    RegionKind kind{RegionKind::unknown};
    std::string type_name;
    std::string evidence_id;

    [[nodiscard]] bool valid() const noexcept;
};

struct OwnershipClaim final {
    std::string owner_id;
    ByteRange range;
    std::string rationale;

    [[nodiscard]] bool valid() const noexcept;
};

struct RegionConflict final {
    std::string left_region_id;
    std::string right_region_id;
    ByteRange overlap;
};

class Document final {
public:
    Document(gdspaces::ResourceRef resource, std::uint64_t byte_size);

    [[nodiscard]] const gdspaces::ResourceRef& resource() const noexcept;
    [[nodiscard]] std::uint64_t byte_size() const noexcept;
    [[nodiscard]] bool add_region(Region region);
    [[nodiscard]] bool add_ownership(OwnershipClaim claim);
    [[nodiscard]] const Region* find_region(std::string_view id) const noexcept;
    [[nodiscard]] const std::vector<Region>& regions() const noexcept;
    [[nodiscard]] const std::vector<OwnershipClaim>& ownership() const noexcept;
    [[nodiscard]] std::uint64_t coverage_bytes() const;
    [[nodiscard]] std::vector<ByteRange> unknown_ranges() const;
    [[nodiscard]] std::vector<RegionConflict> conflicts() const;

private:
    gdspaces::ResourceRef resource_;
    std::uint64_t byte_size_{};
    std::vector<Region> regions_;
    std::vector<OwnershipClaim> ownership_;
};

} // namespace dmc::rengine::binary
