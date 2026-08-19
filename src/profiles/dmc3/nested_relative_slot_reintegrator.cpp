#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

struct PreparedChild final {
    const gdspaces::ContainerChild* child{};
    const AuthoredChildImage* authored{};
    bool changed{false};
};

struct PreparedSpan final {
    std::uint64_t offset{};
    std::uint64_t size{};
    std::vector<const gdspaces::ContainerChild*> aliases;
    std::vector<const AuthoredChildImage*> authored;
    std::vector<std::byte> replacement;
};

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] NestedReintegrationResult failure(
    NestedReintegrationStatus status,
    std::string detail) {
    return NestedReintegrationResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] const gdspaces::ContainerChild* find_child(
    const gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourceId& resource) noexcept {
    const auto found = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [&resource](const auto& child) {
            return child.entry.populated && child.payload.resource.id == resource;
        });
    return found == expansion.children.end() ? nullptr : &*found;
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

[[nodiscard]] std::string span_edit_id(
    std::uint64_t offset,
    std::uint64_t size) {
    return "nested-span-" + std::to_string(offset) + "-" +
        std::to_string(size);
}

} // namespace

bool AuthoredChildImage::valid() const noexcept {
    return resource.valid() && resource.size != 0U &&
        bytes.size() == resource.size && source_sha256.size() == 64U &&
        output_sha256.size() == 64U && !writer_mode.empty();
}

bool NestedSpanReceipt::valid() const noexcept {
    if (size == 0U || source_sha256.size() != 64U ||
        output_sha256.size() != 64U || affected_aliases.empty() ||
        authored_aliases.empty()) {
        return false;
    }
    for (const auto& alias : affected_aliases) {
        if (!alias.valid() || alias.size != size) {
            return false;
        }
    }
    for (const auto& alias : authored_aliases) {
        if (!alias.valid() || alias.size != size) {
            return false;
        }
    }
    return true;
}

bool NestedReintegrationReceipt::valid() const noexcept {
    if (!parent.valid() || parent_source_sha256.size() != 64U ||
        parent_output_sha256.size() != 64U || spans.empty() ||
        !parent_rebuild.valid() || parent_rebuild.resource != parent ||
        parent_rebuild.source_sha256 != parent_source_sha256 ||
        parent_rebuild.output_sha256 != parent_output_sha256 ||
        parent_rebuild.working_revision != parent_revision) {
        return false;
    }
    for (const auto& span : spans) {
        if (!span.valid() || span.offset > parent.size ||
            span.size > parent.size - span.offset) {
            return false;
        }
    }
    return true;
}

