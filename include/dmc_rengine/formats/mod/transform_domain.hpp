#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::mod::transform_domain {

struct ParseResult final {
    bool recognized{false};
    std::uint8_t raw_domain_count{};
    std::uint64_t document_offset{};
    std::vector<std::uint8_t> reference_table;
    std::vector<std::uint8_t> permutation_table;
    std::vector<std::int16_t> derived_hierarchy_candidate;
    bool permutation_is_complete{false};
    bool hierarchy_candidate_is_acyclic{false};
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::mod::transform_domain
