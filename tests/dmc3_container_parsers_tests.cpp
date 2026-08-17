#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

std::vector<std::byte> make_container(bool pnst) {
    std::vector<std::byte> bytes(0x30U, std::byte{0});
    if (pnst) {
        bytes[0] = std::byte{'P'};
        bytes[1] = std::byte{'N'};
        bytes[2] = std::byte{'S'};
        bytes[3] = std::byte{'T'};
    } else {
        bytes[0] = std::byte{'P'};
        bytes[1] = std::byte{'A'};
        bytes[2] = std::byte{'C'};
        bytes[3] = std::byte{0};
    }
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x18U);
    return bytes;
}

} // namespace

int main() {
    const auto registry =
        dmc::rengine::profiles::dmc3::make_container_parser_registry();
    assert(registry.size() == 2U);

    const auto pac = make_container(false);
    const auto* pac_parser = registry.select(
        std::span<const std::byte>{pac}, "DMC3/unknown.bin");
    assert(pac_parser != nullptr);
    assert(pac_parser->id() == "dmc3-pac-structural-v1");
    const auto pac_result = registry.parse(
        std::span<const std::byte>{pac}, "DMC3/unknown.bin");
    assert(pac_result.ok());
    assert(pac_result.document.format == "PAC");

    const auto pnst = make_container(true);
    const auto* pnst_parser = registry.select(
        std::span<const std::byte>{pnst}, "DMC3/misleading.pac");
    assert(pnst_parser != nullptr);
    assert(pnst_parser->id() == "dmc3-pnst-structural-v1");
    const auto pnst_result = registry.parse(
        std::span<const std::byte>{pnst}, "DMC3/misleading.pac");
    assert(pnst_result.ok());
    assert(pnst_result.document.format == "PNST");
    assert(pnst_result.document.entries[0].offset == 0x18U);

    auto malformed = pnst;
    put_u32(malformed, 8U, 0x08U);
    const auto malformed_result = registry.parse(
        std::span<const std::byte>{malformed}, "DMC3/malformed.bin");
    assert(malformed_result.recognized);
    assert(!malformed_result.ok());
    assert(!malformed_result.diagnostics.empty());
    assert(malformed_result.diagnostics.front().code ==
        "dmc3.pnst.slot_offset_before_payload");

    return 0;
}
