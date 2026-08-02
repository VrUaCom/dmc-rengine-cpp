#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::exe {

struct KnownExecutableTarget final {
    std::string id;
    std::string display_name;
    std::string sha256;
    PeKind kind{PeKind::pe32};
    PeMachine machine{PeMachine::unknown};
    std::uint64_t image_base{};
    std::uint32_t entry_point_rva{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool matches_hash(std::string_view hash) const noexcept;
    [[nodiscard]] bool matches_metadata(const PeImage& image) const noexcept;
};

} // namespace dmc::rengine::exe
