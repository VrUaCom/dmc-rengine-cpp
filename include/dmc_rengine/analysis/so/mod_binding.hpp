#pragma once

#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/so/link_table.hpp"
#include "dmc_rengine/formats/so/volume_table.hpp"

#include <cstddef>

namespace dmc::rengine::analysis::so {

struct ModBindingAnalysis final {
    bool link_middle_fields_fit_domain{false};
    bool post_prefix_link_count_equals_domain{false};
    bool volume_count_equals_domain{false};
    bool complete_cardinality_alignment{false};
    std::size_t domain_count{};
    std::size_t post_prefix_link_count{};
    std::size_t volume_count{};
};

[[nodiscard]] ModBindingAnalysis analyze_mod_binding(
    const formats::mod::transform_domain::ParseResult& mod,
    const formats::so::link_table::ParseResult& links,
    const formats::so::volume_table::ParseResult& volumes) noexcept;

} // namespace dmc::rengine::analysis::so
