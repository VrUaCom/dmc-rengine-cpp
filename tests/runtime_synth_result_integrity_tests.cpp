#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_synth_relative_slot_writer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

std::string sha256_of(const std::vector<std::byte>& bytes) {
    return dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{bytes.data(), bytes.size()}).hex();
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace gdspaces = dmc::rengine::gdspaces;

    std::vector<std::byte> source_bytes(0x60U, std::byte{0});
    source_bytes[0] = std::byte{'P'};
    source_bytes[1] = std::byte{'A'};
    source_bytes[2] = std::byte{'C'};
    source_bytes[3] = std::byte{0};
    put_u32(source_bytes, 4U, 2U);
    put_u32(source_bytes, 8U, 0x20U);
    put_u32(source_bytes, 12U, 0x40U);
    source_bytes[0x20U] = std::byte{'A'};
    source_bytes[0x40U] = std::byte{'B'};

    const auto size = static_cast<std::uint64_t>(source_bytes.size());
    gdspaces::ResourcePayload source{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "runtime-synth-integrity-source",
                .logical_path = "GData.afs/integrity.pac",
                .container_chain = "nbz[0]",
                .offset = 0x4000U,
                .size = size,
            },
            .display_name = "integrity.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = source_bytes,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };

    std::vector<std::byte> child0{std::byte{1}, std::byte{2}, std::byte{3}};
    std::vector<std::byte> child1{std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}};
    const std::vector<dmc3::ExactChildImage> children{
        dmc3::ExactChildImage{
            .slot_index = 0U,
            .authority_kind = dmc3::ExactChildAuthorityKind::external_exact_resource,
            .authority_id = "external:child0",
            .sha256 = sha256_of(child0),
            .bytes = child0,
        },
        dmc3::ExactChildImage{
            .slot_index = 1U,
            .authority_kind = dmc3::ExactChildAuthorityKind::external_exact_resource,
            .authority_id = "external:child1",
            .sha256 = sha256_of(child1),
            .bytes = child1,
        },
    };

    const auto built = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(source, children);
    assert(built.ok());

    auto mutated_bytes = built;
    mutated_bytes.bytes.back() ^= std::byte{1};
    assert(!mutated_bytes.ok());

    auto reordered_receipt = *built.receipt;
    std::swap(reordered_receipt.children[0], reordered_receipt.children[1]);
    assert(!reordered_receipt.valid());

    auto duplicate_receipt = *built.receipt;
    duplicate_receipt.children[1].slot_index = duplicate_receipt.children[0].slot_index;
    assert(!duplicate_receipt.valid());

    return 0;
}
