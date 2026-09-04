#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::gdspaces::ResourceClassifier;
using dmc::rengine::integration::FormatIntegrationRegistry;
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
//
// Five of the six family tags also open with a three-byte tag the registry
// probe (evidence site A) recognizes on its own — MOD/EFM/SCM/MRP/SHW all do.
// That probe runs first because it is the more specific site, so those five
// report runtime_content_tag_confirmed rather than magic_confirmed. MCV is
// the one tag the registry probe does not recognize at all, so it is the only
// one that actually falls through to the family-mask branch.
void test_classifier_recognizes_family_mask_tags() {
    for (const auto& entry : ResourceTypeContract::family_tagged_types) {
        const auto payload = bytes_of(
            std::string{entry.tag_with_space} + "payload-body-padding");
        const auto classification = ResourceClassifier::classify(
            "unnamed", std::span<const std::byte>{payload});
        assert(classification.format ==
               ResourceTypeContract::canonical_extension(entry.mask));
        assert(classification.byte_derived);
        if (entry.mask == ResourceTypeContract::FamilyMask::motion_curve) {
            assert(classification.magic_confirmed);
            assert(classification.runtime_family_mask_confirmed);
        } else {
            assert(!classification.magic_confirmed);
            assert(classification.runtime_content_tag_confirmed);
        }
    }
}

// "MODX" is not a family tag — the fourth byte is not a space — but the
// registry probe (evidence site A) never inspects a fourth byte at all, so
// it recognizes the same three bytes regardless. This is the recovered
// behavior, not a gap: `type_for_prefix`'s own doc says exactly this.
void test_registry_probe_ignores_the_fourth_byte() {
    const auto payload = bytes_of("MODXpayload");
    const auto classification = ResourceClassifier::classify(
        "unnamed", std::span<const std::byte>{payload});
    assert(classification.format == "mod");
    assert(classification.runtime_content_tag_confirmed);
    assert(!classification.magic_confirmed);
    assert(!classification.runtime_family_mask_confirmed);
}

// A prefix neither probe recognizes, and no path extension to fall back to,
// must stay unknown rather than guess.
void test_unrecognized_prefix_without_extension_is_not_confirmed() {
    const auto payload = bytes_of("ZZZZpayload");
    const auto classification = ResourceClassifier::classify(
        "unnamed", std::span<const std::byte>{payload});
    assert(classification.format == "unknown");
    assert(!classification.magic_confirmed);
    assert(!classification.runtime_content_tag_confirmed);
    assert(!classification.runtime_family_mask_confirmed);
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
    const auto classification = ResourceClassifier::classify("obj/basic.mod");
    assert(classification.format == "mod");
    assert(!classification.magic_confirmed);
    assert(!classification.container);
}

// A container format named by extension alone, with no bytes to check, is
// reported as a container optimistically: an index built before
// materialization has nothing better to go on, and refusing to offer
// expansion until every member is read would cost more than it explains. See
// ResourceClassifier::classify's note on the `structural` claim.
void test_container_format_by_extension_alone_is_optimistic() {
    const auto classification = ResourceClassifier::classify("tex/basic.ptx");
    assert(classification.format == "ptx");
    assert(!classification.magic_confirmed);
    assert(classification.container);
}

} // namespace

int main() {
    test_every_contract_family_type_is_registered();
    test_every_contract_registry_type_is_registered();
    test_every_contract_extension_type_is_registered();
    test_observed_retail_extensions_are_registered();
    test_classifier_recognizes_family_mask_tags();
    test_registry_probe_ignores_the_fourth_byte();
    test_unrecognized_prefix_without_extension_is_not_confirmed();
    test_zip_magic_is_recognized_as_nbz_container();
    test_magic_outranks_extension();
    test_extension_fallback_still_applies();
    test_container_format_by_extension_alone_is_optimistic();
    return 0;
}
