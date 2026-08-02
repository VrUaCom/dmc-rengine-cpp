#pragma once

#include "dmc_rengine/binary/document.hpp"

#include <string>

namespace dmc::rengine::binary {

[[nodiscard]] std::string manifest_json(const Document& document);

} // namespace dmc::rengine::binary
