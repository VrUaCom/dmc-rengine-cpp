#include "dmc_rengine/profiles/dmc3/relative_slot_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/formats/relative_slot_container.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

enum class RelativeSlotKind : std::uint8_t {
    pac,
    pnst,
};

[[nodiscard]] bool has_magic(
    std::span<const std::byte> bytes,
    const char (&magic)[5]) noexcept {
    if (bytes.size() < 4U) {
        return false;
    }
    for (std::size_t index = 0U; index < 4U; ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(magic[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::optional<RelativeSlotKind> detect_kind(
    std::span<const std::byte> bytes) noexcept {
    if (has_magic(bytes, "PAC\0")) {
        return RelativeSlotKind::pac;
    }
    if (has_magic(bytes, "PNST")) {
        return RelativeSlotKind::pnst;
    }
    return std::nullopt;
}

[[nodiscard]] formats::RelativeSlotParseResult parse_as(
    RelativeSlotKind kind,
    std::span<const std::byte> bytes) {
    if (kind == RelativeSlotKind::pac) {
        return formats::PacParser::parse(bytes);
    }
    return formats::PnstParser::parse(bytes);
}

[[nodiscard]] RelativeSlotTopology topology_of(
    const formats::ContainerDocument& document) {
    RelativeSlotTopology topology{
        .format = document.format,
        .declared_slot_count = document.declared_slot_count,
        .container_size = document.container_size,
        .protected_prefix_size = document.container_size,
        .entries = {},
    };
    topology.entries.reserve(document.entries.size());

    for (const auto& entry : document.entries) {
        topology.entries.push_back(RelativeSlotTopologyEntry{
            .slot_index = entry.slot_index,
            .offset = entry.offset,
            .size = entry.size,
            .populated = entry.populated,
        });
        if (entry.populated) {
            topology.protected_prefix_size = std::min(
                topology.protected_prefix_size,
                entry.offset);
        }
    }
    return topology;
}

[[nodiscard]] RelativeSlotRebuildResult failure(
    RelativeSlotRebuildStatus status,
    std::string detail) {
    return RelativeSlotRebuildResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

} // namespace

bool RelativeSlotTopology::valid() const noexcept {
    if ((format != "PAC" && format != "PNST") ||
        entries.size() != static_cast<std::size_t>(declared_slot_count) ||
        protected_prefix_size > container_size || container_size < 8U) {
        return false;
    }

    std::uint64_t expected_prefix = container_size;
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (entry.slot_index != index) {
            return false;
        }
        if (!entry.populated) {
            if (entry.offset != 0U || entry.size != 0U) {
                return false;
            }
            continue;
        }
        if (entry.offset >= container_size ||
            entry.size > container_size - entry.offset) {
            return false;
        }
        expected_prefix = std::min(expected_prefix, entry.offset);
    }
    return protected_prefix_size == expected_prefix;
}

bool LayoutPreservingRebuildReceipt::valid() const noexcept {
    return resource.valid() && source_sha256.size() == 64U &&
        output_sha256.size() == 64U && source_topology.valid() &&
        output_topology.valid() && source_topology == output_topology &&
        resource.size == source_topology.container_size;
}

RelativeSlotRebuildResult RelativeSlotLayoutWriter::rebuild(
    const gdspaces::ResourcePayload& source,
    const gdspaces::WorkingCopy& working_copy) {
    if (!source.readable() ||
        source.resource.id.size !=
            static_cast<std::uint64_t>(source.bytes.size())) {
        return failure(
            RelativeSlotRebuildStatus::invalid_source,
            "Source payload is unreadable or its ResourceId size does not match the materialized bytes.");
    }
    if (working_copy.resource().id != source.resource.id) {
        return failure(
            RelativeSlotRebuildStatus::resource_mismatch,
            "WorkingCopy belongs to a different canonical resource identity.");
    }

    const auto source_span = std::span<const std::byte>{
        source.bytes.data(), source.bytes.size()};
    const auto source_sha = sha256_of(source_span);
    if (working_copy.source_sha256() != source_sha) {
        return failure(
            RelativeSlotRebuildStatus::source_hash_mismatch,
            "WorkingCopy source SHA-256 does not match the supplied immutable source bytes.");
    }

    const auto kind = detect_kind(source_span);
    if (!kind.has_value()) {
        return failure(
            RelativeSlotRebuildStatus::unsupported_format,
            "Layout-preserving writer supports only materialized PAC\\0 and PNST images.");
    }

    const auto& output_bytes = working_copy.bytes();
    if (output_bytes.size() != source.bytes.size()) {
        return failure(
            RelativeSlotRebuildStatus::size_changed,
            "Layout-preserving mode requires an unchanged materialized container byte count.");
    }

    const auto source_parsed = parse_as(*kind, source_span);
    if (!source_parsed.ok()) {
        return failure(
            RelativeSlotRebuildStatus::source_parse_failed,
            source_parsed.message.empty()
                ? "Canonical source PAC/PNST parse failed."
                : source_parsed.message);
    }

    const auto output_span = std::span<const std::byte>{
        output_bytes.data(), output_bytes.size()};
    const auto output_parsed = parse_as(*kind, output_span);
    if (!output_parsed.ok()) {
        return failure(
            RelativeSlotRebuildStatus::output_parse_failed,
            output_parsed.message.empty()
                ? "Canonical rebuilt PAC/PNST parse failed."
                : output_parsed.message);
    }

    const auto source_topology = topology_of(*source_parsed.document);
    const auto output_topology = topology_of(*output_parsed.document);
    if (!source_topology.valid() || !output_topology.valid()) {
        return failure(
            RelativeSlotRebuildStatus::topology_changed,
            "Canonical parser produced an invalid topology receipt.");
    }
    if (source_topology != output_topology) {
        return failure(
            RelativeSlotRebuildStatus::topology_changed,
            "Layout-preserving mode forbids changes to slot count, occupancy, offsets, bounded spans or total container size.");
    }

    if (source_topology.protected_prefix_size >
        static_cast<std::uint64_t>(source.bytes.size())) {
        return failure(
            RelativeSlotRebuildStatus::topology_changed,
            "Protected container prefix lies outside the supplied byte image.");
    }
    const auto prefix_size = static_cast<std::size_t>(
        source_topology.protected_prefix_size);
    if (!std::equal(
            source.bytes.begin(),
            source.bytes.begin() + static_cast<std::ptrdiff_t>(prefix_size),
            output_bytes.begin())) {
        return failure(
            RelativeSlotRebuildStatus::protected_prefix_changed,
            "Layout-preserving mode forbids edits to the container header/table and pre-payload byte region.");
    }

    auto emitted = output_bytes;
    const auto output_sha = sha256_of(std::span<const std::byte>{
        emitted.data(), emitted.size()});
    LayoutPreservingRebuildReceipt receipt{
        .resource = source.resource.id,
        .source_sha256 = source_sha,
        .output_sha256 = output_sha,
        .working_revision = working_copy.revision(),
        .source_topology = source_topology,
        .output_topology = output_topology,
    };
    if (!receipt.valid()) {
        return failure(
            RelativeSlotRebuildStatus::topology_changed,
            "Rebuild receipt failed internal validation.");
    }

    return RelativeSlotRebuildResult{
        .status = RelativeSlotRebuildStatus::ok,
        .bytes = std::move(emitted),
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
