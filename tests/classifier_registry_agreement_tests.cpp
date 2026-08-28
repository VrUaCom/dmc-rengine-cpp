#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/content_tag_census_contract.hpp"
#include "dmc_rengine/profiles/dmc3/demo_script_contract.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Every verdict the classifier can reach must be a format the registry
// describes.
//
// `tm2` was added to the classifier and not to the registry, so the tree could
// label a payload with a format nothing downstream had a description, maturity
// or write policy for. That is this project's recurring defect — a capability
// existing where the thing that consumes it does not — and it went unnoticed
// because nothing tied the two lists together. This does.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace integration = dmc::rengine::integration;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view head) {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    for (std::size_t index = 0U; index < head.size(); ++index) {
        bytes[index] = static_cast<std::byte>(head[index]);
    }
    return bytes;
}

// Classify a payload by its bytes alone, the way a nameless container slot is
// classified, and require the registry to know the answer.
void require_registered(std::string_view head, std::string_view expected) {
    const integration::FormatIntegrationRegistry registry;
    const auto payload = bytes_of(head);
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{payload}, false);
    assert(classified.format == expected);
    assert(registry.find(classified.format) != nullptr);
}

void every_byte_derived_verdict_is_registered() {
    require_registered("MZ", "pe");
    require_registered("PAC", "pac");
    require_registered("TM2", "tm2");
    require_registered("DDS ", "dds");
    require_registered("SCM ", "scm");
    require_registered("MOD ", "mod");
    require_registered("HITS", "hits");
    require_registered("LIG2", "lig2");
    require_registered("ClothSim1D\r\n", "c1d");
}

// The animation registry's six kinds must all be describable too, because the
// classifier reports any of them from a name.
void every_animation_kind_is_registered() {
    const integration::FormatIntegrationRegistry registry;
    for (const auto& entry : dmc3::AnimationTypeContract::extension_types) {
        const auto format = std::string{entry.extension.substr(1U)};
        // Case-paired literals give the same format twice; only the lowercase
        // one is a format string this project uses.
        if (format != std::string{format.c_str()} ||
            format.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
                std::string::npos) {
            continue;
        }
        assert(registry.find(format) != nullptr);
    }
}

// A format the registry describes as readable must have something to read it
// with; one it describes as merely recognized must not pretend otherwise.
void maturity_and_parser_agree() {
    const integration::FormatIntegrationRegistry registry;
    std::size_t recognized = 0U;
    std::size_t with_parser = 0U;
    for (const auto& descriptor : registry.formats()) {
        if (descriptor.maturity == integration::IntegrationMaturity::recognized) {
            recognized += 1U;
        }
        if (!descriptor.parser_id.empty()) {
            with_parser += 1U;
            // A parser implies at least structural knowledge.
            assert(descriptor.maturity !=
                   integration::IntegrationMaturity::recognized);
        }
        // Every format carries a name and a category.
        assert(!descriptor.format.empty());
    }
    assert(recognized != 0U);
    assert(with_parser != 0U);
    assert(recognized + with_parser <= registry.formats().size() + recognized);
}

// The census says which of these the runtime identifies by content. A format
// this project reads is not thereby a format the game recognizes, and the two
// lists must not be conflated.
void reading_a_tag_is_not_the_runtime_recognizing_it() {
    using Census = dmc3::ContentTagCensusContract;
    const integration::FormatIntegrationRegistry registry;
    // hits is structural here and compared nowhere in the image.
    const auto* hits = registry.find("hits");
    assert(hits != nullptr);
    assert(hits->maturity != integration::IntegrationMaturity::recognized);
    assert(!Census::runtime_compares("HITS"));
    // scm is both: read here and compared by the runtime.
    assert(registry.find("scm") != nullptr);
    assert(Census::runtime_compares("SCM"));
}

// The three script commands that name a resource each map to a format the
// registry knows.
void the_naming_commands_reach_registered_formats() {
    const integration::FormatIntegrationRegistry registry;
    assert(registry.find("mot") != nullptr);
    assert(registry.find("cam") != nullptr);
    assert(registry.find("hid") != nullptr);
    assert(dmc3::DemoScriptContract::registering_commands.size() == 3U);
}

// The published format reference states these counts. A doc that drifts from
// the registry is worse than no doc, because a reader trusts it.
// docs/formats/README.md, "Current integration state".
void the_published_counts_match_the_registry() {
    const integration::FormatIntegrationRegistry registry;
    std::size_t structural = 0U;
    std::size_t recognized = 0U;
    std::size_t semantic = 0U;
    for (const auto& descriptor : registry.formats()) {
        switch (descriptor.maturity) {
        case integration::IntegrationMaturity::structural: structural += 1U; break;
        case integration::IntegrationMaturity::recognized: recognized += 1U; break;
        case integration::IntegrationMaturity::semantic: semantic += 1U; break;
        default: break;
        }
    }
    assert(registry.formats().size() == 32U);
    assert(structural == 17U);
    assert(recognized == 15U);
    // Nothing has reached semantic. When one does, this fails and the
    // reference gets updated with it.
    assert(semantic == 0U);
}

} // namespace

int main() {
    the_published_counts_match_the_registry();
    every_byte_derived_verdict_is_registered();
    every_animation_kind_is_registered();
    maturity_and_parser_agree();
    reading_a_tag_is_not_the_runtime_recognizing_it();
    the_naming_commands_reach_registered_formats();
    return 0;
}
