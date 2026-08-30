#include "dmc_rengine/gdspaces/resource_semantic_evidence.hpp"

#include <algorithm>
#include <cctype>
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

ResourceSemanticEvidence::ResourceSemanticEvidence(
    ResourceSemanticEvidenceKind kind,
    ResourceId authority_resource,
    std::string authority_sha256,
    std::string semantic_format,
    std::string canonical_extension,
    std::uint32_t physical_slot_index)
    : kind_(kind),
      authority_resource_(std::move(authority_resource)),
      authority_sha256_(std::move(authority_sha256)),
      semantic_format_(std::move(semantic_format)),
      canonical_extension_(std::move(canonical_extension)),
      physical_slot_index_(physical_slot_index) {}

ResourceSemanticEvidenceKind ResourceSemanticEvidence::kind() const noexcept {
    return kind_;
}

const ResourceId& ResourceSemanticEvidence::authority_resource() const noexcept {
    return authority_resource_;
}

std::string_view ResourceSemanticEvidence::authority_sha256() const noexcept {
    return authority_sha256_;
}

std::string_view ResourceSemanticEvidence::semantic_format() const noexcept {
    return semantic_format_;
}

std::string_view ResourceSemanticEvidence::canonical_extension() const noexcept {
    return canonical_extension_;
}

std::uint32_t ResourceSemanticEvidence::physical_slot_index() const noexcept {
    return physical_slot_index_;
}

bool ResourceSemanticEvidence::valid() const noexcept {
    return authority_resource_.valid() && valid_digest(authority_sha256_) &&
        !semantic_format_.empty() && !canonical_extension_.empty();
}

} // namespace dmc::rengine::gdspaces
