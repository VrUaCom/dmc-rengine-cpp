#include "dmc_rengine/analysis/so/companion_correlation.hpp"

namespace dmc::rengine::analysis::so {

CompanionCorrelation correlate_companions(
    const formats::so::link_table::ParseResult& links,
    const formats::so::volume_table::ParseResult& volumes) noexcept {
    CompanionCorrelation result;
    result.link_record_count = links.records.size();
    result.volume_record_count = volumes.records.size();
    result.one_header_plus_one_link_per_volume =
        links.ok() && volumes.ok() && links.records.size() == volumes.records.size() + 1U;
    return result;
}

} // namespace dmc::rengine::analysis::so
