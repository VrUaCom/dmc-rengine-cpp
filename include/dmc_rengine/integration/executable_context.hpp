#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <string>

namespace dmc::rengine::integration {

struct ExecutableResourceContext final {
    exe::PeImage image;
    std::string sha256;
    std::string known_target_id;
    std::string known_target_name;
    bool known_target_hash_match{false};
    bool known_target_metadata_match{false};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool recognized_target() const noexcept {
        return known_target_hash_match && !known_target_id.empty();
    }
};

} // namespace dmc::rengine::integration
