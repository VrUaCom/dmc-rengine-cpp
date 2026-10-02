#include "dmc_rengine/hits/pac_editor.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::hits::pac_editor {
namespace {

[[nodiscard]] std::string sha256_of(
    std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] ReplaceResult failure(
    ReplaceStatus status,
    std::uint32_t slot_index,
    std::string detail) {
    return ReplaceResult{
        .status = status,
        .slot_index = slot_index,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] const gdspaces::ContainerChild* find_slot(
    const gdspaces::ContainerExpansion& expansion,
    std::uint32_t slot_index) noexcept {
    const auto found = std::find_if(
        expansion.children.begin(),
        expansion.children.end(),
        [slot_index](const gdspaces::ContainerChild& child) {
            return child.entry.slot_index == slot_index &&
                child.entry.populated;
        });
    return found == expansion.children.end() ? nullptr : &*found;
}

} // namespace

ReplaceResult PacHitsWriter::replace_slot(
    const gdspaces::ResourcePayload& parent_pac,
    std::uint32_t slot_index,
    std::span<const std::byte> replacement_hits,
    std::uint64_t revision) {
    if (!parent_pac.readable() || parent_pac.bytes.empty()) {
        return failure(
            ReplaceStatus::invalid_parent,
            slot_index,
            "The parent PAC payload is not readable.");
    }

    const auto replacement_scan =
        formats::hits::RecordScanner::scan(replacement_hits);
    if (!replacement_scan.ok()) {
        return failure(
            ReplaceStatus::invalid_replacement_hits,
            slot_index,
            "Replacement bytes do not pass the canonical HITS parser.");
    }

    const auto registry =
        profiles::dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{
            parent_pac.bytes.data(),
            parent_pac.bytes.size()},
        parent_pac.resource.id.logical_path);
    if (!parsed.ok() || parsed.document.format != "PAC") {
        return failure(
            ReplaceStatus::invalid_parent,
            slot_index,
            "The parent resource does not parse as a canonical PAC container.");
    }

    const auto expansion =
        gdspaces::ContainerExpander::expand(parent_pac, parsed);
    if (!expansion.usable() || expansion.parser_format != "pac") {
        return failure(
            ReplaceStatus::invalid_parent,
            slot_index,
            "PAC expansion failed before HITS replacement.");
    }

    const auto* target = find_slot(expansion, slot_index);
    if (target == nullptr) {
        return failure(
            ReplaceStatus::target_slot_not_found,
            slot_index,
            "The requested physical PAC slot is empty or absent.");
    }

    const auto source_hits = formats::hits::RecordScanner::scan(
        std::span<const std::byte>{
            target->payload.bytes.data(),
            target->payload.bytes.size()});
    if (!source_hits.ok()) {
        return failure(
            ReplaceStatus::target_slot_not_hits,
            slot_index,
            "The requested PAC slot is populated but is not canonical HITS.");
    }

    std::vector<std::byte> authored_bytes{
        replacement_hits.begin(),
        replacement_hits.end()};

    const profiles::dmc3::AuthoredChildImage authored{
        .resource = target->payload.resource.id,
        .source_sha256 = sha256_of(
            std::span<const std::byte>{
                target->payload.bytes.data(),
                target->payload.bytes.size()}),
        .output_sha256 = sha256_of(replacement_hits),
        .revision = revision,
        .writer_mode = "hits-editor-spatial-rebuild",
        .bytes = authored_bytes,
    };
    const std::array<profiles::dmc3::AuthoredChildImage, 1U>
        authored_children{authored};

    auto rebuilt =
        profiles::dmc3::RelativeSlotPackedReflowWriter::rebuild(
            parent_pac,
            expansion,
            authored_children);
    if (!rebuilt.ok()) {
        return failure(
            ReplaceStatus::reflow_failed,
            slot_index,
            std::string{"PAC reflow failed: "} +
                std::string{
                    profiles::dmc3::to_string(rebuilt.status)} +
                (rebuilt.detail.empty()
                    ? std::string{}
                    : std::string{"; "} + rebuilt.detail));
    }

    auto reopened_parent = parent_pac;
    reopened_parent.bytes = rebuilt.bytes;
    reopened_parent.resource.id.size =
        static_cast<std::uint64_t>(reopened_parent.bytes.size());

    const auto reparsed = registry.parse(
        std::span<const std::byte>{
            reopened_parent.bytes.data(),
            reopened_parent.bytes.size()},
        reopened_parent.resource.id.logical_path);
    if (!reparsed.ok() || reparsed.document.format != "PAC") {
        return failure(
            ReplaceStatus::reopen_failed,
            slot_index,
            "Rebuilt PAC failed canonical reopen.");
    }

    const auto reopened_expansion =
        gdspaces::ContainerExpander::expand(
            reopened_parent,
            reparsed);
    if (!reopened_expansion.usable()) {
        return failure(
            ReplaceStatus::reopen_failed,
            slot_index,
            "Rebuilt PAC failed container expansion.");
    }

    const auto* reopened_target =
        find_slot(reopened_expansion, slot_index);
    if (reopened_target == nullptr) {
        return failure(
            ReplaceStatus::reopen_failed,
            slot_index,
            "Rebuilt PAC no longer contains the target physical slot.");
    }

    if (reopened_target->payload.bytes != authored_bytes) {
        return failure(
            ReplaceStatus::replacement_mismatch,
            slot_index,
            "Reopened PAC target bytes differ from the authored HITS image.");
    }

    const auto reopened_hits =
        formats::hits::RecordScanner::scan(
            std::span<const std::byte>{
                reopened_target->payload.bytes.data(),
                reopened_target->payload.bytes.size()});
    if (!reopened_hits.ok()) {
        return failure(
            ReplaceStatus::reopen_failed,
            slot_index,
            "Reopened target slot no longer passes the canonical HITS parser.");
    }

    return ReplaceResult{
        .status = ReplaceStatus::ok,
        .slot_index = slot_index,
        .bytes = std::move(rebuilt.bytes),
        .receipt = std::move(rebuilt.receipt),
        .detail = "PAC HITS slot replaced and reopened successfully.",
    };
}

} // namespace dmc::rengine::hits::pac_editor
