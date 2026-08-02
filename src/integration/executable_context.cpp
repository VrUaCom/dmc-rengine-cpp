#include "dmc_rengine/integration/executable_context.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::integration {

bool ExecutableResourceContext::valid() const noexcept {
    if (sha256.size() != 64U || image.machine == exe::PeMachine::unknown ||
        image.section_count != image.sections.size()) {
        return false;
    }
    if (!std::all_of(
            sha256.begin(), sha256.end(),
            [](unsigned char character) {
                return std::isxdigit(character) != 0;
            })) {
        return false;
    }
    if (known_target_hash_match &&
        (known_target_id.empty() || known_target_name.empty())) {
        return false;
    }
    if (known_target_metadata_match && !known_target_hash_match) {
        return false;
    }
    return true;
}

} // namespace dmc::rengine::integration
