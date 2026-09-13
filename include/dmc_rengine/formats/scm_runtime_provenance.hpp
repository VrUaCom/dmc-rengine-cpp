#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/formats/scm.hpp"

#include <optional>
#include <span>

namespace dmc::rengine::formats::scm {

// Adds evidence-backed runtime dataflow to an already materialized SCM byte
// map. This layer deliberately records operational provenance rather than
// inventing high-level names for preservation-only fields.
[[nodiscard]] bool annotate_runtime_provenance(
    binary::Document& document,
    const ParseResult& parsed);

// Product-facing deep document used by Native Reader / inspect-scm. It combines
// the physical Binary Inspector map with canonical DMC3 runtime provenance.
[[nodiscard]] std::optional<binary::Document> build_deep_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ParseResult& parsed);

} // namespace dmc::rengine::formats::scm
