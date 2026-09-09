#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/native_reader_registry.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::gdspaces::ResourceClassifier;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::integration::FormatIntegrationRegistry;
using dmc::rengine::integration::NativeReaderModuleRegistry;
using dmc::rengine::integration::ToolRegistry;
using dmc::rengine::integration::ToolRoute;
using dmc::rengine::integration::ToolRouteRole;
using dmc::rengine::profiles::dmc3::AnimationTypeContract;
using dmc::rengine::profiles::dmc3::ResourceTypeContract;

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> out;
    out.reserve(text.size());
    for (const auto character : text) {
        out.push_back(static_cast<std::byte>(
            static_cast<unsigned char>(character)));
    }
    return out;
}

[[nodiscard]] bool uses_non_native_reader_parser(
    std::string_view format) noexcept {
    // PAC/PNST parser validation is owned by the relative-slot container
    // workspace path. SLTC is its synthetic-only regression format. They are
    // intentionally not NativeReaderModule entries.
    return format == "pac" || format == "pnst" || format == "sltc";
}

[[nodiscard]] bool has_tool_route(
    const std::vector<ToolRoute>& routes,
    dmc::rengine::gdspaces::ToolTarget target) {
    return std::any_of(
        routes.begin(), routes.end(),
        [target](const ToolRoute& route) {
            return route.target == target;
        });
}

[[nodiscard]] bool has_primary_tool_route(
    const std::vector<ToolRoute>& routes,
    dmc::rengine::gdspaces::ToolTarget target) {
    return std::any_of(
        routes.begin(), routes.end(),
        [target](const ToolRoute& route) {
            return route.target == target && route.role == ToolRouteRole::primary;
        });
}

[[nodiscard]] ResourceRef contract_resource(std::string_view format) {
    return ResourceRef{
        .id = ResourceId{
            .source_id = "registry-contract",
            .logical_path = std::string{"contract."} + std::string{format},
            .container_chain = {},
            .offset = 0U,
            .size = 1U,
        },
        .display_name = std::string{"contract."} + std::string{format},
        .format = std::string{format},
        .profile = "dmc3-hd",
        .synthetic_name = true,
        .container = false,
    };
}

// The recovered runtime contract is the census authority. Any type it declares
// must be nameable by the product, otherwise the classifier and the registry
// drift away from the evidence that is already in the tree.
void test_every_contract_family_type_is_registered() {
    const FormatIntegrationRegistry registry;
    for (const auto& entry : ResourceTypeContract::family_tagged_types) {
        const auto format = ResourceTypeContract::canonical_extension(entry.mask);
        assert(!format.empty());
        assert(registry.find(format) != nullptr);
    }
}

void test_every_contract_registry_type_is_registered() {
    const FormatIntegrationRegistry registry;
    for (const auto& entry : ResourceTypeContract::registry_tagged_types) {
        const auto format = ResourceTypeContract::canonical_extension(entry.code);
        assert(!format.empty());
        assert(registry.find(format) != nullptr);
    }
}

void test_every_contract_extension_type_is_registered() {
    const FormatIntegrationRegistry registry;
    for (const auto& entry : ResourceTypeContract::extension_types) {
        const auto format = ResourceTypeContract::canonical_extension(entry.code);
        assert(!format.empty());
        assert(registry.find(format) != nullptr);
    }
}

// The second recovered registry (0x1402E01A0) is a separate census authority
// from ResourceTypeContract, with its own table, its own capacity and its own
// type codes. Holding only the first one to this standard is what let TSC and
// HID sit in the evidence for weeks without a registry row: nothing compared
// the second table against the product. `.clt` appears in both, so a format
// covered here is not necessarily covered there.
void test_every_animation_contract_extension_type_is_registered() {
    const FormatIntegrationRegistry registry;
    for (const auto& entry : AnimationTypeContract::extension_types) {
        // The table enumerates case in pairs; the registry is keyed lowercase.
        const auto extension = entry.extension.substr(1U);
        std::string format;
        for (const auto character : extension) {
            format.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))));
        }
        assert(!format.empty());
        assert(registry.find(format) != nullptr);
    }
}

// Every registry row must be internally valid and uniquely addressable. This
// catches duplicate format keys and impossible capability combinations before
// they can become workspace/routing ambiguity.
void test_format_registry_is_internally_consistent() {
    const FormatIntegrationRegistry registry;
    for (const auto& descriptor : registry.formats()) {
        assert(descriptor.valid());
        assert(registry.find(descriptor.format) == &descriptor);
    }
}

