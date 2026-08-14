#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

enum class StageResourceCategory {
    scripts,
    models,
    textures,
    animations,
    cameras,
    lighting,
    events,
    positions,
    effects,
    collision,
    sounds,
    unknown,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageResourceCategory category) noexcept {
    switch (category) {
    case StageResourceCategory::scripts: return "scripts";
    case StageResourceCategory::models: return "models";
    case StageResourceCategory::textures: return "textures";
    case StageResourceCategory::animations: return "animations";
    case StageResourceCategory::cameras: return "cameras";
    case StageResourceCategory::lighting: return "lighting";
    case StageResourceCategory::events: return "events";
    case StageResourceCategory::positions: return "positions";
    case StageResourceCategory::effects: return "effects";
    case StageResourceCategory::collision: return "collision";
    case StageResourceCategory::sounds: return "sounds";
    case StageResourceCategory::unknown: return "unknown";
    }
    return "unknown";
}

struct StageIdentity final {
    std::string profile;

    // Evidence-backed semantic gameplay-stage identity when known. This may be
    // empty for an executable-derived resource-set/catalog row whose gameplay
    // meaning has not yet been established.
    std::string stage_id;

    std::string display_name;
    std::string exe_evidence_id;

    // Stable technical identity for the resource grouping represented by this
    // StageBundle. DMC3 StageCatalog entries use their catalog_entry_id here.
    // It is intentionally separate from semantic stage_id. Trailing placement
    // preserves source compatibility with existing aggregate initializers.
    std::string resource_set_id;

    [[nodiscard]] std::string_view resource_set_key() const noexcept {
        return resource_set_id.empty()
            ? std::string_view{stage_id}
            : std::string_view{resource_set_id};
    }

    [[nodiscard]] bool semantic_stage_known() const noexcept {
        return !stage_id.empty();
    }

    [[nodiscard]] bool valid() const noexcept {
        return !resource_set_key().empty();
    }
};

struct StageMember final {
    StageResourceCategory category{StageResourceCategory::unknown};
    ResourceRef resource;
    std::string role;

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid();
    }
};

class StageBundle final {
public:
    explicit StageBundle(StageIdentity identity);

    [[nodiscard]] const StageIdentity& identity() const noexcept;
    [[nodiscard]] bool add(StageMember member);
    void add_diagnostic(Diagnostic diagnostic);
    [[nodiscard]] std::vector<const StageMember*> members(
        StageResourceCategory category) const;
    [[nodiscard]] const std::vector<StageMember>& all_members() const noexcept;
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    StageIdentity identity_;
    std::vector<StageMember> members_;
    std::vector<Diagnostic> diagnostics_;
};

} // namespace dmc::rengine::gdspaces
