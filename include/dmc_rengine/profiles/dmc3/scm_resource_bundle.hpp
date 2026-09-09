#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class ScmResourceBundleStatus : std::uint8_t {
    ok,
    invalid_texture_companion,
    texture_count_mismatch,
    texture_index_out_of_range,
    scm_write_failed,
    texture_write_failed,
    output_texture_companion_invalid,
    output_texture_count_changed,
};

[[nodiscard]] constexpr std::string_view to_string(
    ScmResourceBundleStatus status) noexcept {
    switch (status) {
    case ScmResourceBundleStatus::ok: return "ok";
    case ScmResourceBundleStatus::invalid_texture_companion:
        return "invalid-texture-companion";
    case ScmResourceBundleStatus::texture_count_mismatch:
        return "texture-count-mismatch";
    case ScmResourceBundleStatus::texture_index_out_of_range:
        return "texture-index-out-of-range";
    case ScmResourceBundleStatus::scm_write_failed:
        return "scm-write-failed";
    case ScmResourceBundleStatus::texture_write_failed:
        return "texture-write-failed";
    case ScmResourceBundleStatus::output_texture_companion_invalid:
        return "output-texture-companion-invalid";
    case ScmResourceBundleStatus::output_texture_count_changed:
        return "output-texture-count-changed";
    }
    return "invalid-texture-companion";
}

struct ScmResourceBundle final {
    formats::scm::Document scm;

    // Physical bytes for the external texture slot/resource loaded alongside
    // SCM by the DMC3-HD profile. This stays profile-side so the generic SCM
    // format module does not depend on DMC3 texture framing.
    std::vector<std::byte> texture_companion_source;
};

struct ScmResourceBundleSafety final {
    TextureSlotFramingSafety framing{};
    TextureSlotPackedReflowSafety texture_reflow{};
};

struct ScmResourceBundleWriteResult final {
    ScmResourceBundleStatus status{
        ScmResourceBundleStatus::invalid_texture_companion};
    std::vector<std::byte> scm_bytes;
    std::vector<std::byte> texture_companion_bytes;
    std::optional<TextureSlotPackedReflowReceipt> texture_receipt;
    std::vector<formats::ParseDiagnostic> scm_diagnostics;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == ScmResourceBundleStatus::ok;
    }
};

class ScmResourceBundleWriter final {
public:
    // Coherent authoring gate for DMC3-HD SCM + external texture companion.
    //
    // The texture-slot framing parser is the count authority. SCM +0x12 must
    // mirror that count exactly and every mesh texture_index must be in range.
    // Optional DDS edits are delegated to the existing evidence-backed texture
    // reflow writer; this coordinator never implements a second texture format.
    [[nodiscard]] static ScmResourceBundleWriteResult write(
        const ScmResourceBundle& bundle,
        formats::scm::WriteMode scm_mode,
        std::span<const AuthoredPackedTextureDds> authored_textures = {},
        ScmResourceBundleSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