NestedReintegrationResult NestedRelativeSlotReintegrator::reintegrate(
    const gdspaces::ResourcePayload& parent,
    const gdspaces::ContainerExpansion& expansion,
    std::span<const AuthoredChildImage> authored_children) {
    if (!parent.readable() ||
        parent.resource.id.size != static_cast<std::uint64_t>(parent.bytes.size())) {
        return failure(
            NestedReintegrationStatus::invalid_parent,
            "Nested reintegration requires a readable immutable parent whose ResourceId size matches its materialized bytes.");
    }
    if (!expansion.usable() || expansion.parent != parent.resource ||
        (expansion.parser_format != "PAC" && expansion.parser_format != "PNST")) {
        return failure(
            NestedReintegrationStatus::invalid_expansion,
            "ContainerExpansion is not a usable exact PAC/PNST expansion of the supplied parent resource.");
    }
    if (authored_children.empty()) {
        return failure(
            NestedReintegrationStatus::no_authored_children,
            "Nested reintegration requires at least one writer-validated authored child image.");
    }

    // Validate the expansion against the exact immutable parent byte image,
    // including aliases that are not themselves authored inputs.
    for (const auto& child : expansion.children) {
        if (!child.entry.populated) {
            continue;
        }
        if (!exact_parent_span(parent, child)) {
            return failure(
                NestedReintegrationStatus::parent_span_mismatch,
                "A populated expansion child no longer equals its exact bounded span in the immutable parent bytes.");
        }
        if (parent.resource.id.offset >
            std::numeric_limits<std::uint64_t>::max() - child.entry.offset ||
            child.payload.resource.id.offset !=
                parent.resource.id.offset + child.entry.offset ||
            child.payload.resource.id.source_id != parent.resource.id.source_id) {
            return failure(
                NestedReintegrationStatus::invalid_expansion,
                "A populated child identity is not bound to the supplied parent expansion coordinates.");
        }
    }

    std::set<std::string> seen_children;
    std::vector<PreparedChild> prepared;
    prepared.reserve(authored_children.size());
    for (const auto& authored : authored_children) {
        if (!authored.valid()) {
            return failure(
                NestedReintegrationStatus::invalid_authored_image,
                "An authored child image has an invalid identity, hash, writer-mode or same-size byte envelope.");
        }
        if (!seen_children.insert(authored.resource.canonical()).second) {
            return failure(
                NestedReintegrationStatus::duplicate_child_input,
                "The same child ResourceId appears more than once in the authored input set.");
        }

        const auto* child = find_child(expansion, authored.resource);
        if (child == nullptr) {
            return failure(
                NestedReintegrationStatus::child_not_found,
                "An authored child ResourceId is not a populated child of the exact supplied expansion.");
        }
        if (authored.bytes.size() != child->entry.size) {
            return failure(
                NestedReintegrationStatus::child_size_changed,
                "Layout-preserving nested reintegration requires the authored child byte count to equal its bounded source span.");
        }

        const auto source_sha = sha256_of(std::span<const std::byte>{
            child->payload.bytes.data(), child->payload.bytes.size()});
        if (source_sha != authored.source_sha256) {
            return failure(
                NestedReintegrationStatus::child_source_mismatch,
                "Authored child source SHA-256 does not bind to the exact expanded source bytes.");
        }
        const auto output_sha = sha256_of(std::span<const std::byte>{
            authored.bytes.data(), authored.bytes.size()});
        if (output_sha != authored.output_sha256) {
            return failure(
                NestedReintegrationStatus::child_output_mismatch,
                "Authored child output SHA-256 does not bind to the supplied replacement bytes.");
        }

        prepared.push_back(PreparedChild{
            .child = child,
            .authored = &authored,
            .changed = authored.bytes != child->payload.bytes,
        });
    }

    if (std::none_of(
            prepared.begin(), prepared.end(),
            [](const PreparedChild& value) { return value.changed; })) {
        return failure(
            NestedReintegrationStatus::no_changes,
            "All writer-validated child images are byte-identical to their expanded sources.");
    }

    // Build physical-span groups from the exact expansion so a single edited
    // alias patches the bytes shared by every declared alias identity.
    std::vector<PreparedSpan> spans;
    for (const auto& child : expansion.children) {
        if (!child.entry.populated) {
            continue;
        }
        const auto found = std::find_if(
            spans.begin(), spans.end(),
            [&child](const PreparedSpan& span) {
                return span.offset == child.entry.offset &&
                    span.size == child.entry.size;
            });
        if (found == spans.end()) {
            spans.push_back(PreparedSpan{
                .offset = child.entry.offset,
                .size = child.entry.size,
                .aliases = {&child},
                .authored = {},
                .replacement = {},
            });
        } else {
            found->aliases.push_back(&child);
        }
    }

    for (const auto& item : prepared) {
        if (!item.changed) {
            continue;
        }
        auto span = std::find_if(
            spans.begin(), spans.end(),
            [&item](const PreparedSpan& candidate) {
                return candidate.offset == item.child->entry.offset &&
                    candidate.size == item.child->entry.size;
            });
        if (span == spans.end()) {
            return failure(
                NestedReintegrationStatus::invalid_expansion,
                "A validated authored child has no parent physical-span group.");
        }
        if (span->replacement.empty()) {
            span->replacement = item.authored->bytes;
        } else if (span->replacement != item.authored->bytes) {
            return failure(
                NestedReintegrationStatus::alias_conflict,
                "Two changed alias identities request divergent bytes for one shared parent span.");
        }
        span->authored.push_back(item.authored);
    }

    spans.erase(
        std::remove_if(
            spans.begin(), spans.end(),
            [](const PreparedSpan& span) { return span.authored.empty(); }),
        spans.end());
    std::sort(
        spans.begin(), spans.end(),
        [](const PreparedSpan& left, const PreparedSpan& right) {
            if (left.offset != right.offset) {
                return left.offset < right.offset;
            }
            return left.size < right.size;
        });

    gdspaces::WorkingCopy parent_working{parent};
    std::vector<NestedSpanReceipt> span_receipts;
    span_receipts.reserve(spans.size());
    for (const auto& span : spans) {
        const auto offset = static_cast<std::size_t>(span.offset);
        const auto size = static_cast<std::size_t>(span.size);
        std::vector<std::byte> expected(
            parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
            parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset + size));
        const auto source_sha = sha256_of(
            std::span<const std::byte>{expected.data(), expected.size()});
        const auto output_sha = sha256_of(std::span<const std::byte>{
            span.replacement.data(), span.replacement.size()});

        const auto edit_result = parent_working.apply(gdspaces::EditOperation{
            .id = span_edit_id(span.offset, span.size),
            .base_revision = parent_working.revision(),
            .offset = span.offset,
            .expected = std::move(expected),
            .replacement = span.replacement,
            .description = "Reintegrate writer-validated nested child image into its materialized parent span.",
        });
        if (!edit_result.applied) {
            return failure(
                NestedReintegrationStatus::parent_edit_failed,
                "Guarded parent WorkingCopy edit failed while applying an arbitrated nested span.");
        }

        NestedSpanReceipt receipt{
            .offset = span.offset,
            .size = span.size,
            .source_sha256 = source_sha,
            .output_sha256 = output_sha,
            .affected_aliases = {},
            .authored_aliases = {},
        };
        receipt.affected_aliases.reserve(span.aliases.size());
        for (const auto* alias : span.aliases) {
            receipt.affected_aliases.push_back(alias->payload.resource.id);
        }
        receipt.authored_aliases.reserve(span.authored.size());
        for (const auto* authored : span.authored) {
            receipt.authored_aliases.push_back(authored->resource);
        }
        span_receipts.push_back(std::move(receipt));
    }

    const auto parent_rebuild = RelativeSlotLayoutWriter::rebuild(
        parent, parent_working);
    if (!parent_rebuild.ok()) {
        return failure(
            NestedReintegrationStatus::parent_rebuild_failed,
            "Reintegrated parent did not pass the canonical layout-preserving PAC/PNST writer boundary: " +
                parent_rebuild.detail);
    }

    NestedReintegrationReceipt receipt{
        .parent = parent.resource.id,
        .parent_source_sha256 = parent_rebuild.receipt->source_sha256,
        .parent_output_sha256 = parent_rebuild.receipt->output_sha256,
        .parent_revision = parent_rebuild.receipt->working_revision,
        .spans = std::move(span_receipts),
        .parent_rebuild = *parent_rebuild.receipt,
    };
    if (!receipt.valid()) {
        return failure(
            NestedReintegrationStatus::invalid_receipt,
            "Nested reintegration receipt failed internal validation.");
    }

    return NestedReintegrationResult{
        .status = NestedReintegrationStatus::ok,
        .bytes = parent_rebuild.bytes,
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
