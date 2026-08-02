#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

struct PeReadResult final {
    std::optional<PeImage> image;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    [[nodiscard]] bool ok() const noexcept {
        return image.has_value() && errors.empty();
    }
};

class PeReader final {
public:
    [[nodiscard]] static PeReadResult read(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::exe
