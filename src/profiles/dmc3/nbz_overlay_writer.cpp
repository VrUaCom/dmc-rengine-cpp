#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::uint32_t k_local_signature = 0x04034B50U;
constexpr std::uint32_t k_central_signature = 0x02014B50U;
constexpr std::uint32_t k_eocd_signature = 0x06054B50U;
constexpr std::uint16_t k_version_20 = 20U;
constexpr std::uint16_t k_store_method = 0U;
constexpr std::uint16_t k_flags = 0U;
constexpr std::uint16_t k_dos_time = 0U;
constexpr std::uint16_t k_dos_date_1980_01_01 = 0x0021U;
constexpr std::uint32_t k_dos_archive_attribute = 0x20U;
constexpr std::uint64_t k_local_fixed_size = 30U;
constexpr std::uint64_t k_central_fixed_size = 46U;
constexpr std::uint64_t k_eocd_size = 22U;

struct PreparedMember final {
    const NbzOverlayMember* source{};
    std::string normalized_key;
    std::uint32_t crc32{};
};

void append_u16(std::vector<std::byte>& output, std::uint16_t value) {
    output.push_back(static_cast<std::byte>(value & 0xFFU));
    output.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

void append_u32(std::vector<std::byte>& output, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        output.push_back(static_cast<std::byte>((value >> shift) & 0xFFU));
    }
}

void append_text(std::vector<std::byte>& output, std::string_view text) {
    output.reserve(output.size() + text.size());
    for (const char value : text) {
        output.push_back(static_cast<std::byte>(value));
    }
}

void append_bytes(
    std::vector<std::byte>& output,
    std::span<const std::byte> bytes) {
    output.insert(output.end(), bytes.begin(), bytes.end());
}

