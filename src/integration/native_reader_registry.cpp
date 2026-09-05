#include "dmc_rengine/integration/native_reader_registry.hpp"

#include "dmc_rengine/integration/native_reader_modules.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::integration {

NativeReaderModuleRegistry::NativeReaderModuleRegistry() {
    static_cast<void>(register_module(native_reader_modules::dds()));
    static_cast<void>(register_module(native_reader_modules::ptx()));
    static_cast<void>(register_module(native_reader_modules::hits()));
    static_cast<void>(register_module(native_reader_modules::dca()));
    static_cast<void>(register_module(native_reader_modules::lig2()));
    static_cast<void>(register_module(native_reader_modules::stage_txt()));
    static_cast<void>(register_module(native_reader_modules::scm()));
    static_cast<void>(register_module(native_reader_modules::mod()));
    static_cast<void>(register_module(native_reader_modules::pe()));
}

bool NativeReaderModuleRegistry::register_module(NativeReaderModule module) {
    if (!module.valid() || find(module.parser_id) != nullptr) {
        return false;
    }
    modules_.push_back(std::move(module));
    return true;
}

const NativeReaderModule* NativeReaderModuleRegistry::find(
    std::string_view parser_id) const noexcept {
    const auto iterator = std::find_if(
        modules_.begin(), modules_.end(),
        [parser_id](const NativeReaderModule& module) {
            return module.parser_id == parser_id;
        });
    return iterator == modules_.end() ? nullptr : &*iterator;
}

const std::vector<NativeReaderModule>&
NativeReaderModuleRegistry::modules() const noexcept {
    return modules_;
}

std::size_t NativeReaderModuleRegistry::size() const noexcept {
    return modules_.size();
}

} // namespace dmc::rengine::integration
