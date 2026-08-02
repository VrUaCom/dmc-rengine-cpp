#include "dmc_rengine/binary/document.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace dmc::rengine::binary {
namespace {

[[nodiscard]] ByteRange intersection(
    const ByteRange& left,
    const ByteRange& right) noexcept {
    const auto begin = std::max(left.offset, right.offset);
    const auto finish = std::min(left.end(), right.end());
    return ByteRange{
        .offset = begin,
        .size = finish > begin ? finish - begin : 0U,
    };
}

} // namespace

bool ByteRange::valid() const noexcept {
    return size != 0U && offset <= std::numeric_limits<std::uint64_t>::max() - size;
}

std::uint64_t ByteRange::end() const noexcept {
    return valid() ? offset + size : offset;
}

bool ByteRange::within(std::uint64_t total_size) const noexcept {
    return valid() && offset <= total_size && size <= total_size - offset;
}

bool ByteRange::overlaps(const ByteRange& other) const noexcept {
    return valid() && other.valid() && offset < other.end() && other.offset < end();
}

bool ByteRange::contains(std::uint64_t position) const noexcept {
    return valid() && position >= offset && position < end();
}

bool ByteRange::contains(const ByteRange& other) const noexcept {
    return valid() && other.valid() && other.offset >= offset && other.end() <= end();
}

bool Region::valid() const noexcept {
    return !id.empty() && !name.empty() && range.valid();
}

bool Field::valid() const noexcept {
    return !id.empty() && !name.empty() && range.valid();
}

bool OwnershipClaim::valid() const noexcept {
    return !owner_id.empty() && !rationale.empty() && range.valid();
}

bool Annotation::valid() const noexcept {
    return !id.empty() && !text.empty() && range.valid();
}

Document::Document(gdspaces::ResourceRef resource, std::uint64_t byte_size)
    : resource_(std::move(resource)), byte_size_(byte_size) {}

const gdspaces::ResourceRef& Document::resource() const noexcept {
    return resource_;
}

std::uint64_t Document::byte_size() const noexcept {
    return byte_size_;
}

bool Document::add_region(Region region) {
    if (!region.valid() || !region.range.within(byte_size_) ||
        find_region(region.id) != nullptr) {
        return false;
    }

    regions_.push_back(std::move(region));
    std::sort(
        regions_.begin(), regions_.end(),
        [](const Region& left, const Region& right) {
            if (left.range.offset != right.range.offset) {
                return left.range.offset < right.range.offset;
            }
            return left.id < right.id;
        });
    return true;
}

bool Document::add_field(Field field) {
    if (!field.valid() || !field.range.within(byte_size_) ||
        find_field(field.id) != nullptr) {
        return false;
    }

    if (!field.parent_id.empty()) {
        const auto* parent = find_field(field.parent_id);
        if (parent == nullptr || !parent->range.contains(field.range)) {
            return false;
        }
    }

    fields_.push_back(std::move(field));
    std::sort(
        fields_.begin(), fields_.end(),
        [](const Field& left, const Field& right) {
            if (left.range.offset != right.range.offset) {
                return left.range.offset < right.range.offset;
            }
            if (left.range.size != right.range.size) {
                return left.range.size > right.range.size;
            }
            return left.id < right.id;
        });
    return true;
}

bool Document::add_ownership(OwnershipClaim claim) {
    if (!claim.valid() || !claim.range.within(byte_size_)) {
        return false;
    }

    const auto duplicate = std::find_if(
        ownership_.begin(), ownership_.end(),
        [&claim](const OwnershipClaim& existing) {
            return existing.owner_id == claim.owner_id &&
                existing.range == claim.range &&
                existing.rationale == claim.rationale;
        });
    if (duplicate != ownership_.end()) {
        return false;
    }

    ownership_.push_back(std::move(claim));
    std::sort(
        ownership_.begin(), ownership_.end(),
        [](const OwnershipClaim& left, const OwnershipClaim& right) {
            if (left.range.offset != right.range.offset) {
                return left.range.offset < right.range.offset;
            }
            return left.owner_id < right.owner_id;
        });
    return true;
}

bool Document::add_annotation(Annotation annotation) {
    if (!annotation.valid() || !annotation.range.within(byte_size_) ||
        find_annotation(annotation.id) != nullptr) {
        return false;
    }

    annotation.tags.erase(
        std::remove_if(
            annotation.tags.begin(), annotation.tags.end(),
            [](const std::string& tag) {
                return tag.empty();
            }),
        annotation.tags.end());
    std::sort(annotation.tags.begin(), annotation.tags.end());
    annotation.tags.erase(
        std::unique(annotation.tags.begin(), annotation.tags.end()),
        annotation.tags.end());

    annotations_.push_back(std::move(annotation));
    std::sort(
        annotations_.begin(), annotations_.end(),
        [](const Annotation& left, const Annotation& right) {
            if (left.range.offset != right.range.offset) {
                return left.range.offset < right.range.offset;
            }
            return left.id < right.id;
        });
    return true;
}

const Region* Document::find_region(std::string_view id) const noexcept {
    const auto iterator = std::find_if(
        regions_.begin(), regions_.end(),
        [id](const Region& region) {
            return region.id == id;
        });
    return iterator == regions_.end() ? nullptr : &*iterator;
}