[[nodiscard]] std::uint32_t crc32_of(std::span<const std::byte> bytes) noexcept {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const auto value : bytes) {
        crc ^= std::to_integer<std::uint8_t>(value);
        for (unsigned bit = 0U; bit < 8U; ++bit) {
            const auto mask = static_cast<std::uint32_t>(
                0U - static_cast<std::uint32_t>(crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

[[nodiscard]] bool fits_generated_u32(std::uint64_t value) noexcept {
    return value <
        static_cast<std::uint64_t>(NbzStoreOverlayWriter::zip32_u32_sentinel);
}

[[nodiscard]] NbzOverlayWriteResult failure(
    NbzOverlayWriteStatus status,
    std::string detail) {
    return NbzOverlayWriteResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] bool ordinary_member_path(std::string_view path) noexcept {
    if (path.empty() || !ResourcePathPolicy::valid_input(path) ||
        path.size() >
            static_cast<std::size_t>(
                std::numeric_limits<std::uint16_t>::max())) {
        return false;
    }
    return path.back() != '/' && path.back() != '\\';
}

} // namespace

bool NbzOverlayMemberReceipt::valid() const noexcept {
    if (!ordinary_member_path(logical_path) || normalized_archive_key.empty() ||
        size == 0U) {
        return false;
    }
    const auto expected_key = ResourcePathPolicy::archive(logical_path);
    if (expected_key.empty() || expected_key != normalized_archive_key) {
        return false;
    }
    const auto expected_data =
        static_cast<std::uint64_t>(local_header_offset) + k_local_fixed_size +
        static_cast<std::uint64_t>(logical_path.size());
    return fits_generated_u32(expected_data) &&
        data_offset == static_cast<std::uint32_t>(expected_data) &&
        fits_generated_u32(
            static_cast<std::uint64_t>(data_offset) + size);
}

bool NbzOverlayWriteReceipt::valid() const noexcept {
    if (!VolumeBootstrapPolicy::runtime_index_valid(volume_index) ||
        filename != VolumeBootstrapPolicy::volume_filename(volume_index) ||
        archive_sha256.size() != 64U || members.empty() ||
        members.size() > NbzStoreOverlayWriter::max_generated_members ||
        !fits_generated_u32(archive_size) ||
        !fits_generated_u32(central_offset) ||
        !fits_generated_u32(central_size)) {
        return false;
    }

    std::uint64_t expected_local_offset = 0U;
    std::uint64_t expected_central_size = 0U;
    std::string_view previous_key;
    bool first = true;
    for (const auto& member : members) {
        if (!member.valid() || member.local_header_offset != expected_local_offset) {
            return false;
        }
        if (!first && !(previous_key < member.normalized_archive_key)) {
            return false;
        }
        first = false;
        previous_key = member.normalized_archive_key;
        expected_local_offset =
            static_cast<std::uint64_t>(member.data_offset) + member.size;
        expected_central_size +=
            k_central_fixed_size +
            static_cast<std::uint64_t>(member.logical_path.size());
    }

    if (expected_local_offset != central_offset ||
        expected_central_size != central_size) {
        return false;
    }
    return static_cast<std::uint64_t>(central_offset) + central_size +
            k_eocd_size == archive_size;
}

NbzOverlayWriteResult NbzStoreOverlayWriter::build(
    const VolumeBootstrapPlan& bootstrap,
    std::span<const NbzOverlayMember> members) {
    if (!bootstrap.valid()) {
        return failure(
            NbzOverlayWriteStatus::invalid_bootstrap,
            "Overlay authoring requires a valid recovered numbered-volume discovery plan.");
    }
    if (!bootstrap.present_after_first_gap.empty()) {
        return failure(
            NbzOverlayWriteStatus::post_gap_volume_present,
            "Filling the first numbered-volume discovery gap would activate already-present later runtime filenames.");
    }
    if (!VolumeBootstrapPolicy::runtime_index_valid(
            bootstrap.first_missing_index)) {
        return failure(
            NbzOverlayWriteStatus::volume_index_out_of_domain,
            "The next contiguous discovered filename lies outside the recovered non-negative signed %d namespace.");
    }
    if (members.empty()) {
        return failure(
            NbzOverlayWriteStatus::no_members,
            "A generated overlay must contain at least one modified root resource.");
    }
    if (members.size() > max_generated_members) {
        return failure(
            NbzOverlayWriteStatus::too_many_members,
            "Generated member count reaches the classic-ZIP 0xFFFF sentinel domain.");
    }

    std::set<std::string> raw_paths;
    std::set<std::string> normalized_keys;
    std::vector<PreparedMember> prepared;
    prepared.reserve(members.size());

    for (const auto& member : members) {
        if (!ordinary_member_path(member.logical_path)) {
            return failure(
                NbzOverlayWriteStatus::invalid_member_path,
                "Generated NBZ members require non-empty C-string-compatible ordinary file paths within the ZIP16 name bound.");
        }
        if (member.bytes.empty()) {
            return failure(
                NbzOverlayWriteStatus::empty_member,
                "The first DMC3 overlay writer does not emit zero-size runtime resources.");
        }
        if (!fits_generated_u32(member.bytes.size())) {
            return failure(
                NbzOverlayWriteStatus::member_too_large,
                "Generated member size reaches the unsupported classic-ZIP 32-bit sentinel domain.");
        }
        if (!raw_paths.insert(member.logical_path).second) {
            return failure(
                NbzOverlayWriteStatus::duplicate_member_path,
                "Generated overlay contains a duplicate raw member path.");
        }

        auto normalized = ResourcePathPolicy::archive(member.logical_path);
        if (normalized.empty()) {
            return failure(
                NbzOverlayWriteStatus::invalid_member_path,
                "Generated member cannot be represented by the canonical DMC3 archive-key profile.");
        }
        if (!normalized_keys.insert(normalized).second) {
            return failure(
                NbzOverlayWriteStatus::normalized_key_collision,
                "Two generated member paths collapse to one DMC3 0x0E archive key; no semantic duplicate winner is invented.");
        }

        prepared.push_back(PreparedMember{
            .source = &member,
            .normalized_key = std::move(normalized),
            .crc32 = crc32_of(std::span<const std::byte>{
                member.bytes.data(), member.bytes.size()}),
        });
    }

    std::sort(
        prepared.begin(), prepared.end(),
        [](const PreparedMember& left, const PreparedMember& right) {
            if (left.normalized_key != right.normalized_key) {
                return left.normalized_key < right.normalized_key;
            }
            return left.source->logical_path < right.source->logical_path;
        });

    std::uint64_t local_bytes = 0U;
    std::uint64_t central_bytes = 0U;
    for (const auto& item : prepared) {
        const auto name_size =
            static_cast<std::uint64_t>(item.source->logical_path.size());
        local_bytes += k_local_fixed_size + name_size + item.source->bytes.size();
        central_bytes += k_central_fixed_size + name_size;
        if (!fits_generated_u32(local_bytes) ||
            !fits_generated_u32(central_bytes)) {
            return failure(
                NbzOverlayWriteStatus::archive_too_large,
                "Generated local or central byte domain exceeds the classic-ZIP 32-bit envelope.");
        }
    }

    const auto total_size = local_bytes + central_bytes + k_eocd_size;
    if (!fits_generated_u32(total_size)) {
        return failure(
            NbzOverlayWriteStatus::archive_too_large,
            "Generated NBZ archive reaches the unsupported classic-ZIP 32-bit sentinel domain.");
    }

    std::vector<std::byte> output;
    output.reserve(static_cast<std::size_t>(total_size));
    std::vector<NbzOverlayMemberReceipt> receipts;
    receipts.reserve(prepared.size());

    for (const auto& item : prepared) {
        const auto& member = *item.source;
        const auto local_offset = static_cast<std::uint32_t>(output.size());
        const auto member_size = static_cast<std::uint32_t>(member.bytes.size());
        const auto name_size = static_cast<std::uint16_t>(member.logical_path.size());

        append_u32(output, k_local_signature);
        append_u16(output, k_version_20);
        append_u16(output, k_flags);
        append_u16(output, k_store_method);
        append_u16(output, k_dos_time);
        append_u16(output, k_dos_date_1980_01_01);
        append_u32(output, item.crc32);
        append_u32(output, member_size);
        append_u32(output, member_size);
        append_u16(output, name_size);
        append_u16(output, 0U); // local extra length
        append_text(output, member.logical_path);
        const auto data_offset = static_cast<std::uint32_t>(output.size());
        append_bytes(
            output,
            std::span<const std::byte>{
                member.bytes.data(), member.bytes.size()});

        receipts.push_back(NbzOverlayMemberReceipt{
            .logical_path = member.logical_path,
            .normalized_archive_key = item.normalized_key,
            .crc32 = item.crc32,
            .size = member_size,
            .local_header_offset = local_offset,
            .data_offset = data_offset,
        });
    }

    const auto central_offset = static_cast<std::uint32_t>(output.size());
    for (std::size_t index = 0U; index < prepared.size(); ++index) {
        const auto& item = prepared[index];
        const auto& member = *item.source;
        const auto& receipt = receipts[index];
        const auto name_size = static_cast<std::uint16_t>(member.logical_path.size());

        append_u32(output, k_central_signature);
        append_u16(output, k_version_20); // product metadata, host 0
        append_u16(output, k_version_20);
        append_u16(output, k_flags);
        append_u16(output, k_store_method);
        append_u16(output, k_dos_time);
        append_u16(output, k_dos_date_1980_01_01);
        append_u32(output, item.crc32);
        append_u32(output, receipt.size);
        append_u32(output, receipt.size);
        append_u16(output, name_size);
        append_u16(output, 0U); // central extra length
        append_u16(output, 0U); // comment length
        append_u16(output, 0U); // disk start
        append_u16(output, 0U); // internal attributes
        append_u32(output, k_dos_archive_attribute);
        append_u32(output, receipt.local_header_offset);
        append_text(output, member.logical_path);
    }

    const auto central_size = static_cast<std::uint32_t>(
        output.size() - static_cast<std::size_t>(central_offset));
    const auto count = static_cast<std::uint16_t>(prepared.size());
    append_u32(output, k_eocd_signature);
    append_u16(output, 0U); // disk number
    append_u16(output, 0U); // central-directory disk
    append_u16(output, count);
    append_u16(output, count);
    append_u32(output, central_size);
    append_u32(output, central_offset);
    append_u16(output, 0U); // archive comment length

    const auto archive_sha = core::Sha256::compute(
        std::span<const std::byte>{output.data(), output.size()}).hex();
    NbzOverlayWriteReceipt receipt{
        .volume_index = bootstrap.first_missing_index,
        .filename = VolumeBootstrapPolicy::volume_filename(
            bootstrap.first_missing_index),
        .archive_sha256 = archive_sha,
        .archive_size = static_cast<std::uint64_t>(output.size()),
        .central_offset = central_offset,
        .central_size = central_size,
        .members = std::move(receipts),
    };
    if (!receipt.valid()) {
        return failure(
            NbzOverlayWriteStatus::invalid_receipt,
            "Generated NBZ bytes failed deterministic writer-receipt validation.");
    }

    return NbzOverlayWriteResult{
        .status = NbzOverlayWriteStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
