#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <sstream>

namespace dmc::rengine::gdspaces {

bool ResourceId::valid() const noexcept {
    return !source_id.empty() && !logical_path.empty();
}

std::string ResourceId::canonical() const {
    std::ostringstream out;
    out << source_id << ':' << logical_path;
    if (!container_chain.empty()) {
        out << '#' << container_chain;
    }
    out << "@" << offset << "+" << size;
    return out.str();
}

} // namespace dmc::rengine::gdspaces
