#include "dmc_rengine/binary/document.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace dmc::rengine::binary {

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

bool Region::valid() const noexcept {
    return !id.empty() && !name.empty() && range.valid();
}

bool OwnershipClaim::valid() const noexcept {
    return !owner_id.empty() && !rationale.empty() && range.valid();
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

bool Document::add_ownership(OwnershipClaim claim) {
    if (!claim.valid() || !claim.range.within(byte_size_)) {
        return false;
    }

    ownership_.push_back(std::move(claim));
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

const std::vector<Region>& Document::regions() const noexcept {
    return regions_;
}

const std::vector<OwnershipClaim>& Document::ownership() const noexcept {
    return ownership_;
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

            const auto overlap_start = std::max(
                left_region.range.offset,
                right_region.range.offset);
            const auto overlap_end = std::min(
                left_region.range.end(),
                right_region.range.end());
            result.push_back(RegionConflict{
                .left_region_id = left_region.id,
                .right_region_id = right_region.id,
                .overlap = ByteRange{
                    .offset = overlap_start,
                    .size = overlap_end - overlap_start,
                },
            });
        }
    }

    return result;
}

} // namespace dmc::rengine::binary
