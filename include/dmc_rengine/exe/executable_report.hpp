#pragma once

#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/exe/rtti_scanner.hpp"

#include <cstdint>
#include <string>

namespace dmc::rengine::exe {

struct ExecutableReportOptions final {
    /// The full RUNTIME_FUNCTION table is large. A size histogram is always
    /// emitted; the individual ranges are opt-in because a committed evidence
    /// record rarely needs a hundred thousand lines that can be regenerated.
    bool include_function_ranges{false};
    bool include_import_functions{true};
    bool include_rtti_classes{true};
    bool include_rtti_hierarchy{true};
};

/// Identity of the analysed artifact.
///
/// The report records a hash and a size, never the bytes: the artifact stays
/// with whoever supplied it.
struct ExecutableArtifactIdentity final {
    std::string sha256;
    std::uint64_t size{};
};

/// Deterministic JSON analysis report for one executable.
///
/// Two runs over identical bytes produce byte-identical output, which is what
/// makes the report usable as an evidence record rather than a printout.
[[nodiscard]] std::string to_json(const ExecutableArtifactIdentity& artifact, const PeImage& image,
                                  const PeDirectories& directories, const RttiScanResult& rtti,
                                  const ExecutableReportOptions& options = {});

} // namespace dmc::rengine::exe
