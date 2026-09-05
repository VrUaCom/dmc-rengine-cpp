#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/formats/ptx.hpp"

#include <cstddef>
#include <optional>
#include <span>

namespace dmc::rengine::formats::ptx {

[[nodiscard]] std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan);

} // namespace dmc::rengine::formats::ptx
