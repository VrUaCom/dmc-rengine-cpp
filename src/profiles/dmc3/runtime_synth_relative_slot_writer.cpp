#include "dmc_rengine/profiles/dmc3/runtime_synth_relative_slot_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void write_u32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] RuntimeSynthResult failure(
    RuntimeSynthStatus status,
    std::string detail) {
    return RuntimeSynthResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
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

[[nodiscard]] formats::RelativeSlotParseResult parse_for_format(
    std::string_view format,
    std::span<const std::byte> bytes) {
    if (format == "PAC") {
        return formats::PacParser::parse(bytes);
    }
    return formats::PnstParser::parse(bytes);
}

[[nodiscard]] bool supported_provider(
    ExactChildAuthorityKind kind) noexcept {
    return kind == ExactChildAuthorityKind::loose_resource ||
        kind == ExactChildAuthorityKind::external_exact_resource;
}

[[nodiscard]] bool proven_intrinsic_extent(
    ExactChildAuthorityKind authority_kind,
    ExactChildExtentKind extent_kind,
    std::string_view writer_mode) noexcept {
    if (!supported_provider(authority_kind)) {
        return false;
    }
    return extent_kind == ExactChildExtentKind::intrinsic_resource &&
        writer_mode.empty();
}

} // namespace

bool RuntimeSynthChildReceipt::valid() const noexcept {
    return supported_provider(authority_kind) &&
        proven_intrinsic_extent(authority_kind, extent_kind, writer_mode) &&
        !authority_id.empty() && input_sha256.size() == 64U &&
        intrinsic_size != 0U && emitted_offset != 0U;
}

bool RuntimeSynthReceipt::valid() const noexcept {
    if (!source_resource.valid() || source_sha256.size() != 64U ||
        output_sha256.size() != 64U ||
        writer_mode != "runtime-synth-relative-slot" ||
        !source_topology.valid() || !output_topology.valid() ||
        source_resource.size != source_topology.container_size ||
        source_topology.format != output_topology.format ||
        source_topology.declared_slot_count !=
            output_topology.declared_slot_count ||
        source_topology.entries.size() != output_topology.entries.size()) {
        return false;
    }

    std::size_t expected_children = 0U;
    for (std::size_t index = 0U; index < source_topology.entries.size(); ++index) {
        const auto& source_entry = source_topology.entries[index];
        const auto& output_entry = output_topology.entries[index];
        if (source_entry.slot_index != output_entry.slot_index ||
            source_entry.populated != output_entry.populated) {
            return false;
        }
        if (source_entry.populated) {
            ++expected_children;
        } else if (output_entry.offset != 0U || output_entry.size != 0U) {
            return false;
        }
    }
    if (children.size() != expected_children) {
        return false;
    }

    bool have_previous_slot = false;
    std::uint32_t previous_slot = 0U;
    for (const auto& child : children) {
        if (!child.valid() ||
            child.slot_index >= output_topology.entries.size() ||
            (have_previous_slot && child.slot_index <= previous_slot)) {
            return false;
        }
        const auto& entry = output_topology.entries[child.slot_index];
        if (!entry.populated || entry.offset != child.emitted_offset ||
            child.intrinsic_size > entry.size) {
            return false;
        }
        have_previous_slot = true;
        previous_slot = child.slot_index;
    }
    return true;
}

bool RuntimeSynthResult::ok() const noexcept {
    if (status != RuntimeSynthStatus::ok || !receipt.has_value() ||
        !receipt->valid() ||
        receipt->output_topology.container_size != bytes.size()) {
        return false;
    }
    return receipt->output_sha256 == sha256_of(
        std::span<const std::byte>{bytes.data(), bytes.size()});
}

