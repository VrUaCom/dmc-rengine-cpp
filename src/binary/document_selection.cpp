#include "dmc_rengine/binary/document.hpp"

namespace dmc::rengine::binary {

std::vector<const Region*> Document::regions_at(
    std::uint64_t offset) const {
    std::vector<const Region*> result;
    for (const auto& region : regions_) {
        if (region.range.contains(offset)) {
            result.push_back(&region);
        }
    }
    return result;
}

std::vector<const Region*> Document::regions_overlapping(
    ByteRange range) const {
    std::vector<const Region*> result;
    if (!range.within(byte_size_)) {
        return result;
    }

    for (const auto& region : regions_) {
        if (region.range.overlaps(range)) {
            result.push_back(&region);
        }
    }
    return result;
}

std::vector<const Field*> Document::fields_overlapping(
    ByteRange range) const {
    std::vector<const Field*> result;
    if (!range.within(byte_size_)) {
        return result;
    }

    for (const auto& field : fields_) {
        if (field.range.overlaps(range)) {
            result.push_back(&field);
        }
    }
    return result;
}

std::vector<const OwnershipClaim*> Document::owners_overlapping(
    ByteRange range) const {
    std::vector<const OwnershipClaim*> result;
    if (!range.within(byte_size_)) {
        return result;
    }

    for (const auto& claim : ownership_) {
        if (claim.range.overlaps(range)) {
            result.push_back(&claim);
        }
    }
    return result;
}

std::vector<const Annotation*> Document::annotations_overlapping(
    ByteRange range) const {
    std::vector<const Annotation*> result;
    if (!range.within(byte_size_)) {
        return result;
    }

    for (const auto& annotation : annotations_) {
        if (annotation.range.overlaps(range)) {
            result.push_back(&annotation);
        }
    }
    return result;
}

SelectionContext Document::selection_for_range(ByteRange range) const {
    SelectionContext context;
    if (!range.within(byte_size_)) {
        return context;
    }

    context.regions = regions_overlapping(range);
    context.fields = fields_overlapping(range);
    context.owners = owners_overlapping(range);
    context.annotations = annotations_overlapping(range);
    return context;
}

} // namespace dmc::rengine::binary
