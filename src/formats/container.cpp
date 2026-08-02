#include "dmc_rengine/formats/container.hpp"

#include <algorithm>
#include <limits>

namespace dmc::rengine::formats {

bool ContainerEntry::valid(std::uint64_t container_size) const noexcept {
    if (!populated) {
        return offset == 0U && size == 0U;
    }

    if (size == 0U || offset > container_size || size > container_size - offset) {
        return false;
    }

    return !logical_name.empty();
}

bool ContainerDocument::valid() const noexcept {
    if (format.empty() || schema_version == 0U ||
        entries.size() != static_cast<std::size_t>(declared_slot_count)) {
        return false;
    }

    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (entries[index].slot_index != index ||
            !entries[index].valid(container_size)) {
            return false;
        }
    }

    return true;
}

bool ContainerParseResult::ok() const noexcept {
    if (!recognized || !document.valid()) {
        return false;
    }

    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

} // namespace dmc::rengine::formats
