#pragma once

#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct NbzOverlayMember final {
    std::string logical_path;
    std::vector<std::byte> bytes;
};

enum class NbzOverlayWriteStatus : std::uint8_t {
    ok,
    invalid_bootstrap,
    post_gap_volume_present,
    volume_index_out_of_domain,
    no_members,
    too_many_members,
    invalid_member_path,
    empty_member,
    duplicate_member_path,
    normalized_key_collision,
    member_too_large,
    archive_too_large,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    NbzOverlayWriteStatus status) noexcept {
    switch (status) {
    case NbzOverlayWriteStatus::ok: return "ok";
    case NbzOverlayWriteStatus::invalid_bootstrap: return "invalid-bootstrap";
    case NbzOverlayWriteStatus::post_gap_volume_present:
        return "post-gap-volume-present";
    case NbzOverlayWriteStatus::volume_index_out_of_domain:
        return "volume-index-out-of-domain";
    case NbzOverlayWriteStatus::no_members: return "no-members";
    case NbzOverlayWriteStatus::too_many_members: return "too-many-members";
    case NbzOverlayWriteStatus::invalid_member_path: return "invalid-member-path";
    case NbzOverlayWriteStatus::empty_member: return "empty-member";
    case NbzOverlayWriteStatus::duplicate_member_path:
        return "duplicate-member-path";
    case NbzOverlayWriteStatus::normalized_key_collision:
        return "normalized-key-collision";
    case NbzOverlayWriteStatus::member_too_large: return "member-too-large";
    case NbzOverlayWriteStatus::archive_too_large: return "archive-too-large";
    case NbzOverlayWriteStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct NbzOverlayMemberReceipt final {
    std::string logical_path;
    std::string normalized_archive_key;
    std::uint32_t crc32{};
    std::uint32_t size{};
    std::uint32_t local_header_offset{};
    std::uint32_t data_offset{};

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzOverlayWriteReceipt final {
    std::uint32_t volume_index{};
    std::string filename;
    std::string archive_sha256;
    std::uint64_t archive_size{};
    std::uint32_t central_offset{};
    std::uint32_t central_size{};
    std::vector<NbzOverlayMemberReceipt> members;

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzOverlayWriteResult final {
    NbzOverlayWriteStatus status{NbzOverlayWriteStatus::invalid_bootstrap};
    std::vector<std::byte> bytes;
    std::optional<NbzOverlayWriteReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == NbzOverlayWriteStatus::ok && receipt.has_value() &&
            receipt->valid() && receipt->archive_size == bytes.size();
    }
};

class NbzStoreOverlayWriter final {
public:
    static constexpr std::uint32_t zip32_u32_sentinel = 0xFFFFFFFFU;
    static constexpr std::uint16_t zip32_u16_sentinel = 0xFFFFU;
    static constexpr std::size_t max_generated_members =
        static_cast<std::size_t>(zip32_u16_sentinel) - 1U;

    // Builds a new DMC3-N.nbz product overlay using classic ZIP method 0
    // STORE. Original retail volumes are not modified. This consumes recovered
    // DMC3 mount/read behavior but does not claim Capcom archive-builder
    // equivalence.
    [[nodiscard]] static NbzOverlayWriteResult build(
        const VolumeBootstrapPlan& bootstrap,
        std::span<const NbzOverlayMember> members);
};

} // namespace dmc::rengine::profiles::dmc3
