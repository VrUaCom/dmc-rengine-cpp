#pragma once

#include "dmc_rengine/integration/native_reader_module.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

class NativeReaderModuleRegistry final {
public:
    NativeReaderModuleRegistry();

    [[nodiscard]] bool register_module(NativeReaderModule module);
    [[nodiscard]] const NativeReaderModule* find(
        std::string_view parser_id) const noexcept;
    [[nodiscard]] const NativeReaderModule* find_by_format(
        std::string_view format) const noexcept;
    [[nodiscard]] const std::vector<NativeReaderModule>& modules() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<NativeReaderModule> modules_;
};

} // namespace dmc::rengine::integration
