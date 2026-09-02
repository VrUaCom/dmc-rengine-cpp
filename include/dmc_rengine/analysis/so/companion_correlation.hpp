#pragma once

#include "dmc_rengine/formats/so/link_table.hpp"
#include "dmc_rengine/formats/so/volume_table.hpp"

#include <cstddef>

namespace dmc::rengine::analysis::so {

struct CompanionCorrelation final {
    bool one_header_plus_one_link_per_volume{false};
    std::size_t link_record_count{};
    std::size_t volume_record_count{};
};

[[nodiscard]] CompanionCorrelation correlate_companions(
    const formats::so::link_table::ParseResult& links,
    const formats::so::volume_table::ParseResult& volumes) noexcept;

} // namespace dmc::rengine::analysis::so
