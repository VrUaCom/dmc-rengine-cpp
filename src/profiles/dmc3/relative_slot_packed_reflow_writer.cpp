#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::string_view kWriterMode = "packed-relative-slot-reflow";
constexpr std::size_t kHeaderFixedSize = 8U;
constexpr std::size_t kOffsetWidth = 4U;
constexpr std::size_t kNoSpan = std::numeric_limits<std::size_t>::max();

struct PhysicalSpan final {
    std::uint64_t source_offset{};
    std::uint64_t source_size{};
    std::uint64_t output_offset{};
    std::vector<const gdspaces::ContainerChild*> aliases;
    std::vector<const AuthoredChildImage*> authored;
    std::vector<std::byte> replacement;
    bool changed{};
};

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] RelativeSlotPackedReflowResult failure(
    RelativeSlotPackedReflowStatus status,
    std::string detail) {
    return RelativeSlotPackedReflowResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] formats::RelativeSlotParseResult parse_for_format(
    std::string_view format,
    std::span<const std::byte> bytes) {
    if (format == "PAC") {
        return formats::PacParser::parse(bytes);
    }
    if (format == "PNST") {
        return formats::PnstParser::parse(bytes);
    }
    return formats::RelativeSlotParseResult{
        .document = std::nullopt,
        .error = formats::RelativeSlotParseError::invalid_magic,
        .message = "Packed reflow supports only PAC/PNST relative-slot images.",
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
                topology.protected_prefix_size, entry.offset);
        }
    }
    return topology;
}

