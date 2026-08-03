#include "dmc_rengine/exe/evidence_address_resolver.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::exe {
namespace {

void add_diagnostic(
    EvidenceAddressResolution& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(EvidenceAddressDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

[[nodiscard]] std::string normalized_hash(std::string_view hash) {
    std::string result(hash);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

[[nodiscard]] bool merge_rva(
    EvidenceAddressResolution& result,
    std::optional<std::uint64_t>& resolved,
    std::uint64_t candidate,
    std::string_view source) {
    if (candidate > std::numeric_limits<std::uint32_t>::max()) {
        add_diagnostic(
            result,
            "evidence-address.rva-out-of-range",
            std::string(source) + " produces an RVA outside the PE32/PE32+ 32-bit RVA range.");
        return false;
    }
    if (resolved.has_value() && *resolved != candidate) {
        add_diagnostic(
            result,
            "evidence-address.address-conflict",
            std::string(source) + " conflicts with another file-offset/RVA/VA representation.");
        return false;
    }
    resolved = candidate;
    return true;
}

} // namespace

EvidenceAddressResolution resolve_evidence_location(
    const evidence::EvidenceLocation& location,
    const evidence::ArtifactIdentity& artifact,
    const integration::ExecutableResourceContext& executable,
    std::uint64_t file_size) {
    EvidenceAddressResolution result;

    if (!location.valid() || !artifact.valid() || !executable.valid()) {
        add_diagnostic(
            result,
            "evidence-address.invalid-input",
            "Evidence location, artifact identity, or executable context is invalid.");
        return result;
    }
    if (location.artifact_id != artifact.id) {
        add_diagnostic(
            result,
            "evidence-address.artifact-id-mismatch",
            "The EvidenceLocation references a different artifact identity.");
    }
    if (normalized_hash(artifact.sha256) !=
        normalized_hash(executable.sha256)) {
        add_diagnostic(
            result,
            "evidence-address.sha256-mismatch",
            "The executable bytes do not match the Evidence Packet artifact SHA-256.");
    }
    if (artifact.size != file_size) {
        add_diagnostic(
            result,
            "evidence-address.artifact-size-mismatch",
            "The executable resource size does not match the Evidence Packet artifact size.");
    }
    if (!location.size.has_value() || *location.size == 0U) {
        add_diagnostic(
            result,
            "evidence-address.size-missing",
            "A non-zero evidence range size is required for patch-grade address resolution.");
    }
    if (!result.diagnostics.empty()) {
        return result;
    }

    std::optional<std::uint64_t> resolved_rva;
    if (location.rva.has_value()) {
        static_cast<void>(merge_rva(
            result, resolved_rva, *location.rva, "The explicit RVA"));
    }
    if (location.va.has_value()) {
        if (*location.va < executable.image.image_base) {
            add_diagnostic(
                result,
                "evidence-address.va-below-image-base",
                "The explicit VA is below the parsed PE image base.");
        } else {
            static_cast<void>(merge_rva(
                result,
                resolved_rva,
                *location.va - executable.image.image_base,
                "The explicit VA"));
        }
    }
    if (location.file_offset.has_value()) {
        if (*location.file_offset >
            std::numeric_limits<std::uint32_t>::max()) {
            add_diagnostic(
                result,
                "evidence-address.file-offset-out-of-range",
                "The explicit file offset is outside the PE32/PE32+ 32-bit file-offset range.");
        } else {
            const auto offset_rva = executable.image.file_offset_to_rva(
                static_cast<std::uint32_t>(*location.file_offset));
            if (!offset_rva.has_value()) {
                add_diagnostic(
                    result,
                    "evidence-address.file-offset-unmapped",
                    "The explicit file offset is not mapped by the parsed PE headers or sections.");
            } else {
                static_cast<void>(merge_rva(
                    result,
                    resolved_rva,
                    *offset_rva,
                    "The explicit file offset"));
            }
        }
    }

    if (!resolved_rva.has_value()) {
        add_diagnostic(
            result,
            "evidence-address.address-missing",
            "At least one file offset, RVA, or VA is required.");
        return result;
    }
    if (!result.diagnostics.empty()) {
        return result;
    }

    const auto resolved_rva32 = static_cast<std::uint32_t>(*resolved_rva);
    const auto file_offset = executable.image.rva_to_file_offset(resolved_rva32);
    const auto va = executable.image.rva_to_va(resolved_rva32);
    if (!file_offset.has_value()) {
        add_diagnostic(
            result,
            "evidence-address.rva-unmapped",
            "The resolved RVA is not mapped to file bytes by the parsed PE image.");
        return result;
    }
    if (!va.has_value()) {
        add_diagnostic(
            result,
            "evidence-address.va-overflow",
            "The resolved RVA cannot be added safely to the image base.");
        return result;
    }
    if (location.file_offset.has_value() &&
        *location.file_offset != *file_offset) {
        add_diagnostic(
            result,
            "evidence-address.file-offset-conflict",
            "The explicit file offset disagrees with the section-resolved offset.");
        return result;
    }
    if (location.va.has_value() && *location.va != *va) {
        add_diagnostic(
            result,
            "evidence-address.va-conflict",
            "The explicit VA disagrees with ImageBase + resolved RVA.");
        return result;
    }
    if (*file_offset > file_size ||
        *location.size > file_size - *file_offset) {
        add_diagnostic(
            result,
            "evidence-address.range-out-of-file",
            "The resolved evidence range extends beyond the executable file.");
        return result;
    }

    result.location = ResolvedEvidenceLocation{
        .artifact_id = artifact.id,
        .file_offset = *file_offset,
        .rva = *resolved_rva,
        .va = *va,
        .size = *location.size,
        .symbol = location.symbol,
        .note = location.note,
    };
    return result;
}

} // namespace dmc::rengine::exe
