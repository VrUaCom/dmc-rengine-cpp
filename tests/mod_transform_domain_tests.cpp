#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace domain = dmc::rengine::formats::mod::transform_domain;

namespace {
void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint64_t value) {
    put_u32(bytes, offset, static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
    put_u32(bytes, offset + 4U, static_cast<std::uint32_t>(value >> 32U));
}
} // namespace

int main() {
    std::vector<std::byte> bytes(0x90U);
    bytes[0U] = std::byte{'M'};
    bytes[1U] = std::byte{'O'};
    bytes[2U] = std::byte{'D'};
    bytes[3U] = std::byte{' '};
    bytes[0x11U] = std::byte{3U};
    put_u64(bytes, 0x20U, 0x40U);
    put_u32(bytes, 0x40U, 0x20U);
    put_u32(bytes, 0x44U, 0x24U);
    put_u32(bytes, 0x48U, 0x28U);
    put_u32(bytes, 0x4CU, 0x30U);

    bytes[0x60U] = std::byte{0xFFU};
    bytes[0x61U] = std::byte{0U};
    bytes[0x62U] = std::byte{1U};
    bytes[0x64U] = std::byte{0U};
    bytes[0x65U] = std::byte{1U};
    bytes[0x66U] = std::byte{2U};

    const auto result = domain::parse(bytes);
    assert(result.ok());
    assert(result.raw_domain_count == 3U);
    assert(result.permutation_is_complete);
    assert(result.hierarchy_candidate_is_acyclic);
    assert(result.derived_hierarchy_candidate.size() == 3U);
    assert(result.derived_hierarchy_candidate[0] == -1);
    assert(result.derived_hierarchy_candidate[1] == 0);
    assert(result.derived_hierarchy_candidate[2] == 1);
    return 0;
}
