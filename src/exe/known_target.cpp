#include "dmc_rengine/exe/known_target.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] bool is_hex_sha256(std::string_view value) noexcept {
    return value.size() == 64U && std::all_of(
        value.begin(), value.end(),
        [](unsigned char character) {
            return std::isxdigit(character) != 0;
        });
}

} // namespace

bool KnownExecutableTarget::valid() const noexcept {
    return !id.empty() && !display_name.empty() && is_hex_sha256(sha256) &&
        image_base != 0U;
}

bool KnownExecutableTarget::matches_hash(std::string_view hash) const noexcept {
    if (!valid() || !is_hex_sha256(hash)) {
        return false;
    }

    return std::equal(
        sha256.begin(), sha256.end(), hash.begin(),
        [](unsigned char left, unsigned char right) {
            return std::tolower(left) == std::tolower(right);
        });
}

bool KnownExecutableTarget::matches_metadata(const PeImage& image) const noexcept {
    return valid() && image.kind == kind && image.machine == machine &&
        image.image_base == image_base &&
        image.entry_point_rva == entry_point_rva;
}

} // namespace dmc::rengine::exe
