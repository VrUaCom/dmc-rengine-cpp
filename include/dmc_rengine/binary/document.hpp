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
    [[nodiscard]] bool contains(std::uint64_t position) const noexcept;
    [[nodiscard]] bool contains(const ByteRange& other) const noexcept;

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

enum class FieldKind {
    unsigned_integer,
    signed_integer,
    floating_point,
    boolean,
    string,
    bytes,
    pointer,
    enumeration,
    structure,
    array,
    unknown,
};

[[nodiscard]] constexpr std::string_view to_string(FieldKind kind) noexcept {
    switch (kind) {
    case FieldKind::unsigned_integer: return "unsigned-integer";
    case FieldKind::signed_integer: return "signed-integer";
    case FieldKind::floating_point: return "floating-point";
    case FieldKind::boolean: return "boolean";
    case FieldKind::string: return "string";
    case FieldKind::bytes: return "bytes";
    case FieldKind::pointer: return "pointer";
    case FieldKind::enumeration: return "enumeration";
    case FieldKind::structure: return "structure";
    case FieldKind::array: return "array";
    case FieldKind::unknown: return "unknown";
    }
    return "unknown";
}

struct Field final {
    std::string id;
    std::string name;
    ByteRange range;
    FieldKind kind{FieldKind::unknown};
    std::string type_name;
    std::string display_value;
    std::string parent_id;
    std::string evidence_id;

    [[nodiscard]] bool valid() const noexcept;
};

struct OwnershipClaim final {
    std::string owner_id;
    ByteRange range;
    std::string rationale;

    [[nodiscard]] bool valid() const noexcept;
};

struct Annotation final {
    std::string id;
    ByteRange range;
    std::string text;
    std::string evidence_id;
    std::vector<std::string> tags;

    [[nodiscard]] bool valid() const noexcept;
};

struct RegionConflict final {
    std::string left_region_id;
    std::string right_region_id;
    ByteRange overlap;
};

struct OwnershipConflict final {
    std::string left_owner_id;
    std::string right_owner_id;
    ByteRange overlap;
};

struct SelectionContext final {
    std::vector<const Region*> regions;
    std::vector<const Field*> fields;
    std::vector<const OwnershipClaim*> owners;
    std::vector<const Annotation*> annotations;
};

class Document final {
public:
    Document(gdspaces::ResourceRef resource, std::uint64_t byte_size);

    [[nodiscard]] const gdspaces::ResourceRef& resource() const noexcept;
    [[nodiscard]] std::uint64_t byte_size() const noexcept;
    [[nodiscard]] bool add_region(Region region);
    [[nodiscard]] bool add_field(Field field);
    [[nodiscard]] bool add_ownership(OwnershipClaim claim);
    [[nodiscard]] bool add_annotation(Annotation annotation);

    [[nodiscard]] const Region* find_region(std::string_view id) const noexcept;
    [[nodiscard]] const Field* find_field(std::string_view id) const noexcept;
    [[nodiscard]] const Annotation* find_annotation(
        std::string_view id) const noexcept;

    [[nodiscard]] const std::vector<Region>& regions() const noexcept;
    [[nodiscard]] const std::vector<Field>& fields() const noexcept;
    [[nodiscard]] const std::vector<OwnershipClaim>& ownership() const noexcept;
    [[nodiscard]] const std::vector<Annotation>& annotations() const noexcept;

    [[nodiscard]] std::vector<const Field*> child_fields(
        std::string_view parent_id) const;
    [[nodiscard]] std::vector<const Field*> fields_at(
        std::uint64_t offset) const;
    [[nodiscard]] std::vector<const OwnershipClaim*> owners_at(
        std::uint64_t offset) const;
    [[nodiscard]] std::vector<const Annotation*> annotations_at(
        std::uint64_t offset) const;
    [[nodiscard]] SelectionContext selection_at(std::uint64_t offset) const;

    [[nodiscard]] std::uint64_t coverage_bytes() const;
    [[nodiscard]] std::vector<ByteRange> unknown_ranges() const;
    [[nodiscard]] std::vector<RegionConflict> conflicts() const;
    [[nodiscard]] std::vector<OwnershipConflict> ownership_conflicts() const;

private:
    gdspaces::ResourceRef resource_;
    std::uint64_t byte_size_{};
    std::vector<Region> regions_;
    std::vector<Field> fields_;
    std::vector<OwnershipClaim> ownership_;
    std::vector<Annotation> annotations_;
};

} // namespace dmc::rengine::binary
