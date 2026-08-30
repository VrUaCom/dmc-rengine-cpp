#include "dmc_rengine/gdspaces/enclosing_container_name_evidence.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool valid_digest(std::string_view digest) noexcept {
    return digest.size() == 64U && std::all_of(
        digest.begin(), digest.end(),
        [](unsigned char character) {
            return std::isxdigit(character) != 0;
        });
}

} // namespace

EnclosingContainerNameEvidence::EnclosingContainerNameEvidence(
    ResourceId authority_resource,
    std::string authority_sha256,
    ResourceId target_resource,
    std::string target_sha256,
    std::string raw_label,
    std::string normalized_name,
    std::uint32_t physical_slot_index,
    std::size_t source_line)
    : authority_resource_(std::move(authority_resource)),
      authority_sha256_(std::move(authority_sha256)),
      target_resource_(std::move(target_resource)),
      target_sha256_(std::move(target_sha256)),
      raw_label_(std::move(raw_label)),
      normalized_name_(std::move(normalized_name)),
      physical_slot_index_(physical_slot_index),
      source_line_(source_line) {}

const ResourceId& EnclosingContainerNameEvidence::authority_resource() const noexcept {
    return authority_resource_;
}

std::string_view EnclosingContainerNameEvidence::authority_sha256() const noexcept {
    return authority_sha256_;
}

const ResourceId& EnclosingContainerNameEvidence::target_resource() const noexcept {
    return target_resource_;
}

std::string_view EnclosingContainerNameEvidence::target_sha256() const noexcept {
    return target_sha256_;
}

std::string_view EnclosingContainerNameEvidence::raw_label() const noexcept {
    return raw_label_;
}

std::string_view EnclosingContainerNameEvidence::normalized_name() const noexcept {
    return normalized_name_;
}

std::uint32_t EnclosingContainerNameEvidence::physical_slot_index() const noexcept {
    return physical_slot_index_;
}

std::size_t EnclosingContainerNameEvidence::source_line() const noexcept {
    return source_line_;
}

bool EnclosingContainerNameEvidence::valid() const noexcept {
    return authority_resource_.valid() && valid_digest(authority_sha256_) &&
        target_resource_.valid() && valid_digest(target_sha256_) &&
        !raw_label_.empty() && !normalized_name_.empty() && source_line_ > 0U;
}

} // namespace dmc::rengine::gdspaces
