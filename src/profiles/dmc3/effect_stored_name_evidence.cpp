#include "dmc_rengine/profiles/dmc3/effect_stored_name_evidence.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/effect_pack.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"

#include <algorithm>
#include <iomanip>
#include <span>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

void add_error(
    EffectStoredNameApplyResult& result,
    const gdspaces::ResourceId& resource,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = gdspaces::DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

[[nodiscard]] const formats::ContainerEntry* slot_entry(
    const formats::ContainerDocument& document,
    std::uint32_t slot) noexcept {
    const auto found = std::find_if(
        document.entries.begin(), document.entries.end(),
        [slot](const formats::ContainerEntry& entry) {
            return entry.slot_index == slot;
        });
    return found == document.entries.end() ? nullptr : &*found;
}

[[nodiscard]] std::string slot_component(std::uint32_t slot) {
    std::ostringstream output;
    output << "slot-" << std::setfill('0') << std::setw(4) << slot;
    return output.str();
}

[[nodiscard]] gdspaces::ResourceId expected_records_id(
    const gdspaces::ResourceId& enclosing,
    const formats::ContainerEntry& records_entry) {
    auto chain = enclosing.container_chain;
    if (!chain.empty()) {
        chain.push_back('/');
    }
    chain.append("PNST[1]");
    return gdspaces::ResourceId{
        .source_id = enclosing.source_id,
        .logical_path = enclosing.logical_path + "::PNST/" +
            slot_component(static_cast<std::uint32_t>(
                EffectPackContract::records_slot_index)),
        .container_chain = std::move(chain),
        .offset = enclosing.offset + records_entry.offset,
        .size = records_entry.size,
    };
}

[[nodiscard]] bool same_evidence(
    const gdspaces::EnclosingContainerNameEvidence& evidence,
    const gdspaces::ResourceId& authority,
    std::string_view digest,
    const formats::EffectRecord& record) noexcept {
    return evidence.valid() && evidence.authority_resource() == authority &&
        evidence.authority_sha256() == digest &&
        evidence.raw_label() == record.name &&
        evidence.normalized_name() == record.name &&
        evidence.physical_slot_index() == record.slot_index &&
        evidence.source_line() == record.source_line;
}

} // namespace

bool EffectStoredNameApplyResult::ok() const noexcept {
    return applicable && applied && !has_error(diagnostics);
}

