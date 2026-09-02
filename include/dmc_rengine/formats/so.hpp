#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::so {

inline constexpr std::size_t type6_header_size = 0x0EU;
inline constexpr std::size_t type8_header_size = 0x08U;
inline constexpr std::size_t link_record_size = 0x04U;
inline constexpr std::size_t volume_record_size = 0x50U;

struct Diagnostic final {
    std::string message;
};

struct IndexedBlock final {
    std::uint16_t type{};
    std::uint64_t base_offset{};
    std::uint64_t extent_size{};
    std::vector<std::uint16_t> header_words;
    std::vector<std::uint16_t> entry_offsets;
};

struct GraphParseResult final {
    bool recognized{false};
    std::vector<IndexedBlock> blocks;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct LinkRecord final {
    std::uint8_t field0{};
    std::uint8_t field1{};
    std::uint8_t field2{};
    std::uint8_t field3{};
};

struct LinkParseResult final {
    bool recognized{false};
    std::vector<LinkRecord> records;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct Vec4 final {
    float x{};
    float y{};
    float z{};
    float w{};
};

struct VolumeRecord final {
    std::uint32_t type{};
    std::array<std::byte, 12> prefix_unknown{};
    Vec4 vector0{};
    Vec4 vector1{};
    Vec4 vector2{};
    Vec4 vector3{};
};

struct VolumeParseResult final {
    bool recognized{false};
    std::vector<VolumeRecord> records;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct CompanionCorrelation final {
    bool one_header_plus_one_link_per_volume{false};
    std::size_t link_record_count{};
    std::size_t volume_record_count{};
};

// Minimal, evidence-bounded view of the MOD document region needed to test the
// SO companion relationship. The table names stay deliberately semantic-neutral
// until their exact runtime ownership is confirmed in dmc3.exe.
struct ModTransformDomain final {
    bool recognized{false};
    std::uint8_t raw_domain_count{};
    std::uint64_t document_offset{};
    std::vector<std::uint8_t> reference_table;
    std::vector<std::uint8_t> permutation_table;
    std::vector<std::int16_t> derived_hierarchy_candidate;
    bool permutation_is_complete{false};
    bool hierarchy_candidate_is_acyclic{false};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct ModCompanionCorrelation final {
    bool link_middle_fields_fit_domain{false};
    bool post_prefix_link_count_equals_domain{false};
    bool volume_count_equals_domain{false};
    bool complete_cardinality_alignment{false};
    std::size_t domain_count{};
    std::size_t post_prefix_link_count{};
    std::size_t volume_count{};
};

[[nodiscard]] GraphParseResult parse_graph(std::span<const std::byte> bytes);
[[nodiscard]] LinkParseResult parse_links(std::span<const std::byte> bytes);
[[nodiscard]] VolumeParseResult parse_volumes(std::span<const std::byte> bytes);
[[nodiscard]] CompanionCorrelation correlate_companions(const LinkParseResult& links,
                                                        const VolumeParseResult& volumes) noexcept;
[[nodiscard]] ModTransformDomain parse_mod_transform_domain(std::span<const std::byte> bytes);
[[nodiscard]] ModCompanionCorrelation correlate_mod_companions(const ModTransformDomain& mod,
                                                               const LinkParseResult& links,
                                                               const VolumeParseResult& volumes) noexcept;

} // namespace dmc::rengine::formats::so