const Field* Document::find_field(std::string_view id) const noexcept {
    const auto iterator = std::find_if(
        fields_.begin(), fields_.end(),
        [id](const Field& field) {
            return field.id == id;
        });
    return iterator == fields_.end() ? nullptr : &*iterator;
}

const Annotation* Document::find_annotation(
    std::string_view id) const noexcept {
    const auto iterator = std::find_if(
        annotations_.begin(), annotations_.end(),
        [id](const Annotation& annotation) {
            return annotation.id == id;
        });
    return iterator == annotations_.end() ? nullptr : &*iterator;
}

const std::vector<Region>& Document::regions() const noexcept {
    return regions_;
}

const std::vector<Field>& Document::fields() const noexcept {
    return fields_;
}

const std::vector<OwnershipClaim>& Document::ownership() const noexcept {
    return ownership_;
}

const std::vector<Annotation>& Document::annotations() const noexcept {
    return annotations_;
}

std::vector<const Field*> Document::child_fields(
    std::string_view parent_id) const {
    std::vector<const Field*> result;
    for (const auto& field : fields_) {
        if (field.parent_id == parent_id) {
            result.push_back(&field);
        }
    }
    return result;
}

std::vector<const Field*> Document::fields_at(
    std::uint64_t offset) const {
    std::vector<const Field*> result;
    for (const auto& field : fields_) {
        if (field.range.contains(offset)) {
            result.push_back(&field);
        }
    }
    return result;
}

std::vector<const OwnershipClaim*> Document::owners_at(
    std::uint64_t offset) const {
    std::vector<const OwnershipClaim*> result;
    for (const auto& claim : ownership_) {
        if (claim.range.contains(offset)) {
            result.push_back(&claim);
        }
    }
    return result;
}

std::vector<const Annotation*> Document::annotations_at(
    std::uint64_t offset) const {
    std::vector<const Annotation*> result;
    for (const auto& annotation : annotations_) {
        if (annotation.range.contains(offset)) {
            result.push_back(&annotation);
        }
    }
    return result;
}

SelectionContext Document::selection_at(std::uint64_t offset) const {
    SelectionContext context;
    for (const auto& region : regions_) {
        if (region.range.contains(offset)) {
            context.regions.push_back(&region);
        }
    }
    context.fields = fields_at(offset);
    context.owners = owners_at(offset);
    context.annotations = annotations_at(offset);
    return context;
}

std::uint64_t Document::coverage_bytes() const {
    if (regions_.empty()) {
        return 0U;
    }

    std::uint64_t covered = 0U;
    auto current_start = regions_.front().range.offset;
    auto current_end = regions_.front().range.end();

    for (std::size_t index = 1U; index < regions_.size(); ++index) {
        const auto& range = regions_[index].range;
        if (range.offset <= current_end) {
            current_end = std::max(current_end, range.end());
            continue;
        }

        covered += current_end - current_start;
        current_start = range.offset;
        current_end = range.end();
    }

    covered += current_end - current_start;
    return covered;
}

std::vector<ByteRange> Document::unknown_ranges() const {
    std::vector<ByteRange> result;
    std::uint64_t cursor = 0U;

    for (const auto& region : regions_) {
        const auto& range = region.range;
        if (range.offset > cursor) {
            result.push_back(ByteRange{
                .offset = cursor,
                .size = range.offset - cursor,
            });
        }
        cursor = std::max(cursor, range.end());
    }

    if (cursor < byte_size_) {
        result.push_back(ByteRange{
            .offset = cursor,
            .size = byte_size_ - cursor,
        });
    }

    return result;
}

std::vector<RegionConflict> Document::conflicts() const {
    std::vector<RegionConflict> result;

    for (std::size_t left = 0U; left < regions_.size(); ++left) {
        for (std::size_t right = left + 1U; right < regions_.size(); ++right) {
            const auto& left_region = regions_[left];
            const auto& right_region = regions_[right];
            if (!left_region.range.overlaps(right_region.range)) {
                if (right_region.range.offset >= left_region.range.end()) {
                    break;
                }
                continue;
            }

            result.push_back(RegionConflict{
                .left_region_id = left_region.id,
                .right_region_id = right_region.id,
                .overlap = intersection(left_region.range, right_region.range),
            });
        }
    }

    return result;
}

std::vector<OwnershipConflict> Document::ownership_conflicts() const {
    std::vector<OwnershipConflict> result;

    for (std::size_t left = 0U; left < ownership_.size(); ++left) {
        for (std::size_t right = left + 1U; right < ownership_.size(); ++right) {
            const auto& left_claim = ownership_[left];
            const auto& right_claim = ownership_[right];
            if (left_claim.owner_id == right_claim.owner_id ||
                !left_claim.range.overlaps(right_claim.range)) {
                continue;
            }

            result.push_back(OwnershipConflict{
                .left_owner_id = left_claim.owner_id,
                .right_owner_id = right_claim.owner_id,
                .overlap = intersection(left_claim.range, right_claim.range),
            });
        }
    }

    return result;
}

} // namespace dmc::rengine::binary