EffectStoredNameApplyResult EffectStoredNameEvidenceBuilder::apply(
    const gdspaces::ResourcePayload& enclosing_container,
    gdspaces::ContainerExpansion& records_expansion) {
    EffectStoredNameApplyResult result;
    const auto original = records_expansion;

    if (!enclosing_container.readable() || !records_expansion.usable()) {
        add_error(
            result,
            enclosing_container.resource.id,
            "gdspaces.effect-names.unreadable-context",
            "Effect stored-name binding requires a readable enclosing container and usable records expansion.");
        return result;
    }

    const auto enclosing_bytes = std::span<const std::byte>{
        enclosing_container.bytes.data(), enclosing_container.bytes.size()};
    const auto parsed = formats::EffectPackParser::parse(enclosing_bytes);
    if (!parsed.ok()) {
        // An explicitly supplied enclosing context is authority-bearing input;
        // a structural near miss must fail closed rather than silently naming.
        add_error(
            result,
            enclosing_container.resource.id,
            "gdspaces.effect-names.invalid-enclosing-container",
            "Supplied enclosing container does not satisfy the recovered effect-pack contract: " +
                parsed.message);
        return result;
    }
    result.applicable = true;

    const auto outer = formats::PnstParser::parse(enclosing_bytes);
    if (!outer.ok()) {
        add_error(
            result,
            enclosing_container.resource.id,
            "gdspaces.effect-names.outer-reparse-failed",
            "Validated effect container could not be rebound to its physical outer PNST structure.");
        return result;
    }
    const auto* records_entry = slot_entry(
        *outer.document,
        static_cast<std::uint32_t>(EffectPackContract::records_slot_index));
    if (records_entry == nullptr || !records_entry->populated) {
        add_error(
            result,
            enclosing_container.resource.id,
            "gdspaces.effect-names.records-slot-missing",
            "Validated effect container no longer exposes physical records slot 1.");
        return result;
    }

    const auto expected_parent = expected_records_id(
        enclosing_container.resource.id, *records_entry);
    if (records_expansion.parent.id != expected_parent ||
        records_expansion.parser_format != "PNST") {
        add_error(
            result,
            records_expansion.parent.id,
            "gdspaces.effect-names.parent-identity-mismatch",
            "Records expansion is not the exact physical PNST slot-1 child of the supplied enclosing effect container.");
        return result;
    }

    const auto digest = core::Sha256::compute(enclosing_bytes).hex();
    auto staged = records_expansion;
    std::size_t bound = 0U;

    for (const auto& record : parsed.document->records) {
        const auto child_it = std::find_if(
            staged.children.begin(), staged.children.end(),
            [&record](const gdspaces::ContainerChild& child) {
                return child.entry.slot_index == record.slot_index;
            });
        if (child_it == staged.children.end() || !child_it->entry.populated) {
            add_error(
                result,
                records_expansion.parent.id,
                "gdspaces.effect-names.record-slot-missing",
                "Effect manifest names a populated physical record slot absent from the materialized records expansion.");
            records_expansion = original;
            return result;
        }

        const auto expected_offset = enclosing_container.resource.id.offset + record.offset;
        if (child_it->payload.resource.id.source_id != enclosing_container.resource.id.source_id ||
            child_it->payload.resource.id.offset != expected_offset ||
            child_it->payload.resource.id.size != record.extent ||
            child_it->payload.bytes.size() != record.extent) {
            add_error(
                result,
                child_it->payload.resource.id,
                "gdspaces.effect-names.record-physical-mismatch",
                "Stored effect name could not be bound to the exact physical record byte range.");
            records_expansion = original;
            return result;
        }

        const auto relative = static_cast<std::size_t>(record.offset);
        const auto extent = static_cast<std::size_t>(record.extent);
        if (relative > enclosing_container.bytes.size() ||
            extent > enclosing_container.bytes.size() - relative ||
            !std::equal(
                child_it->payload.bytes.begin(),
                child_it->payload.bytes.end(),
                enclosing_container.bytes.begin() + static_cast<std::ptrdiff_t>(relative))) {
            add_error(
                result,
                child_it->payload.resource.id,
                "gdspaces.effect-names.record-bytes-mismatch",
                "Materialized record bytes differ from the byte range named by the enclosing effect manifest.");
            records_expansion = original;
            return result;
        }

        if (!child_it->payload.enclosing_container_name_evidence.empty()) {
            const bool identical = child_it->payload.enclosing_container_name_evidence.size() == 1U &&
                same_evidence(
                    child_it->payload.enclosing_container_name_evidence.front(),
                    enclosing_container.resource.id,
                    digest,
                    record);
            if (!identical) {
                add_error(
                    result,
                    child_it->payload.resource.id,
                    "gdspaces.effect-names.authority-conflict",
                    "A physical child already carries different enclosing-container naming evidence.");
                records_expansion = original;
                return result;
            }
            ++bound;
            continue;
        }

        child_it->payload.enclosing_container_name_evidence.emplace_back(
            enclosing_container.resource.id,
            digest,
            record.name,
            record.name,
            record.slot_index,
            record.source_line);
        ++bound;
    }

    const auto populated_count = static_cast<std::size_t>(std::count_if(
        staged.children.begin(), staged.children.end(),
        [](const gdspaces::ContainerChild& child) {
            return child.entry.populated;
        }));
    if (bound != populated_count || bound != parsed.document->records.size()) {
        add_error(
            result,
            records_expansion.parent.id,
            "gdspaces.effect-names.incomplete-binding",
            "Enclosing effect manifest must bind exactly one stored name to every populated records payload.");
        records_expansion = original;
        return result;
    }

    records_expansion = std::move(staged);
    result.applied = true;
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
