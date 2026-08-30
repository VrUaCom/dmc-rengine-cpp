#include "dmc_rengine/gdspaces/format_identity.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::gdspaces {

std::string ResourceFormatIdentity::canonical_extension(
    std::string_view semantic_format) {
    std::string format{semantic_format};
    std::transform(
        format.begin(), format.end(), format.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

    if (format == "pe") {
        return "exe";
    }
    if (format == "name-list") {
        return "index";
    }
    return format;
}

} // namespace dmc::rengine::gdspaces
