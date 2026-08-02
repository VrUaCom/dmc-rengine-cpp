#include "dmc_rengine/evidence/artifact_registry.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace dmc::rengine::evidence {
namespace {

[[nodiscard]] std::string normalized_hash(std::string_view hash) {
    std::string result(hash);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

} // namespace

bool ArtifactRegistry::add(ArtifactIdentity artifact) {
    if (!artifact.valid()) {
        return false;
    }
    artifact.sha256 = normalized_hash(artifact.sha256);
    return artifacts_.emplace(artifact.id, std::move(artifact)).second;
}

bool ArtifactRegistry::replace(ArtifactIdentity artifact) {
    if (!artifact.valid()) {
        return false;
    }
    const auto iterator = artifacts_.find(artifact.id);
    if (iterator == artifacts_.end()) {
        return false;
    }
    artifact.sha256 = normalized_hash(artifact.sha256);
    iterator->second = std::move(artifact);
    return true;
}

const ArtifactIdentity* ArtifactRegistry::find(
    std::string_view id) const noexcept {
    const auto iterator = artifacts_.find(id);
    return iterator == artifacts_.end() ? nullptr : &iterator->second;
}

std::vector<const ArtifactIdentity*> ArtifactRegistry::by_sha256(
    std::string_view sha256) const {
    const auto normalized = normalized_hash(sha256);
    std::vector<const ArtifactIdentity*> result;
    for (const auto& [id, artifact] : artifacts_) {
        static_cast<void>(id);
        if (artifact.sha256 == normalized) {
            result.push_back(&artifact);
        }
    }
    return result;
}

std::vector<const ArtifactIdentity*> ArtifactRegistry::by_role(
    std::string_view role) const {
    std::vector<const ArtifactIdentity*> result;
    for (const auto& [id, artifact] : artifacts_) {
        static_cast<void>(id);
        if (artifact.role == role) {
            result.push_back(&artifact);
        }
    }
    return result;
}

const std::map<std::string, ArtifactIdentity, std::less<>>&
ArtifactRegistry::all() const noexcept {
    return artifacts_;
}

std::size_t ArtifactRegistry::size() const noexcept {
    return artifacts_.size();
}

bool ArtifactRegistry::empty() const noexcept {
    return artifacts_.empty();
}

} // namespace dmc::rengine::evidence
