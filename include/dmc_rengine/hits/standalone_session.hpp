#pragma once

#include "dmc_rengine/hits/editor.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::hits::standalone {

enum class SaveStatus : std::uint8_t {
    ok = 0,
    no_changes,
    rebuild_failed,
};

struct SaveResult final {
    SaveStatus status{SaveStatus::no_changes};
    std::vector<std::byte> bytes;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == SaveStatus::ok ||
            status == SaveStatus::no_changes;
    }
};

// Standalone product boundary.
//
// This session intentionally has no GDSpaces dependency. A desktop/mobile shell
// may open raw HITS bytes, expose the shared native editor::Session, then save
// rebuilt HITS bytes back to a file or any shell-owned container workflow.
class Session final {
public:
    [[nodiscard]] static std::optional<Session> open(
        std::span<const std::byte> source_bytes);

    [[nodiscard]] editor::Session& editor() noexcept;
    [[nodiscard]] const editor::Session& editor() const noexcept;
    [[nodiscard]] bool dirty() const noexcept;

    [[nodiscard]] SaveResult save() const;

private:
    explicit Session(editor::Session editor);

    editor::Session editor_;
};

} // namespace dmc::rengine::hits::standalone
