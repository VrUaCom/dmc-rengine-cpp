#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <sstream>
#include <string_view>

namespace dmc::rengine::gdspaces {
namespace {

void append_string_field(std::ostringstream& output, std::string_view value) {
    output << value.size() << ':' << value << '|';
}

} // namespace

bool ResourceId::valid() const noexcept {
    return !source_id.empty() && !logical_path.empty();
}

std::string ResourceId::canonical() const {
    // `canonical()` is a machine identity key, not a presentation path. The
    // legacy delimiter-only form was not injective because source/path/chain
    // may themselves contain ':', '#', '@' and '+'. Bind every arbitrary
    // string by byte length and keep a version tag so the encoding boundary is
    // explicit in manifests, graph keys and provenance authority IDs.
    std::ostringstream out;
    out << "rid2|";
    append_string_field(out, source_id);
    append_string_field(out, logical_path);
    append_string_field(out, container_chain);
    out << offset << '|' << size;
    return out.str();
}

} // namespace dmc::rengine::gdspaces
