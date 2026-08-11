#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <cassert>
#include <cstdint>
#include <string>

int main() {
    using dmc::rengine::gdspaces::ResourcePathNormalizationFlag;
    using dmc::rengine::gdspaces::ResourcePathNormalizer;
    using dmc::rengine::gdspaces::flag_value;
    using dmc::rengine::profiles::dmc3::ResourcePathPolicy;

    assert(ResourcePathPolicy::physical_flags == 0x0CU);
    assert(ResourcePathPolicy::archive_flags == 0x0EU);

    assert(ResourcePathNormalizer::normalize("a/b//c\\\\d", 0U) ==
        "a\\b\\c\\d");

    const auto strip_flags =
        flag_value(ResourcePathNormalizationFlag::strip_leading_separators) |
        flag_value(ResourcePathNormalizationFlag::strip_trailing_separators);
    assert(ResourcePathNormalizer::normalize("///Foo//BAR\\baz///", strip_flags) ==
        "Foo\\BAR\\baz");
    assert(ResourcePathNormalizer::normalize("////", strip_flags).empty());

    const auto uppercase_flags = strip_flags |
        flag_value(ResourcePathNormalizationFlag::uppercase_ascii);
    assert(ResourcePathNormalizer::normalize("/Abc/xyz/", uppercase_flags) ==
        "ABC\\XYZ");

    const auto lowercase_flags = strip_flags |
        flag_value(ResourcePathNormalizationFlag::lowercase_ascii);
    assert(ResourcePathNormalizer::normalize("/AbC/XYZ/", lowercase_flags) ==
        "abc\\xyz");

    // The executable checks uppercase bit first; if both case bits are set,
    // uppercase wins and the lowercase branch is skipped.
    const auto both_case_flags = lowercase_flags |
        flag_value(ResourcePathNormalizationFlag::uppercase_ascii);
    assert(ResourcePathNormalizer::normalize("/AbC/xyz/", both_case_flags) ==
        "ABC\\XYZ");

    assert(ResourcePathPolicy::physical("//GData.AFS///Room/ST001.PAC//") ==
        "GData.AFS\\Room\\ST001.PAC");
    assert(ResourcePathPolicy::archive("//GData.AFS///Room/ST001.PAC//") ==
        "gdata.afs\\room\\st001.pac");
    assert(ResourcePathPolicy::archive("\\\\GData.AFS\\Room\\st001.pac\\") ==
        "gdata.afs\\room\\st001.pac");

    assert(ResourcePathPolicy::archive("Already\\normalized.pac") ==
        "already\\normalized.pac");
    assert(ResourcePathPolicy::physical("Already\\normalized.pac") ==
        "Already\\normalized.pac");

    return 0;
}
