#include "dmc_rengine/hits/pac_session.hpp"

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/hits/pac_editor.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <span>
#include <utility>

namespace dmc::rengine::hits::pac_session {
namespace {

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

[[nodiscard]] std::optional<Source> open_source(
    const gdspaces::ContainerExpansion& expansion,
    std::uint32_t slot_index,
    evidence::StaticSourceProfile profile) {
    const auto* child = find_slot(expansion, slot_index);
    if (child == nullptr) {
        return std::nullopt;
    }

    auto opened = editor::Session::open(
        std::span<const std::byte>{
            child->payload.bytes.data(),
            child->payload.bytes.size()});
    if (!opened) {
        return std::nullopt;
    }

    return Source{
        .slot_index = slot_index,
        .profile = profile,
        .editor = std::move(*opened),
    };
}

} // namespace

PacSession::PacSession(
    gdspaces::ResourcePayload parent_pac,
    std::optional<Source> source0,
    std::optional<Source> source1)
    : parent_pac_(std::move(parent_pac)),
      source0_(std::move(source0)),
      source1_(std::move(source1)) {}

std::optional<PacSession> PacSession::open(
    gdspaces::ResourcePayload parent_pac) {
    if (!parent_pac.readable() || parent_pac.bytes.empty()) {
        return std::nullopt;
    }

    const auto registry =
        profiles::dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{
            parent_pac.bytes.data(),
            parent_pac.bytes.size()},
        parent_pac.resource.id.logical_path);
    if (!parsed.ok() || parsed.document.format != "PAC") {
        return std::nullopt;
    }

    const auto expansion =
        gdspaces::ContainerExpander::expand(parent_pac, parsed);
    if (!expansion.usable() || expansion.parser_format != "PAC") {
        return std::nullopt;
    }

    auto source0 = open_source(
        expansion,
        3U,
        evidence::StaticSourceProfile::source_0_member_3);
    auto source1 = open_source(
        expansion,
        6U,
        evidence::StaticSourceProfile::source_1_member_6);

    if (!source0 && !source1) {
        return std::nullopt;
    }

    return PacSession{
        std::move(parent_pac),
        std::move(source0),
        std::move(source1),
    };
}

const gdspaces::ResourcePayload& PacSession::parent() const noexcept {
    return parent_pac_;
}

Source* PacSession::source0() noexcept {
    return source0_ ? &*source0_ : nullptr;
}

const Source* PacSession::source0() const noexcept {
    return source0_ ? &*source0_ : nullptr;
}

Source* PacSession::source1() noexcept {
    return source1_ ? &*source1_ : nullptr;
}

const Source* PacSession::source1() const noexcept {
    return source1_ ? &*source1_ : nullptr;
}

bool PacSession::dirty() const noexcept {
    return (source0_ && source0_->editor.dirty()) ||
        (source1_ && source1_->editor.dirty());
}

BuildResult PacSession::rebuild_pac() const {
    if (!dirty()) {
        return BuildResult{
            .status = BuildStatus::no_changes,
            .bytes = parent_pac_.bytes,
            .replaced_slots = {},
            .detail = "No HITS source has authored changes.",
        };
    }

    auto current_parent = parent_pac_;
    std::vector<std::uint32_t> replaced_slots;

    const auto rebuild_source =
        [&current_parent, &replaced_slots](
            const Source& source) -> std::optional<BuildResult> {
        if (!source.editor.dirty()) {
            return std::nullopt;
        }

        const auto rebuilt_hits = source.editor.rebuild();
        if (!rebuilt_hits.ok()) {
            return BuildResult{
                .status = BuildStatus::hits_rebuild_failed,
                .bytes = {},
                .replaced_slots = replaced_slots,
                .detail =
                    "Canonical HITS rebuild failed before PAC integration.",
            };
        }

        auto replaced = pac_editor::PacHitsWriter::replace_slot(
            current_parent,
            source.slot_index,
            rebuilt_hits.bytes,
            source.editor.revision());
        if (!replaced.ok()) {
            return BuildResult{
                .status = BuildStatus::pac_replace_failed,
                .bytes = {},
                .replaced_slots = replaced_slots,
                .detail = replaced.detail,
            };
        }

        current_parent.bytes = std::move(replaced.bytes);
        current_parent.resource.id.size =
            static_cast<std::uint64_t>(current_parent.bytes.size());
        replaced_slots.push_back(source.slot_index);
        return std::nullopt;
    };

    if (source0_) {
        if (auto failure = rebuild_source(*source0_)) {
            return std::move(*failure);
        }
    }
    if (source1_) {
        if (auto failure = rebuild_source(*source1_)) {
            return std::move(*failure);
        }
    }

    return BuildResult{
        .status = BuildStatus::ok,
        .bytes = std::move(current_parent.bytes),
        .replaced_slots = std::move(replaced_slots),
        .detail =
            "All dirty HITS sources were rebuilt and reinserted into PAC.",
    };
}

} // namespace dmc::rengine::hits::pac_session
