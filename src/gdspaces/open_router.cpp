#include "dmc_rengine/gdspaces/open_router.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string normalized_format(std::string format) {
    std::transform(
        format.begin(), format.end(), format.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return format;
}

} // namespace

OpenRouter::OpenRouter() {
    set_route("exe", ToolTarget::exe_editor);
    set_route("dll", ToolTarget::exe_editor);
    set_route("itm", ToolTarget::item_editor);
    set_route("scm", ToolTarget::modviz_scene);
    set_route("mod", ToolTarget::modviz_scene);
    set_route("dds", ToolTarget::modviz_scene);
    set_route("ptx", ToolTarget::modviz_scene);
    set_route("cam", ToolTarget::stage_ops);
    set_route("dca", ToolTarget::stage_ops);
    set_route("lig", ToolTarget::stage_ops);
    set_route("lig2", ToolTarget::stage_ops);
    set_route("hits", ToolTarget::stage_ops);
    set_route("txt", ToolTarget::stage_ops);
}

void OpenRouter::set_route(std::string format, ToolTarget target) {
    routes_[normalized_format(std::move(format))] = target;
}

ToolTarget OpenRouter::route(const OpenRequest& request) const {
    if (request.preferred_target.has_value()) {
        return *request.preferred_target;
    }

    if (request.menu_context) {
        return ToolTarget::modviz_menu;
    }

    if (request.stage_context) {
        return ToolTarget::stage_ops;
    }

    const auto iterator = routes_.find(normalized_format(request.resource.format));
    return iterator == routes_.end()
        ? ToolTarget::binary_inspector
        : iterator->second;
}

} // namespace dmc::rengine::gdspaces
