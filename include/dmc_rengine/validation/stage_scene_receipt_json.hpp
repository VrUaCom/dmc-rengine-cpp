#pragma once

#include "dmc_rengine/validation/stage_scene_receipt.hpp"

#include <string>

namespace dmc::rengine::validation {

[[nodiscard]] std::string stage_scene_validation_receipt_json(
    const StageSceneValidationReceipt& receipt);

} // namespace dmc::rengine::validation
