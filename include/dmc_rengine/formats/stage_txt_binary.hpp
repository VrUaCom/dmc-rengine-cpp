#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/formats/stage_txt.hpp"

#include <cstddef>
#include <optional>
#include <span>

namespace dmc::rengine::formats::stage_txt {

[[nodiscard]] std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const LexResult& lex);

} // namespace dmc::rengine::formats::stage_txt