// Native Reader, Format Registry and Tool Registry are three independent
// integration surfaces. They must agree on parser authority and on the tool
// that receives parser-completion evidence. A drift here previously allowed a
// parser to run successfully while parser-validation publication failed.
void test_native_reader_registry_and_tool_routes_are_coherent() {
    const FormatIntegrationRegistry formats;
    const NativeReaderModuleRegistry modules;
    const ToolRegistry tools;

    for (const auto& descriptor : formats.formats()) {
        const auto resource = contract_resource(descriptor.format);
        const auto default_routes = tools.routes_for(resource, false, false, true);
        assert(!default_routes.empty());

        for (std::size_t index = 0U; index < default_routes.size(); ++index) {
            const auto& route = default_routes[index];
            assert(route.valid());
            assert(tools.find(route.target) != nullptr);
            for (std::size_t other = index + 1U;
                 other < default_routes.size();
                 ++other) {
                assert(default_routes[other].target != route.target);
            }
        }

        if (descriptor.parser_id.empty()) {
            continue;
        }

        const auto* module = modules.find(descriptor.parser_id);
        if (uses_non_native_reader_parser(descriptor.format)) {
            assert(module == nullptr);
            continue;
        }

        assert(module != nullptr);
        assert(module->valid());
        assert(tools.find(module->consumer) != nullptr);

        const auto contextual_routes = tools.routes_for(
            resource,
            descriptor.stage_category.has_value(),
            false,
            true);
        assert(has_tool_route(contextual_routes, module->consumer));

        // Scene-native parsers publish their completion to ModViz. In the
        // default non-stage context ModViz must therefore be the primary owner,
        // not merely a companion bolted on later by ToolRegistry.
        if (module->consumer == dmc::rengine::gdspaces::ToolTarget::modviz_scene) {
            assert(has_primary_tool_route(default_routes, module->consumer));
        }
    }

    for (const auto& module : modules.modules()) {
        assert(module.valid());
        assert(tools.find(module.consumer) != nullptr);

        bool referenced_by_format = false;
        for (const auto& descriptor : formats.formats()) {
            if (descriptor.parser_id == module.parser_id) {
                referenced_by_format = true;
                const auto routes = tools.routes_for(
                    contract_resource(descriptor.format),
                    descriptor.stage_category.has_value(),
                    false,
                    true);
                assert(has_tool_route(routes, module.consumer));
            }
        }
        assert(referenced_by_format);
    }
}

// Extensions observed in the bound retail central-directory surface. Recording
// them here keeps the census result and the registry from separating.
void test_observed_retail_extensions_are_registered() {
    const FormatIntegrationRegistry registry;
    constexpr std::array<std::string_view, 11> observed{
        "pac", "txt", "bin", "tm2", "mod", "fon", "ptx", "bd", "phd", "tsb",
        "nbz",
    };
    for (const auto format : observed) {
        assert(registry.find(format) != nullptr);
    }
}

// The family-mask probe requires a trailing ASCII space at byte 3. The
// classifier must reproduce that exactly rather than matching a 3-byte prefix.
void test_classifier_recognizes_family_mask_tags() {
    for (const auto& entry : ResourceTypeContract::family_tagged_types) {
        const auto payload = bytes_of(
            std::string{entry.tag_with_space} + "payload-body-padding");
        const auto classification = ResourceClassifier::classify(
            "unnamed", std::span<const std::byte>{payload});
        assert(classification.format ==
               ResourceTypeContract::canonical_extension(entry.mask));
        assert(classification.magic_confirmed);
    }
}

void test_family_tag_without_trailing_space_is_not_confirmed() {
    // "MODX" is not a family tag. Without a path extension there is nothing
    // left to fall back to, so it must stay unknown rather than become "mod".
    const auto payload = bytes_of("MODXpayload");
    const auto classification = ResourceClassifier::classify(
        "unnamed", std::span<const std::byte>{payload});
    assert(classification.format == "unknown");
    assert(!classification.magic_confirmed);
}

// Regression for the container walk stopping at a nested archive that did not
// happen to be named ".nbz".
void test_zip_magic_is_recognized_as_nbz_container() {
    const auto payload = bytes_of(
        std::string{"PK\x03\x04", 4U} + "rest-of-a-local-file-header");
    const auto classification = ResourceClassifier::classify(
        "slot_0003.bin", std::span<const std::byte>{payload});
    assert(classification.format == "nbz");
    assert(classification.magic_confirmed);
    assert(classification.container);
}

// Byte identity must outrank a misleading path extension.
void test_magic_outranks_extension() {
    const auto payload = bytes_of("MOD payload-body-padding");
    const auto classification = ResourceClassifier::classify(
        "actually_named.ptx", std::span<const std::byte>{payload});
    assert(classification.format == "mod");
}

// With no bytes to probe, the path extension remains the only signal.
void test_extension_fallback_still_applies() {
    const auto classification = ResourceClassifier::classify("tex/basic.ptx");
    assert(classification.format == "ptx");
    assert(!classification.magic_confirmed);
    assert(!classification.container);
}

} // namespace

int main() {
    test_every_contract_family_type_is_registered();
    test_every_contract_registry_type_is_registered();
    test_every_contract_extension_type_is_registered();
    test_every_animation_contract_extension_type_is_registered();
    test_format_registry_is_internally_consistent();
    test_native_reader_registry_and_tool_routes_are_coherent();
    test_observed_retail_extensions_are_registered();
    test_classifier_recognizes_family_mask_tags();
    test_family_tag_without_trailing_space_is_not_confirmed();
    test_zip_magic_is_recognized_as_nbz_container();
    test_magic_outranks_extension();
    test_extension_fallback_still_applies();
    return 0;
}
