#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/hits/editor.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace dmc::rengine::hits::gdspaces_tool {

enum class CommitStatus : std::uint8_t {
    ok = 0,
    no_changes,
    rebuild_failed,
};

struct CommitResult final {
    CommitStatus status{CommitStatus::no_changes};
    gdspaces::ResourcePayload payload;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == CommitStatus::ok ||
            status == CommitStatus::no_changes;
    }
};

// Embedded Rengine/GDSpaces boundary.
//
// GDSpaces owns discovery, identity, source/container resolution and eventual
// reintegration. This tool receives one already-materialized HITS ResourcePayload
// and returns the same resource identity/evidence envelope with rebuilt bytes.
// It never performs its own archive search or creates a competing resolver.
class Session final {
public:
    [[nodiscard]] static std::optional<Session> open(
        gdspaces::ResourcePayload payload);

    [[nodiscard]] const gdspaces::ResourcePayload& source_payload() const noexcept;
    [[nodiscard]] editor::Session& editor() noexcept;
    [[nodiscard]] const editor::Session& editor() const noexcept;
    [[nodiscard]] bool dirty() const noexcept;

    [[nodiscard]] CommitResult commit() const;

private:
    Session(
        gdspaces::ResourcePayload source_payload,
        editor::Session editor);

    gdspaces::ResourcePayload source_payload_;
    editor::Session editor_;
};

} // namespace dmc::rengine::hits::gdspaces_tool
