#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace dmc::rengine::evidence {

struct EvidenceLocation final {
    std::string artifact_id;
    std::optional<std::uint64_t> file_offset;
    std::optional<std::uint64_t> size;
    std::optional<std::uint64_t> rva;
    std::optional<std::uint64_t> va;
    std::string symbol;
    std::string note;

    [[nodiscard]] bool valid() const noexcept {
        return !artifact_id.empty();
    }

    friend bool operator==(const EvidenceLocation&, const EvidenceLocation&) = default;
};

} // namespace dmc::rengine::evidence
