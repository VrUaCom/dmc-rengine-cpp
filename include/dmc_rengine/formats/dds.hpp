#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace dmc::rengine::formats::dds {

inline constexpr std::size_t header_size =
    profiles::dmc3::Dmc3DdsProfile::k_header_size;

struct ScanResult final {
    bool recognized{false};
    profiles::dmc3::Dmc3DdsParseResult profile;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Reader final {
public:
    // Native structural DDS reader for the corpus-confirmed DMC3 HD profile.
    // Identity is generic DDS magic; structural acceptance remains constrained
    // to the evidence-backed DXT1/DXT5 DMC3 profile implemented by
    // Dmc3DdsProfile.
    [[nodiscard]] static ScanResult scan(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::dds
