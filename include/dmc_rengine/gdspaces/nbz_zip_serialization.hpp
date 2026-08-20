#pragma once

#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

struct NbzZipSerializationLimits final {
    std::uint64_t max_metadata_bytes{64ULL * 1024ULL * 1024ULL};
};

struct NbzZipSerializationEntry final {
    std::uint32_t central_index{};

    std::uint64_t central_record_offset{};
    std::vector<std::byte> central_record_bytes;

    std::uint64_t local_record_offset{};
    std::vector<std::byte> local_prefix_bytes;
    std::uint64_t data_offset{};
    std::uint64_t stored_data_size{};

    // Local-header start through the next distinct local-header start, or the
    // recovered central-directory start for the final local region. This span
    // intentionally keeps descriptor/padding/gap bytes opaque.
    std::uint64_t local_region_size{};
    bool uses_data_descriptor{};

    [[nodiscard]] bool valid(
        std::uint64_t archive_size,
        std::uint64_t central_start) const noexcept;
};

// Read-side serialization preservation authority. This snapshot is not a
// cryptographic artifact identity and not a writer receipt. A future retail
// repacker must additionally bind source spans to the exact source artifact.
struct NbzZipSerializationSnapshot final {
    std::string source_id;
    std::uint64_t archive_size{};
    std::uint64_t prefix_size{};
    std::uint64_t computed_central_start{};
    std::uint64_t eocd_offset{};
    std::vector<std::byte> eocd_bytes;
    std::vector<NbzZipSerializationEntry> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzZipSerializationScanResult final {
    std::optional<NbzZipSerializationSnapshot> snapshot;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class NbzZipSerializationScanner final {
public:
    [[nodiscard]] static NbzZipSerializationScanResult scan(
        const NbzZipSource& source,
        NbzZipSerializationLimits limits = {});
};

} // namespace dmc::rengine::gdspaces
