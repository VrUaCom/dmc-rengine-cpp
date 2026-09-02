#include "dmc_rengine/analysis/so/mod_binding.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

namespace analysis = dmc::rengine::analysis::so;
namespace links = dmc::rengine::formats::so::link_table;
namespace volumes = dmc::rengine::formats::so::volume_table;
namespace domain = dmc::rengine::formats::mod::transform_domain;

int main() {
    domain::ParseResult mod;
    mod.recognized = true;
    mod.raw_domain_count = 3U;
    mod.permutation_is_complete = true;
    mod.hierarchy_candidate_is_acyclic = true;

    const std::array<std::byte, 16> link_bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x01}, std::byte{0x02}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
    };
    std::vector<std::byte> volume_bytes(3U * volumes::record_size);

    const auto link_result = links::parse(link_bytes);
    const auto volume_result = volumes::parse(volume_bytes);
    const auto result = analysis::analyze_mod_binding(mod, link_result, volume_result);

    assert(result.link_middle_fields_fit_domain);
    assert(result.post_prefix_link_count_equals_domain);
    assert(result.volume_count_equals_domain);
    assert(result.complete_cardinality_alignment);
    return 0;
}
