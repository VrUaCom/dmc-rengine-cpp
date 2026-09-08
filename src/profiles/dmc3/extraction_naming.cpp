#include "dmc_rengine/profiles/dmc3/extraction_naming.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void append_padded(
    std::ostringstream& out, std::uint32_t value, std::size_t digits) {
    out << std::setfill('0') << std::setw(static_cast<int>(digits)) << value;
}

/** The record name as a filename fragment: `V 108` becomes `V108`. */
[[nodiscard]] std::string compact(std::string_view name) {
    std::string out;
    out.reserve(name.size());
    for (const auto character : name) {
        if (character != ' ' && character != '\t') {
            out.push_back(character);
        }
    }
    return out;
}

} // namespace

std::string Dmc3ExtractionNaming::leaf(const ExtractionNameRequest& request) {
    std::ostringstream out;
    out << request.stem;
    for (const auto slot : request.parent_slots) {
        out << '_' << k_nesting_marker;
        append_padded(out, slot, k_parent_slot_digits);
    }
    out << '_';
    append_padded(out, request.index, k_index_digits);
    if (request.record_name.has_value()) {
        const auto tag = compact(*request.record_name);
        if (!tag.empty()) {
            out << '_' << tag;
        }
    }
    if (!request.extension.empty()) {
        out << '.' << request.extension;
    }
    return out.str();
}

std::string Dmc3ExtractionNaming::directory(
    std::string_view stem,
    const std::vector<std::uint32_t>& parent_slots,
    const std::vector<std::string>& parent_extensions) {
    std::ostringstream out;
    out << stem;
    // Each level is named by the leaf its own payload would have carried, so
    // the folder and the resource it expands read as the same thing.
    for (std::size_t level = 0U; level < parent_slots.size(); ++level) {
        ExtractionNameRequest step;
        step.stem = std::string{stem};
        step.parent_slots.assign(
            parent_slots.begin(), parent_slots.begin() + static_cast<long>(level));
        step.index = parent_slots[level];
        step.extension = level < parent_extensions.size()
            ? parent_extensions[level]
            : std::string{};
        out << '/' << leaf(step);
    }
    return out.str();
}

} // namespace dmc::rengine::profiles::dmc3
