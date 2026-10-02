#include "dmc_rengine/hits/gdspaces_tool.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace dmc::rengine::hits::gdspaces_tool {
namespace {

[[nodiscard]] std::string normalized_format(std::string format) {
    std::transform(
        format.begin(),
        format.end(),
        format.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return format;
}

} // namespace

Session::Session(
    gdspaces::ResourcePayload source_payload,
    editor::Session editor)
    : source_payload_(std::move(source_payload)),
      editor_(std::move(editor)) {}

std::optional<Session> Session::open(
    gdspaces::ResourcePayload payload) {
    if (!payload.readable() ||
        normalized_format(payload.resource.format) != "hits" ||
        payload.bytes.empty()) {
        return std::nullopt;
    }

    auto opened = editor::Session::open(payload.bytes);
    if (!opened) {
        return std::nullopt;
    }

    return Session{
        std::move(payload),
        std::move(*opened),
    };
}

const gdspaces::ResourcePayload& Session::source_payload() const noexcept {
    return source_payload_;
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

CommitResult Session::commit() const {
    if (!editor_.dirty()) {
        return CommitResult{
            .status = CommitStatus::no_changes,
            .payload = source_payload_,
            .detail =
                "No authored HITS changes; original GDSpaces payload preserved.",
        };
    }

    auto rebuilt = editor_.rebuild();
    if (!rebuilt.ok()) {
        return CommitResult{
            .status = CommitStatus::rebuild_failed,
            .payload = source_payload_,
            .detail =
                "Canonical HITS rebuild failed; GDSpaces payload not modified.",
        };
    }

    auto output = source_payload_;
    output.bytes = std::move(rebuilt.bytes);
    output.resource.id.size =
        static_cast<std::uint64_t>(output.bytes.size());

    return CommitResult{
        .status = CommitStatus::ok,
        .payload = std::move(output),
        .detail =
            "Rebuilt HITS bytes returned in the original GDSpaces resource envelope.",
    };
}

} // namespace dmc::rengine::hits::gdspaces_tool
