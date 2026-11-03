#include "dmc_rengine/hits/standalone_session.hpp"

#include <utility>

namespace dmc::rengine::hits::standalone {

Session::Session(editor::Session editor)
    : editor_(std::move(editor)) {}

std::optional<Session> Session::open(
    std::span<const std::byte> source_bytes) {
    auto opened = editor::Session::open(source_bytes);
    if (!opened) {
        return std::nullopt;
    }
    return Session{std::move(*opened)};
}

editor::Session& Session::editor() noexcept {
    return editor_;
}

const editor::Session& Session::editor() const noexcept {
    return editor_;
}

bool Session::dirty() const noexcept {
    return editor_.dirty();
}

SaveResult Session::save() const {
    if (!editor_.dirty()) {
        const auto source = editor_.source_bytes();
        return SaveResult{
            .status = SaveStatus::no_changes,
            .bytes = std::vector<std::byte>{source.begin(), source.end()},
            .detail = "No authored HITS changes; source bytes preserved.",
        };
    }

    auto rebuilt = editor_.rebuild();
    if (!rebuilt.ok()) {
        return SaveResult{
            .status = SaveStatus::rebuild_failed,
            .bytes = {},
            .detail = "Canonical HITS rebuild failed.",
        };
    }

    return SaveResult{
        .status = SaveStatus::ok,
        .bytes = std::move(rebuilt.bytes),
        .detail = "Canonical HITS resource rebuilt for standalone export.",
    };
}

} // namespace dmc::rengine::hits::standalone
