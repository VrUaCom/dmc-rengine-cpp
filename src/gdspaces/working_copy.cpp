#include "dmc_rengine/gdspaces/working_copy.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <utility>

namespace dmc::rengine::gdspaces {

WorkingCopy::WorkingCopy(ResourcePayload source)
    : resource_(std::move(source.resource)),
      original_(std::move(source.bytes)),
      current_(original_),
      source_sha256_(core::Sha256::compute(
          std::span<const std::byte>{original_}).hex()) {}

const ResourceRef& WorkingCopy::resource() const noexcept {
    return resource_;
}

const std::string& WorkingCopy::source_sha256() const noexcept {
    return source_sha256_;
}

std::uint64_t WorkingCopy::revision() const noexcept {
    return revision_;
}

const std::vector<std::byte>& WorkingCopy::bytes() const noexcept {
    return current_;
}

const std::vector<EditOperation>& WorkingCopy::history() const noexcept {
    return public_history_;
}

bool WorkingCopy::dirty() const noexcept {
    return current_ != original_;
}

EditResult WorkingCopy::apply(EditOperation operation) {
    EditResult result{.applied = false, .revision = revision_, .error = {}};

    if (!operation.valid()) {
        result.error = "Edit operation is invalid.";
        return result;
    }
    if (operation.base_revision != revision_) {
        result.error = "Edit operation was created for a different revision.";
        return result;
    }
    if (std::any_of(
            public_history_.begin(), public_history_.end(),
            [&operation](const EditOperation& existing) {
                return existing.id == operation.id;
            })) {
        result.error = "Edit operation ID is already present in the history.";
        return result;
    }
    if (operation.offset > static_cast<std::uint64_t>(current_.size())) {
        result.error = "Edit offset is outside the working copy.";
        return result;
    }

    const auto offset = static_cast<std::size_t>(operation.offset);
    if (operation.expected.size() > current_.size() - offset) {
        result.error = "Expected byte range is outside the working copy.";
        return result;
    }
    if (!std::equal(
            operation.expected.begin(), operation.expected.end(),
            current_.begin() + static_cast<std::ptrdiff_t>(offset))) {
        result.error = "Expected bytes do not match the current revision.";
        return result;
    }

    const auto retained_size = current_.size() - operation.expected.size();
    if (operation.replacement.size() > current_.max_size() - retained_size) {
        result.error = "Edit would exceed the maximum working-copy size.";
        return result;
    }

    AppliedOperation applied{
        .operation = operation,
        .previous = operation.expected,
    };

    const auto erase_begin = current_.begin() + static_cast<std::ptrdiff_t>(offset);
    const auto erase_end = erase_begin +
        static_cast<std::ptrdiff_t>(operation.expected.size());
    const auto insertion = current_.erase(erase_begin, erase_end);
    current_.insert(
        insertion,
        operation.replacement.begin(),
        operation.replacement.end());

    applied_.push_back(std::move(applied));
    public_history_.push_back(std::move(operation));
    ++revision_;

    result.applied = true;
    result.revision = revision_;
    return result;
}

bool WorkingCopy::undo_last() {
    if (applied_.empty()) {
        return false;
    }

    const auto& last = applied_.back();
    const auto offset = static_cast<std::size_t>(last.operation.offset);
    if (offset > current_.size() ||
        last.operation.replacement.size() > current_.size() - offset) {
        return false;
    }

    const auto begin = current_.begin() + static_cast<std::ptrdiff_t>(offset);
    if (!std::equal(
            last.operation.replacement.begin(),
            last.operation.replacement.end(),
            begin)) {
        return false;
    }

    const auto end = begin +
        static_cast<std::ptrdiff_t>(last.operation.replacement.size());
    const auto insertion = current_.erase(begin, end);
    current_.insert(insertion, last.previous.begin(), last.previous.end());

    applied_.pop_back();
    public_history_.pop_back();
    ++revision_;
    return true;
}

void WorkingCopy::reset() {
    current_ = original_;
    applied_.clear();
    public_history_.clear();
    revision_ = 0U;
}

} // namespace dmc::rengine::gdspaces
