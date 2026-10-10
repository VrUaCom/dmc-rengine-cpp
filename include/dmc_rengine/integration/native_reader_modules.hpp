#pragma once

#include "dmc_rengine/integration/native_reader_module.hpp"

namespace dmc::rengine::integration::native_reader_modules {

[[nodiscard]] NativeReaderModule dds();
[[nodiscard]] NativeReaderModule ptx();
[[nodiscard]] NativeReaderModule hits();
[[nodiscard]] NativeReaderModule dca();
[[nodiscard]] NativeReaderModule lig2();
[[nodiscard]] NativeReaderModule stage_txt();
[[nodiscard]] NativeReaderModule scm();
[[nodiscard]] NativeReaderModule mod();
[[nodiscard]] NativeReaderModule mot();
[[nodiscard]] NativeReaderModule so_graph();
[[nodiscard]] NativeReaderModule so_volume();
[[nodiscard]] NativeReaderModule so_link();
[[nodiscard]] NativeReaderModule shw();
[[nodiscard]] NativeReaderModule pe();
[[nodiscard]] NativeReaderModule efm();
[[nodiscard]] NativeReaderModule clt();
[[nodiscard]] NativeReaderModule tsc();
[[nodiscard]] NativeReaderModule evt();
[[nodiscard]] NativeReaderModule collision_shapes();
[[nodiscard]] NativeReaderModule motion_script();
[[nodiscard]] NativeReaderModule stage_cfg_pos();
[[nodiscard]] NativeReaderModule stage_cfg_eve();
[[nodiscard]] NativeReaderModule stage_cfg_cam();
[[nodiscard]] NativeReaderModule stage_cfg_itm();
[[nodiscard]] NativeReaderModule stage_cfg_ste();
[[nodiscard]] NativeReaderModule stage_cfg_est();
[[nodiscard]] NativeReaderModule stage_cfg_sef();
[[nodiscard]] NativeReaderModule fx_effect_record();
[[nodiscard]] NativeReaderModule player_param_block();
[[nodiscard]] NativeReaderModule fon_reader();

} // namespace dmc::rengine::integration::native_reader_modules
