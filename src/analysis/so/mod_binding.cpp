#include "dmc_rengine/analysis/so/mod_binding.hpp"

namespace dmc::rengine::analysis::so {

ModBindingAnalysis analyze_mod_binding(
    const formats::mod::transform_domain::ParseResult& mod,
    const formats::so::link_table::ParseResult& links,
    const formats::so::volume_table::ParseResult& volumes) noexcept {
    ModBindingAnalysis result;
    result.domain_count = static_cast<std::size_t>(mod.raw_domain_count);
    result.post_prefix_link_count = links.records.empty() ? 0U : links.records.size() - 1U;
    result.volume_count = volumes.records.size();
    if (!mod.ok() || !links.ok() || !volumes.ok()) {
        return result;
    }

    result.link_middle_fields_fit_domain = !links.records.empty();
    for (std::size_t index = 1U; index < links.records.size(); ++index) {
        const auto& record = links.records[index];
        if (static_cast<std::size_t>(record.field1) >= result.domain_count ||
            static_cast<std::size_t>(record.field2) >= result.domain_count) {
            result.link_middle_fields_fit_domain = false;
            break;
        }
    }

    result.post_prefix_link_count_equals_domain = result.post_prefix_link_count == result.domain_count;
    result.volume_count_equals_domain = result.volume_count == result.domain_count;
    result.complete_cardinality_alignment = result.link_middle_fields_fit_domain &&
                                            result.post_prefix_link_count_equals_domain &&
                                            result.volume_count_equals_domain;
    return result;
}

} // namespace dmc::rengine::analysis::so
