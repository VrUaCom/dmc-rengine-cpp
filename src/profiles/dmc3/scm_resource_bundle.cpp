#include "dmc_rengine/profiles/dmc3/scm_resource_bundle.hpp"

#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool validate_texture_indices(
    const formats::scm::Document& document,
    std::size_t texture_count) noexcept {
    for (const auto& object : document.objects) {
        for (const auto& mesh : object.meshes) {
            if (static_cast<std::size_t>(mesh.texture_index) >= texture_count) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

ScmResourceBundleWriteResult ScmResourceBundleWriter::write(
    const ScmResourceBundle& bundle,
    formats::scm::WriteMode scm_mode,
    std::span<const AuthoredPackedTextureDds> authored_textures,
    ScmResourceBundleSafety safety) {
    ScmResourceBundleWriteResult out;

    const auto source_framing = TextureSlotFramingParser::parse(
        std::span<const std::byte>{bundle.texture_companion_source},
        safety.framing);
    if (!source_framing.ok()) {
        out.status = ScmResourceBundleStatus::invalid_texture_companion;
        out.detail = std::string{"texture companion framing: "} +
                     std::string{to_string(source_framing.status)};
        return out;
    }

    const auto texture_count = source_framing.document.textures.size();
    if (texture_count > std::numeric_limits<std::uint8_t>::max() ||
        bundle.scm.header.texture_slot_count != texture_count) {
        out.status = ScmResourceBundleStatus::texture_count_mismatch;
        out.detail =
            "SCM +0x12 must exactly mirror the external texture companion count.";
        return out;
    }

    if (!validate_texture_indices(bundle.scm, texture_count)) {
        out.status = ScmResourceBundleStatus::texture_index_out_of_range;
        out.detail =
            "At least one SCM mesh texture_index is outside the external texture table.";
        return out;
    }

    if (authored_textures.empty()) {
        out.texture_companion_bytes = bundle.texture_companion_source;
    } else {
        const auto texture_result = TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{bundle.texture_companion_source},
            authored_textures,
            safety.texture_reflow);
        if (!texture_result.ok()) {
            out.status = ScmResourceBundleStatus::texture_write_failed;
            out.detail = std::string{"texture reflow: "} +
                         std::string{to_string(texture_result.status)};
            return out;
        }
        out.texture_companion_bytes = texture_result.bytes;
        out.texture_receipt = texture_result.receipt;
    }

    const auto output_framing = TextureSlotFramingParser::parse(
        std::span<const std::byte>{out.texture_companion_bytes},
        safety.framing);
    if (!output_framing.ok()) {
        out.status = ScmResourceBundleStatus::output_texture_companion_invalid;
        out.detail = "authored texture companion failed framing reparse.";
        return out;
    }
    if (output_framing.document.textures.size() != texture_count) {
        out.status = ScmResourceBundleStatus::output_texture_count_changed;
        out.detail =
            "Current safe SCM bundle authoring does not add or remove texture slots.";
        return out;
    }

    const auto scm_result = formats::scm::Writer::write(bundle.scm, scm_mode);
    out.scm_diagnostics = scm_result.diagnostics;
    if (!scm_result.ok()) {
        out.status = ScmResourceBundleStatus::scm_write_failed;
        out.detail = "SCM writer failed its canonical reparse gate.";
        return out;
    }

    out.scm_bytes = scm_result.bytes;
    out.status = ScmResourceBundleStatus::ok;
    out.detail = authored_textures.empty()
        ? "SCM and texture companion validated coherently; texture bytes preserved."
        : "SCM and texture companion authored coherently through canonical writers.";
    return out;
}

} // namespace dmc::rengine::profiles::dmc3
