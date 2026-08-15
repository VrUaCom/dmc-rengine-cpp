#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace dmc::rengine::stageops {

// Byte-precise navigation context for a Stage Ops domain object. This is a
// projection of already-retained parser provenance, not a second parser result.
struct StageDomainSourceSpan final {
    std::string resource_id;
    std::uint64_t offset{};
    std::uint64_t size{};
    std::optional<std::uint32_t> line;
    std::optional<std::uint32_t> column;
    integration::ParsedByteSource byte_source{
        integration::ParsedByteSource::immutable_source};
    std::uint64_t byte_revision{};

    [[nodiscard]] bool valid() const noexcept {
        return !resource_id.empty() && size != 0U;
    }
};

// Returns a source span only when the StageDomainObject carries exact structural
// byte coordinates. Aggregate domains such as an entire HITS or TXT summary may
// intentionally return nullopt until their domain contract defines one bounded
// source region.
[[nodiscard]] std::optional<StageDomainSourceSpan> domain_source_span(
    const StageDomainObject& object) noexcept;

} // namespace dmc::rengine::stageops
