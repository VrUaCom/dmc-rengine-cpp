#pragma once

#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3::vanilla {

struct MasterResourceNameObservation final {
    std::uint32_t index{};
    std::uint64_t pointer_va{};
    std::uint64_t string_file_offset{};
    std::string logical_name;
};

struct MasterResourceCatalogReadResult final {
    std::vector<MasterResourceNameObservation> names;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
};

struct HdAudioDescriptorObservation final {
    std::uint32_t index{};
    std::uint64_t filename_pointer_va{};
    std::uint64_t filename_file_offset{};
    std::string filename;
    std::uint32_t loop_start_ms{};
    std::uint32_t loop_end_ms{};

    [[nodiscard]] bool has_explicit_loop() const noexcept;
};

struct HdAudioDescriptorTableReadResult final {
    std::vector<HdAudioDescriptorObservation> descriptors;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
};

class VanillaStaticTableReader final {
public:
    // Read the recovered 4,039-pointer executable-owned resource-name catalog.
    // The strings remain in the caller-supplied executable; no game bytes are
    // embedded in DMC Rengine.
    [[nodiscard]] static MasterResourceCatalogReadResult
    read_master_resource_catalog(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::size_t max_name_bytes = 260U);

    // Read the recovered 154 x 0x10 HD OGG descriptor table:
    // filename pointer + loopStartMs + loopEndMs.
    [[nodiscard]] static HdAudioDescriptorTableReadResult
    read_hd_audio_descriptors(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::size_t max_name_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3::vanilla
