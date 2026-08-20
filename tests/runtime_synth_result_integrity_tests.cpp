#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_synth_relative_slot_writer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
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

dmc::rengine::profiles::dmc3::ExactChildImage intrinsic_child(
    std::uint32_t slot,
    std::string authority,
    std::vector<std::byte> bytes) {
    auto child = dmc::rengine::profiles::dmc3::ExactChildImage::
        from_intrinsic_resource(
            slot,
            dmc::rengine::profiles::dmc3::
                ExactChildAuthorityKind::external_exact_resource,
            std::move(authority),
            std::move(bytes));
    assert(child.has_value());
    return std::move(*child);
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace gdspaces = dmc::rengine::gdspaces;

    static_assert(!std::is_aggregate_v<dmc3::ExactChildImage>);
    static_assert(!std::is_default_constructible_v<dmc3::ExactChildImage>);
    static_assert(!std::is_aggregate_v<dmc3::RuntimeSynthChildReceipt>);
    static_assert(
        !std::is_default_constructible_v<dmc3::RuntimeSynthChildReceipt>);

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

    const std::vector<dmc3::ExactChildImage> children{
        intrinsic_child(
            0U,
            "external:child0",
            {std::byte{1}, std::byte{2}, std::byte{3}}),
        intrinsic_child(
            1U,
            "external:child1",
            {std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}}),
    };

    const auto built = dmc3::RuntimeSynthRelativeSlotWriter::rebuild(
        source, children);
    assert(built.ok());

    // Any byte mutation invalidates the construction-time output hash.
    auto mutated_bytes = built;
    mutated_bytes.bytes.back() ^= std::byte{1};
    assert(!mutated_bytes.ok());

    // Even if a caller rewrites the public top-level output hash after changing
    // a child byte, the immutable child receipt still binds the embedded child
    // span and RuntimeSynthResult::ok() rejects it.
    auto mutated_child = built;
    const auto child0_offset = static_cast<std::size_t>(
        mutated_child.receipt->children[0].emitted_offset());
    mutated_child.bytes[child0_offset] ^= std::byte{1};
    mutated_child.receipt->output_sha256 = sha256_of(mutated_child.bytes);
    assert(!mutated_child.ok());

    auto reordered_receipt = *built.receipt;
    std::swap(reordered_receipt.children[0], reordered_receipt.children[1]);
    assert(!reordered_receipt.valid());

    auto duplicate_receipt = *built.receipt;
    duplicate_receipt.children[1] = duplicate_receipt.children[0];
    assert(!duplicate_receipt.valid());

    // A writer child cannot be manufactured from an invalid/self-declared
    // result. The factory requires a live RuntimeSynthResult::ok() receipt.
    const dmc3::RuntimeSynthResult invalid_result{};
    assert(!dmc3::ExactChildImage::from_verified_runtime_synth_result(
        0U, invalid_result).has_value());

    return 0;
}