RuntimeSynthResult RuntimeSynthRelativeSlotWriter::rebuild(
    const gdspaces::ResourcePayload& source,
    std::span<const ExactChildImage> exact_children,
    RuntimeSynthSafetyLimits safety) {
    if (!source.readable() || safety.max_output_bytes == 0U ||
        source.resource.id.size != static_cast<std::uint64_t>(source.bytes.size())) {
        return failure(
            RuntimeSynthStatus::invalid_source,
            "Runtime-synth rebuild requires a readable exact source payload and a non-zero product output budget.");
    }

    gdspaces::WorkingCopy clean_source{source};
    const auto source_receipt = RelativeSlotLayoutWriter::rebuild(
        source, clean_source);
    if (!source_receipt.ok()) {
        if (source_receipt.status == RelativeSlotRebuildStatus::unsupported_format) {
            return failure(
                RuntimeSynthStatus::unsupported_format,
                "Runtime-synth writer supports only canonical PAC/PNST source images.");
        }
        return failure(
            RuntimeSynthStatus::source_parse_failed,
            "Canonical PAC/PNST source validation failed before runtime-synth rebuild: " +
                source_receipt.detail);
    }

    const auto source_topology = source_receipt.receipt->source_topology;

    std::set<std::uint64_t> populated_offsets;
    for (const auto& entry : source_topology.entries) {
        if (!entry.populated) {
            continue;
        }
        if (!populated_offsets.insert(entry.offset).second) {
            return failure(
                RuntimeSynthStatus::alias_topology_unsupported,
                "Runtime-synth .lst authority does not evidence duplicate non-zero alias offsets; use layout-preserving mode for this source topology.");
        }
    }

    std::vector<const ExactChildImage*> child_by_slot(
        source_topology.entries.size(), nullptr);
    for (const auto& child : exact_children) {
        if (!child.valid_envelope()) {
            return failure(
                RuntimeSynthStatus::invalid_child_authority,
                "An exact child image has an invalid authority/hash/byte envelope.");
        }
        if (child.slot_index >= source_topology.entries.size()) {
            return failure(
                RuntimeSynthStatus::unknown_child_slot,
                "An exact child image targets a slot outside the declared source topology.");
        }
        const auto& source_entry = source_topology.entries[child.slot_index];
        if (!source_entry.populated) {
            return failure(
                RuntimeSynthStatus::child_for_empty_slot,
                "The first runtime-synth tier preserves empty/populated occupancy and rejects child bytes for an empty source slot.");
        }
        if (child_by_slot[child.slot_index] != nullptr) {
            return failure(
                RuntimeSynthStatus::duplicate_child_input,
                "More than one exact child image was supplied for one populated physical slot.");
        }
        if (child.authority_kind ==
            ExactChildAuthorityKind::container_extracted_span) {
            return failure(
                RuntimeSynthStatus::forbidden_extracted_span,
                "Packed ContainerEntry extents are bounded extraction spans, not intrinsic child-file length authority.");
        }
        if (child.authority_kind ==
            ExactChildAuthorityKind::format_writer_receipt) {
            return failure(
                RuntimeSynthStatus::unproven_intrinsic_extent,
                "Generic writer-receipt claims are not accepted as intrinsic extent proof. A future typed verified-result factory must bind complete-image writer output before it can feed a size-changing parent.");
        }
        if (!supported_provider(child.authority_kind)) {
            return failure(
                RuntimeSynthStatus::invalid_child_authority,
                "Exact child byte-provider authority kind is not accepted by this runtime-synth writer tier.");
        }
        if (!proven_intrinsic_extent(
                child.authority_kind,
                child.extent_kind,
                child.writer_mode)) {
            return failure(
                RuntimeSynthStatus::unproven_intrinsic_extent,
                "Exact child bytes do not carry independent intrinsic-resource extent authority. Source-span-preserved or container-inferred extents cannot feed a size-changing parent.");
        }
        const auto actual_sha = sha256_of(std::span<const std::byte>{
            child.bytes.data(), child.bytes.size()});
        if (actual_sha != child.sha256) {
            return failure(
                RuntimeSynthStatus::child_hash_mismatch,
                "Exact child SHA-256 does not bind to the supplied intrinsic child bytes.");
        }
        child_by_slot[child.slot_index] = &child;
    }

    for (const auto& entry : source_topology.entries) {
        if (entry.populated && child_by_slot[entry.slot_index] == nullptr) {
            return failure(
                RuntimeSynthStatus::missing_child,
                "Every populated source slot requires explicit exact intrinsic child bytes, including unchanged slots.");
        }
    }

    const auto header_size = LooseContainerListPolicy::header_size(
        source_topology.declared_slot_count);
    if (!header_size.has_value() || *header_size > safety.max_output_bytes ||
        *header_size > static_cast<std::size_t>(
            std::numeric_limits<std::uint32_t>::max())) {
        return failure(
            RuntimeSynthStatus::output_too_large,
            "Recovered runtime-synth header cannot fit the configured output/32-bit offset domain.");
    }

    std::size_t total_size = *header_size;
    for (const auto& entry : source_topology.entries) {
        if (!entry.populated) {
            continue;
        }
        const auto* child = child_by_slot[entry.slot_index];
        const auto aligned = LooseContainerListPolicy::aligned_size(
            child->bytes.size());
        if (aligned == std::numeric_limits<std::size_t>::max() ||
            aligned > safety.max_output_bytes ||
            total_size > safety.max_output_bytes - aligned ||
            aligned > static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max()) ||
            total_size > static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max()) - aligned) {
            return failure(
                RuntimeSynthStatus::output_too_large,
                "Runtime-synth child placement exceeds the product output or 32-bit relative-offset envelope.");
        }
        total_size += aligned;
    }

    std::vector<std::byte> output(total_size, std::byte{0});
    std::copy_n(source.bytes.begin(), 4U, output.begin());
    write_u32_le(output, 4U, source_topology.declared_slot_count);

    std::size_t cursor = *header_size;
    std::vector<RuntimeSynthChildReceipt> child_receipts;
    child_receipts.reserve(exact_children.size());
    for (const auto& entry : source_topology.entries) {
        const auto table_offset = 8U +
            static_cast<std::size_t>(entry.slot_index) * 4U;
        if (!entry.populated) {
            write_u32_le(output, table_offset, 0U);
            continue;
        }

        if (cursor >= static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max())) {
            return failure(
                RuntimeSynthStatus::output_too_large,
                "Runtime-synth emitted child offset reaches the unsupported 32-bit sentinel boundary.");
        }
        const auto* child = child_by_slot[entry.slot_index];
        const auto emitted_offset = static_cast<std::uint32_t>(cursor);
        write_u32_le(output, table_offset, emitted_offset);
        std::copy(
            child->bytes.begin(), child->bytes.end(),
            output.begin() + static_cast<std::ptrdiff_t>(cursor));
        child_receipts.push_back(RuntimeSynthChildReceipt{
            .slot_index = entry.slot_index,
            .authority_kind = child->authority_kind,
            .extent_kind = child->extent_kind,
            .authority_id = child->authority_id,
            .writer_mode = child->writer_mode,
            .input_sha256 = child->sha256,
            .intrinsic_size = static_cast<std::uint64_t>(child->bytes.size()),
            .emitted_offset = emitted_offset,
        });
        cursor += LooseContainerListPolicy::aligned_size(child->bytes.size());
    }

    const auto output_span = std::span<const std::byte>{
        output.data(), output.size()};
    const auto output_parsed = parse_for_format(
        source_topology.format, output_span);
    if (!output_parsed.ok()) {
        return failure(
            RuntimeSynthStatus::output_parse_failed,
            "Generated runtime-synth image failed canonical PAC/PNST reparse: " +
                output_parsed.message);
    }
    const auto output_topology = topology_of(*output_parsed.document);
    if (!output_topology.valid() ||
        output_topology.format != source_topology.format ||
        output_topology.declared_slot_count !=
            source_topology.declared_slot_count ||
        output_topology.entries.size() != source_topology.entries.size()) {
        return failure(
            RuntimeSynthStatus::topology_changed,
            "Generated runtime-synth image changed the source format or declared slot topology.");
    }

    std::size_t receipt_index = 0U;
    for (std::size_t index = 0U; index < source_topology.entries.size(); ++index) {
        const auto& source_entry = source_topology.entries[index];
        const auto& output_entry = output_topology.entries[index];
        if (source_entry.populated != output_entry.populated) {
            return failure(
                RuntimeSynthStatus::topology_changed,
                "Generated runtime-synth image changed source slot occupancy.");
        }
        if (!source_entry.populated) {
            if (output_entry.offset != 0U || output_entry.size != 0U) {
                return failure(
                    RuntimeSynthStatus::topology_changed,
                    "Generated empty source slot did not remain a zero-offset empty slot.");
            }
            continue;
        }
        if (receipt_index >= child_receipts.size() ||
            child_receipts[receipt_index].slot_index != index ||
            output_entry.offset != child_receipts[receipt_index].emitted_offset ||
            child_receipts[receipt_index].intrinsic_size > output_entry.size) {
            return failure(
                RuntimeSynthStatus::topology_changed,
                "Canonical reparse does not match deterministic emitted child placement/extent.");
        }
        ++receipt_index;
    }

    RuntimeSynthReceipt receipt{
        .source_resource = source.resource.id,
        .source_sha256 = source_receipt.receipt->source_sha256,
        .output_sha256 = sha256_of(output_span),
        .writer_mode = "runtime-synth-relative-slot",
        .source_topology = source_topology,
        .output_topology = output_topology,
        .children = std::move(child_receipts),
    };
    if (!receipt.valid()) {
        return failure(
            RuntimeSynthStatus::invalid_receipt,
            "Runtime-synth authoring receipt failed internal validation.");
    }

    return RuntimeSynthResult{
        .status = RuntimeSynthStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
