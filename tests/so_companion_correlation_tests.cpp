#include "dmc_rengine/analysis/so/companion_correlation.hpp"

#include "so_test_fixture.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

namespace analysis = dmc::rengine::analysis::so;
namespace links = dmc::rengine::formats::so::link_table;
namespace volumes = dmc::rengine::formats::so::volume_table;

int main() {
    const std::array<std::byte, 8> link_bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00},
    };
    const auto volume_bytes = so_test_fixture::volume_records(1U);

    const auto link_result = links::parse(link_bytes);
    const auto volume_result = volumes::parse(volume_bytes);
    const auto correlation = analysis::correlate_companions(link_result, volume_result);

    assert(correlation.link_record_count == 2U);
    assert(correlation.volume_record_count == 1U);
    assert(correlation.one_header_plus_one_link_per_volume);
    return 0;
}
