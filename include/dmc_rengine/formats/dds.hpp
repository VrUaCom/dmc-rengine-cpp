#pragma once

#include "dmc_rengine/codecs/dds_bc.hpp"
#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace dmc::rengine::formats::dds {

inline constexpr std::size_t header_size = codecs::dds_bc::header_size;

struct ScanResult final {
    bool recognized{false};

    // Direct read authority for standalone DDS resources. This accepts bounded
    // 2D DXT1/DXT5 DDS files, including valid partial mip chains.
    codecs::dds_bc::ParseResult reader;

    // Separate strict DMC3 authoring/evidence profile. A direct DDS can be
    // readable even when it does not satisfy this full-chain canonical profile.
    profiles::dmc3::Dmc3DdsParseResult profile;

    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Reader final {
public:
    [[nodiscard]] static ScanResult scan(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::dds
