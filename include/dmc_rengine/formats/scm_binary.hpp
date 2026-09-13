#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/formats/scm.hpp"

#include <optional>
#include <span>

namespace dmc::rengine::formats::scm {

// Builds an evidence-aware byte map for a successfully parsed SCM resource.
// The map is read-only: it exposes confirmed fields, physical stream ownership
// and preservation-only domains without upgrading undecoded bytes to semantics.
[[nodiscard]] std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ParseResult& parsed);

} // namespace dmc::rengine::formats::scm
