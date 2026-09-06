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
[[nodiscard]] NativeReaderModule shw();
[[nodiscard]] NativeReaderModule pe();

} // namespace dmc::rengine::integration::native_reader_modules