[[nodiscard]] bool same_alias_partition(
    const RelativeSlotTopology& source,
    const RelativeSlotTopology& output) {
    if (source.entries.size() != output.entries.size()) {
        return false;
    }

    std::unordered_map<std::uint64_t, std::uint64_t> source_to_output;
    std::unordered_map<std::uint64_t, std::uint64_t> output_to_source;
    source_to_output.reserve(source.entries.size());
    output_to_source.reserve(output.entries.size());

    for (std::size_t index = 0U; index < source.entries.size(); ++index) {
        const auto& source_entry = source.entries[index];
        const auto& output_entry = output.entries[index];
        if (source_entry.populated != output_entry.populated) {
            return false;
        }
        if (!source_entry.populated) {
            continue;
        }

        const auto [forward, forward_inserted] = source_to_output.emplace(
            source_entry.offset, output_entry.offset);
        if (!forward_inserted && forward->second != output_entry.offset) {
            return false;
        }
        const auto [reverse, reverse_inserted] = output_to_source.emplace(
            output_entry.offset, source_entry.offset);
        if (!reverse_inserted && reverse->second != source_entry.offset) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool same_relative_slot_relation(
    const RelativeSlotTopology& source,
    const RelativeSlotTopology& output) {
    return source.valid() && output.valid() &&
        source.format == output.format &&
        source.declared_slot_count == output.declared_slot_count &&
        source.protected_prefix_size == output.protected_prefix_size &&
        same_alias_partition(source, output);
}

[[nodiscard]] bool exact_parent_span(
    const gdspaces::ResourcePayload& parent,
    const gdspaces::ContainerChild& child) noexcept {
    const auto parent_size = static_cast<std::uint64_t>(parent.bytes.size());
    if (!child.entry.populated || child.entry.offset > parent_size ||
        child.entry.size > parent_size - child.entry.offset ||
        child.payload.bytes.size() != child.entry.size ||
        child.payload.resource.id.size != child.entry.size) {
        return false;
    }

    const auto offset = static_cast<std::size_t>(child.entry.offset);
    const auto size = static_cast<std::size_t>(child.entry.size);
    return std::equal(
        child.payload.bytes.begin(), child.payload.bytes.end(),
        parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
        parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset + size));
}

[[nodiscard]] bool same_texture_pack_relation(
    const formats::ContainerDocument& source,
    const formats::ContainerDocument& output) {
    // A texture pack has no slot table to preserve; what must survive an edit
    // is the set of textures and their identities. Sizes and offsets are
    // expected to move — that is what size-changing authoring means — so
    // pinning them here would forbid the very edit the writer exists for.
    if (source.entries.size() != output.entries.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < source.entries.size(); ++index) {
        if (source.entries[index].slot_index !=
            output.entries[index].slot_index) {
            return false;
        }
        if (!output.entries[index].valid(output.container_size)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool validate_changed_container_child(
    const gdspaces::ContainerChild& child,
    const AuthoredChildImage& authored) {
    if (!child.payload.resource.container) {
        return true;
    }

    const auto source_bytes = std::span<const std::byte>{
        child.payload.bytes.data(), child.payload.bytes.size()};
    const auto output_bytes = std::span<const std::byte>{
        authored.bytes.data(), authored.bytes.size()};

    const auto opens_with = [](std::span<const std::byte> bytes,
                               std::string_view magic) {
        if (bytes.size() < magic.size()) {
            return false;
        }
        for (std::size_t index = 0U; index < magic.size(); ++index) {
            if (std::to_integer<unsigned char>(bytes[index]) !=
                static_cast<unsigned char>(magic[index])) {
                return false;
            }
        }
        return true;
    };

    std::string_view source_format;
    if (opens_with(source_bytes, RelativeSlotWalkContract::pac_magic)) {
        source_format = "PAC";
    } else if (opens_with(source_bytes, RelativeSlotWalkContract::pnst_magic)) {
        source_format = "PNST";
    } else {
        // Not a relative-slot container. The only other container the tree
        // expands is the texture pack, and it must be validated by its own
        // parser rather than pushed through a slot-topology check it was never
        // going to satisfy.
        const auto source_pack = formats::PtxParser::parse(source_bytes);
        const auto output_pack = formats::PtxParser::parse(output_bytes);
        if (!source_pack.ok() || !output_pack.ok()) {
            return false;
        }
        return same_texture_pack_relation(*source_pack.document, *output_pack.document);
    }

    const auto source_parsed = parse_for_format(source_format, source_bytes);
    const auto output_parsed = parse_for_format(source_format, output_bytes);
    if (!source_parsed.ok() || !output_parsed.ok()) {
        return false;
    }
    return same_relative_slot_relation(
        topology_of(*source_parsed.document),
        topology_of(*output_parsed.document));
}

void write_u32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] bool fits_u32(std::uint64_t value) noexcept {
    return value <= std::numeric_limits<std::uint32_t>::max();
}

[[nodiscard]] bool checked_add(
    std::uint64_t left,
    std::uint64_t right,
    std::uint64_t& output) noexcept {
    if (left > std::numeric_limits<std::uint64_t>::max() - right) {
        return false;
    }
    output = left + right;
    return true;
}

} // namespace

bool RelativeSlotPackedSpanReceipt::valid() const noexcept {
    if (source_size == 0U || output_size == 0U ||
        source_sha256.size() != 64U || output_sha256.size() != 64U ||
        affected_aliases.empty()) {
        return false;
    }
    const bool actually_changed =
        source_size != output_size || source_sha256 != output_sha256;
    if (changed != actually_changed) {
        return false;
    }
    for (const auto& alias : affected_aliases) {
        if (!alias.valid() || alias.size != source_size) {
            return false;
        }
    }
    for (const auto& alias : authored_aliases) {
        if (!alias.valid() || alias.size != source_size) {
            return false;
        }
    }
    return !changed || !authored_aliases.empty();
}

bool RelativeSlotPackedReflowReceipt::valid() const {
    if (!parent.valid() || source_sha256.size() != 64U ||
        output_sha256.size() != 64U || writer_mode != kWriterMode ||
        !source_topology.valid() || !output_topology.valid() ||
        parent.size != source_topology.container_size || spans.empty() ||
        !same_relative_slot_relation(source_topology, output_topology)) {
        return false;
    }

    std::unordered_set<std::uint64_t> distinct_source_offsets;
    distinct_source_offsets.reserve(source_topology.entries.size());
    for (const auto& entry : source_topology.entries) {
        if (entry.populated) {
            distinct_source_offsets.insert(entry.offset);
        }
    }
    if (spans.size() != distinct_source_offsets.size()) {
        return false;
    }

    std::uint64_t previous_source_end = source_topology.protected_prefix_size;
    std::uint64_t previous_output_end = output_topology.protected_prefix_size;
    for (const auto& span : spans) {
        if (!span.valid() || span.source_offset != previous_source_end ||
            span.output_offset != previous_output_end) {
            return false;
        }
        if (!checked_add(
                span.source_offset, span.source_size, previous_source_end) ||
            !checked_add(
                span.output_offset, span.output_size, previous_output_end)) {
            return false;
        }
    }
    return previous_source_end == source_topology.container_size &&
        previous_output_end == output_topology.container_size;
}

bool RelativeSlotPackedReflowResult::ok() const {
    if (status != RelativeSlotPackedReflowStatus::ok ||
        !receipt.has_value() || !receipt->valid() ||
        receipt->output_topology.container_size != bytes.size()) {
        return false;
    }
    return receipt->output_sha256 == sha256_of(
        std::span<const std::byte>{bytes.data(), bytes.size()});
}

RelativeSlotPackedReflowResult RelativeSlotPackedReflowWriter::rebuild(
    const gdspaces::ResourcePayload& parent,
    const gdspaces::ContainerExpansion& expansion,
    std::span<const AuthoredChildImage> authored_children,
    RelativeSlotPackedReflowSafety safety) {
    if (!parent.readable() || safety.max_output_bytes == 0U ||
        parent.resource.id.size != static_cast<std::uint64_t>(parent.bytes.size())) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_parent,
            "Packed relative-slot reflow requires a readable exact parent and a non-zero output budget.");
    }
    if (!expansion.usable() || expansion.parent != parent.resource ||
        (expansion.parser_format != "PAC" && expansion.parser_format != "PNST")) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_expansion,
            "ContainerExpansion is not a usable exact PAC/PNST expansion of the supplied parent.");
    }
    if (authored_children.empty()) {
        return failure(
            RelativeSlotPackedReflowStatus::no_authored_children,
            "Packed relative-slot reflow requires at least one authored child image.");
    }

    gdspaces::WorkingCopy clean_parent{parent};
    const auto canonical_source = RelativeSlotLayoutWriter::rebuild(
        parent, clean_parent);
    if (!canonical_source.ok()) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_parent,
            "Canonical PAC/PNST source validation failed before packed reflow: " +
                canonical_source.detail);
    }
    const auto source_topology = canonical_source.receipt->source_topology;
    if (source_topology.format != expansion.parser_format ||
        expansion.children.size() != source_topology.entries.size()) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_expansion,
            "Expansion topology does not match the canonical source slot topology.");
    }

    std::unordered_map<std::string, std::size_t> child_index_by_id;
    child_index_by_id.reserve(expansion.children.size());
    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        const auto& child = expansion.children[index];
        const auto& topology = source_topology.entries[index];
        if (child.entry.slot_index != topology.slot_index ||
            child.entry.populated != topology.populated ||
            child.entry.offset != topology.offset ||
            child.entry.size != topology.size) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "Expansion child geometry differs from canonical parser geometry.");
        }
        if (!child.entry.populated) {
            continue;
        }
        if (!exact_parent_span(parent, child)) {
            return failure(
                RelativeSlotPackedReflowStatus::parent_span_mismatch,
                "A populated expansion child no longer equals its exact bounded parent span.");
        }
        if (parent.resource.id.offset >
                std::numeric_limits<std::uint64_t>::max() - child.entry.offset ||
            child.payload.resource.id.offset !=
                parent.resource.id.offset + child.entry.offset ||
            child.payload.resource.id.source_id != parent.resource.id.source_id) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "A populated child identity is not bound to the supplied parent coordinates.");
        }
        const auto [_, inserted] = child_index_by_id.emplace(
            child.payload.resource.id.canonical(), index);
        if (!inserted) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "Two populated expansion children expose the same canonical ResourceId.");
        }
    }

    std::vector<std::size_t> physical_order;
    physical_order.reserve(child_index_by_id.size());
    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        if (expansion.children[index].entry.populated) {
            physical_order.push_back(index);
        }
    }
    std::sort(
        physical_order.begin(), physical_order.end(),
        [&expansion](std::size_t left, std::size_t right) {
            const auto& lhs = expansion.children[left].entry;
            const auto& rhs = expansion.children[right].entry;
            if (lhs.offset != rhs.offset) {
                return lhs.offset < rhs.offset;
            }
            return lhs.slot_index < rhs.slot_index;
        });

    std::vector<PhysicalSpan> spans;
    std::vector<std::size_t> span_by_slot(
        expansion.children.size(), kNoSpan);
    for (const auto child_index : physical_order) {
        const auto& child = expansion.children[child_index];
        if (spans.empty() || spans.back().source_offset != child.entry.offset) {
            spans.push_back(PhysicalSpan{
                .source_offset = child.entry.offset,
                .source_size = child.entry.size,
                .output_offset = 0U,
                .aliases = {},
                .authored = {},
                .replacement = {},
                .changed = false,
            });
        } else if (spans.back().source_size != child.entry.size) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "Aliased child identities disagree on their physical source span size.");
        }
        spans.back().aliases.push_back(&child);
        span_by_slot[child.entry.slot_index] = spans.size() - 1U;
    }
    if (spans.empty() ||
        spans.front().source_offset != source_topology.protected_prefix_size) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_expansion,
            "Canonical populated spans do not start at the protected payload boundary.");
    }
    std::uint64_t expected_source_offset = source_topology.protected_prefix_size;
    for (const auto& span : spans) {
        if (span.source_offset != expected_source_offset ||
            !checked_add(
                span.source_offset, span.source_size, expected_source_offset)) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "Canonical physical spans are not contiguous through the parent payload domain.");
        }
    }
    if (expected_source_offset != source_topology.container_size) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_expansion,
            "Canonical physical spans do not cover the complete packed parent payload.");
    }

    std::unordered_set<std::string> seen_authored;
    seen_authored.reserve(authored_children.size());
    bool have_change = false;
    for (const auto& authored : authored_children) {
        if (!authored.valid()) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_authored_image,
                "An authored child image has an invalid identity/hash/writer envelope.");
        }
        const auto canonical_id = authored.resource.canonical();
        if (!seen_authored.insert(canonical_id).second) {
            return failure(
                RelativeSlotPackedReflowStatus::duplicate_child_input,
                "The same child ResourceId appears more than once in the authored input set.");
        }
        const auto child_lookup = child_index_by_id.find(canonical_id);
        if (child_lookup == child_index_by_id.end()) {
            return failure(
                RelativeSlotPackedReflowStatus::child_not_found,
                "An authored child ResourceId is not a populated child of the exact expansion.");
        }
        const auto child_index = child_lookup->second;
        const auto& child = expansion.children[child_index];
        const auto source_sha = sha256_of(std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()});
        if (source_sha != authored.source_sha256) {
            return failure(
                RelativeSlotPackedReflowStatus::child_source_mismatch,
                "Authored child source SHA-256 does not bind to the expanded source span.");
        }
        const auto output_sha = sha256_of(std::span<const std::byte>{
            authored.bytes.data(), authored.bytes.size()});
        if (output_sha != authored.output_sha256) {
            return failure(
                RelativeSlotPackedReflowStatus::child_output_mismatch,
                "Authored child output SHA-256 does not bind to the supplied authored bytes.");
        }

        const bool changed = authored.bytes != child.payload.bytes;
        if (!changed) {
            continue;
        }
        have_change = true;
        if (!validate_changed_container_child(child, authored)) {
            return failure(
                RelativeSlotPackedReflowStatus::child_writer_validation_failed,
                "A changed nested container image failed independent canonical validation for its own format.");
        }
        const auto span_index = span_by_slot[child.entry.slot_index];
        if (span_index == kNoSpan || span_index >= spans.size()) {
            return failure(
                RelativeSlotPackedReflowStatus::invalid_expansion,
                "A changed authored child has no physical source-span group.");
        }
        auto& span = spans[span_index];
        if (span.replacement.empty()) {
            span.replacement = authored.bytes;
        } else if (span.replacement != authored.bytes) {
            return failure(
                RelativeSlotPackedReflowStatus::alias_conflict,
                "Changed aliases request divergent byte images for one physical parent span.");
        }
        span.authored.push_back(&authored);
        span.changed = true;
    }
    if (!have_change) {
        return failure(
            RelativeSlotPackedReflowStatus::no_changes,
            "All authored child images are byte-identical to their source spans.");
    }

    const auto prefix_size = source_topology.protected_prefix_size;
    if (prefix_size > parent.bytes.size() ||
        prefix_size > safety.max_output_bytes || !fits_u32(prefix_size)) {
        return failure(
            RelativeSlotPackedReflowStatus::output_too_large,
            "Protected prefix exceeds the configured output or 32-bit offset domain.");
    }

    std::uint64_t output_size = prefix_size;
    for (auto& span : spans) {
        span.output_offset = output_size;
        const auto emitted_size = span.changed
            ? static_cast<std::uint64_t>(span.replacement.size())
            : span.source_size;
        if (emitted_size == 0U ||
            !checked_add(output_size, emitted_size, output_size) ||
            output_size > safety.max_output_bytes || !fits_u32(output_size)) {
            return failure(
                RelativeSlotPackedReflowStatus::output_too_large,
                "Packed child reflow exceeds the configured output or 32-bit offset domain.");
        }
    }
    if (output_size > std::numeric_limits<std::size_t>::max()) {
        return failure(
            RelativeSlotPackedReflowStatus::output_too_large,
            "Packed child reflow exceeds the host addressable vector domain.");
    }

    std::vector<std::byte> output;
    output.reserve(static_cast<std::size_t>(output_size));
    output.insert(
        output.end(), parent.bytes.begin(),
        parent.bytes.begin() + static_cast<std::ptrdiff_t>(prefix_size));
    for (const auto& span : spans) {
        if (span.changed) {
            output.insert(
                output.end(), span.replacement.begin(), span.replacement.end());
        } else {
            const auto start = static_cast<std::size_t>(span.source_offset);
            const auto end = start + static_cast<std::size_t>(span.source_size);
            output.insert(
                output.end(),
                parent.bytes.begin() + static_cast<std::ptrdiff_t>(start),
                parent.bytes.begin() + static_cast<std::ptrdiff_t>(end));
        }
    }
    if (output.size() != static_cast<std::size_t>(output_size)) {
        return failure(
            RelativeSlotPackedReflowStatus::output_too_large,
            "Packed reflow emission did not produce the planned byte count.");
    }

    if (source_topology.declared_slot_count >
        (std::numeric_limits<std::size_t>::max() - kHeaderFixedSize) /
            kOffsetWidth) {
        return failure(
            RelativeSlotPackedReflowStatus::output_too_large,
            "Relative-slot offset table arithmetic exceeds the host size domain.");
    }
    const auto table_end = kHeaderFixedSize +
        static_cast<std::size_t>(source_topology.declared_slot_count) *
            kOffsetWidth;
    if (table_end > prefix_size) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_parent,
            "Canonical protected prefix does not contain the complete offset table.");
    }

    for (const auto& entry : source_topology.entries) {
        const auto table_offset = kHeaderFixedSize +
            static_cast<std::size_t>(entry.slot_index) * kOffsetWidth;
        if (!entry.populated) {
            write_u32_le(output, table_offset, 0U);
            continue;
        }
        const auto span_index = span_by_slot[entry.slot_index];
        if (span_index == kNoSpan || span_index >= spans.size() ||
            !fits_u32(spans[span_index].output_offset)) {
            return failure(
                RelativeSlotPackedReflowStatus::topology_changed,
                "A populated source slot has no mapped output physical span.");
        }
        write_u32_le(
            output, table_offset,
            static_cast<std::uint32_t>(spans[span_index].output_offset));
    }

    const auto output_parsed = parse_for_format(
        source_topology.format,
        std::span<const std::byte>{output.data(), output.size()});
    if (!output_parsed.ok()) {
        return failure(
            RelativeSlotPackedReflowStatus::output_parse_failed,
            output_parsed.message.empty()
                ? "Canonical PAC/PNST parser rejected the packed reflow output."
                : output_parsed.message);
    }
    const auto output_topology = topology_of(*output_parsed.document);
    if (!same_relative_slot_relation(source_topology, output_topology)) {
        return failure(
            RelativeSlotPackedReflowStatus::topology_changed,
            "Packed reflow changed slot count, occupancy, alias partition or protected-prefix relation.");
    }

    for (std::size_t index = 0U; index < source_topology.entries.size(); ++index) {
        const auto& source_entry = source_topology.entries[index];
        const auto& output_entry = output_topology.entries[index];
        if (!source_entry.populated) {
            if (output_entry.offset != 0U || output_entry.size != 0U) {
                return failure(
                    RelativeSlotPackedReflowStatus::topology_changed,
                    "An empty source slot became populated after packed reflow.");
            }
            continue;
        }
        const auto span_index = span_by_slot[source_entry.slot_index];
        if (span_index == kNoSpan || span_index >= spans.size()) {
            return failure(
                RelativeSlotPackedReflowStatus::topology_changed,
                "A populated source slot lost its physical-span mapping.");
        }
        const auto& span = spans[span_index];
        const auto expected_size = span.changed
            ? static_cast<std::uint64_t>(span.replacement.size())
            : span.source_size;
        if (output_entry.offset != span.output_offset ||
            output_entry.size != expected_size) {
            return failure(
                RelativeSlotPackedReflowStatus::topology_changed,
                "Canonical output slot geometry differs from the planned physical-span reflow.");
        }
    }

    std::vector<RelativeSlotPackedSpanReceipt> span_receipts;
    span_receipts.reserve(spans.size());
    for (const auto& span : spans) {
        const auto source_begin = static_cast<std::size_t>(span.source_offset);
        const auto output_begin = static_cast<std::size_t>(span.output_offset);
        const auto emitted_size = span.changed
            ? span.replacement.size()
            : static_cast<std::size_t>(span.source_size);

        RelativeSlotPackedSpanReceipt span_receipt{
            .source_offset = span.source_offset,
            .source_size = span.source_size,
            .output_offset = span.output_offset,
            .output_size = static_cast<std::uint64_t>(emitted_size),
            .changed = span.changed,
            .source_sha256 = sha256_of(std::span<const std::byte>{
                parent.bytes.data() + source_begin,
                static_cast<std::size_t>(span.source_size)}),
            .output_sha256 = sha256_of(std::span<const std::byte>{
                output.data() + output_begin, emitted_size}),
            .affected_aliases = {},
            .authored_aliases = {},
        };
        span_receipt.affected_aliases.reserve(span.aliases.size());
        for (const auto* alias : span.aliases) {
            span_receipt.affected_aliases.push_back(alias->payload.resource.id);
        }
        span_receipt.authored_aliases.reserve(span.authored.size());
        for (const auto* authored : span.authored) {
            span_receipt.authored_aliases.push_back(authored->resource);
        }
        span_receipts.push_back(std::move(span_receipt));
    }

    RelativeSlotPackedReflowReceipt receipt{
        .parent = parent.resource.id,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            parent.bytes.data(), parent.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            output.data(), output.size()}),
        .writer_mode = std::string{kWriterMode},
        .source_topology = source_topology,
        .output_topology = output_topology,
        .spans = std::move(span_receipts),
    };
    if (!receipt.valid()) {
        return failure(
            RelativeSlotPackedReflowStatus::invalid_receipt,
            "Packed relative-slot reflow receipt failed internal validation.");
    }

    return RelativeSlotPackedReflowResult{
        .status = RelativeSlotPackedReflowStatus::ok,
        .bytes = std::move(output),
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
