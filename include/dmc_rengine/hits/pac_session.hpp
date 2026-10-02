#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/hits/editor.hpp"
#include "dmc_rengine/hits/evidence_profile.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::hits::pac_session {

enum class BuildStatus : std::uint8_t {
    ok = 0,
    no_changes,
    hits_rebuild_failed,
    pac_replace_failed,
};

struct BuildResult final {
    BuildStatus status{BuildStatus::no_changes};
    std::vector<std::byte> bytes;
    std::vector<std::uint32_t> replaced_slots;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == BuildStatus::ok ||
            status == BuildStatus::no_changes;
    }
};

struct Source final {
    std::uint32_t slot_index{};
    evidence::StaticSourceProfile profile{
        evidence::StaticSourceProfile::manual_or_unknown};
    editor::Session editor;
};

class PacSession final {
public:
    [[nodiscard]] static std::optional<PacSession> open(
        gdspaces::ResourcePayload parent_pac);

    [[nodiscard]] const gdspaces::ResourcePayload& parent() const noexcept;

    [[nodiscard]] Source* source0() noexcept;
    [[nodiscard]] const Source* source0() const noexcept;

    [[nodiscard]] Source* source1() noexcept;
    [[nodiscard]] const Source* source1() const noexcept;

    [[nodiscard]] bool dirty() const noexcept;

    [[nodiscard]] BuildResult rebuild_pac() const;

private:
    PacSession(
        gdspaces::ResourcePayload parent_pac,
        std::optional<Source> source0,
        std::optional<Source> source1);

    gdspaces::ResourcePayload parent_pac_;
    std::optional<Source> source0_;
    std::optional<Source> source1_;
};

} // namespace dmc::rengine::hits::pac_session
