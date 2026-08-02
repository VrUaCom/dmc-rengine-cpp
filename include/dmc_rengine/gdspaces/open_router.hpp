#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

enum class ToolTarget {
    spider_hub,
    binary_inspector,
    exe_editor,
    evidence_registry,
    stage_ops,
    modviz_scene,
    modviz_menu,
    item_editor,
    build_test_lab,
};

[[nodiscard]] constexpr std::string_view to_string(ToolTarget target) noexcept {
    switch (target) {
    case ToolTarget::spider_hub: return "spider-hub";
    case ToolTarget::binary_inspector: return "binary-inspector";
    case ToolTarget::exe_editor: return "exe-editor";
    case ToolTarget::evidence_registry: return "evidence-registry";
    case ToolTarget::stage_ops: return "stage-ops";
    case ToolTarget::modviz_scene: return "modviz-scene";
    case ToolTarget::modviz_menu: return "modviz-menu";
    case ToolTarget::item_editor: return "item-editor";
    case ToolTarget::build_test_lab: return "build-test-lab";
    }
    return "binary-inspector";
}

struct OpenRequest final {
    ResourceRef resource;
    std::optional<ToolTarget> preferred_target;
    bool stage_context{false};
    bool menu_context{false};
    bool evidence_context{true};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid();
    }
};

class OpenRouter final {
public:
    OpenRouter();

    void set_route(std::string format, ToolTarget target);
    [[nodiscard]] ToolTarget route(const OpenRequest& request) const;

private:
    std::map<std::string, ToolTarget, std::less<>> routes_;
};

} // namespace dmc::rengine::gdspaces
