#include "dmc_rengine/formats/model_family.hpp"

#include <cassert>

int main() {
    namespace family = dmc::rengine::formats::model_family;

    constexpr auto scm = family::scm_profile();
    static_assert(scm.source_format == family::SourceFormat::scm);
    static_assert(family::has(scm.capabilities, family::Capability::geometry));
    static_assert(family::has(scm.capabilities, family::Capability::uv));
    static_assert(family::has(scm.capabilities, family::Capability::node_hierarchy));
    static_assert(family::has(scm.capabilities, family::Capability::alpha_control));
    static_assert(family::has(scm.capabilities, family::Capability::legacy_gs_sampler));
    static_assert(!family::has(scm.capabilities, family::Capability::skeletal_skinning));
    static_assert(family::has(scm.capabilities, family::Capability::experimental_authoring));
    static_assert(scm.max_serialized_skin_influences == 0U);
    static_assert(!scm.production_writer_authorized);

    constexpr auto mod = family::mod_profile();
    static_assert(mod.source_format == family::SourceFormat::mod);
    static_assert(family::has(mod.capabilities, family::Capability::geometry));
    static_assert(family::has(mod.capabilities, family::Capability::uv));
    static_assert(family::has(mod.capabilities, family::Capability::node_hierarchy));
    static_assert(family::has(mod.capabilities, family::Capability::skeletal_skinning));
    static_assert(!family::has(mod.capabilities, family::Capability::alpha_control));
    static_assert(!family::has(mod.capabilities, family::Capability::experimental_authoring));
    static_assert(mod.max_serialized_skin_influences == 3U);
    static_assert(!mod.production_writer_authorized);

    assert(family::to_string(scm.source_format) == "scm");
    assert(family::to_string(mod.source_format) == "mod");
    return 0;
}
