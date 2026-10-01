#pragma once

#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"
#include "dmc_rengine/spider/crusader.hpp"
#include "dmc_rengine/spider/family.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Spider Tarantula: texture format change as a native workflow.
//
// Replaces the scratch Python prototype (ptx_to_bc7.py + etcpak) that first
// re-encoded Dante's pl000.pac to BC7. The workflow only coordinates; the
// algorithms stay in their modules (codecs::dds_bcn, codecs::dds_bcn_encode,
// profiles::dmc3::texture_reencode, core::no_replace_publication):
//
//   acquire [io]                 read the source file (or take bytes)
//    -> inspect [cpu]            container kind, texture slots, formats
//    -> transform[i] [cpu]       re-encode one texture payload (PAC slot i)
//    -> assemble [cpu]           rebuild the PAC / take the single payload
//    -> validate [cpu]           every touched texture in the new format,
//                                every untouched PAC slot byte-identical
//    -> publish [io]             no-replace atomic publication (optional)
//
// Each step is one Crusader instruction executed by the native executor; the
// transform steps carry the PAC slot as their operand, like the window steps
// of the EXE window packet workflow.
namespace dmc::rengine::spider::tarantula {

inline constexpr Family family = Family::tarantula;

struct TextureReencodeRequest final {
    profiles::dmc3::TextureReencodeOptions options{};
    // Publish target; empty = run in memory only (result bytes returned).
    std::filesystem::path output;
    // Overwrite an existing output (default: no-replace publication).
    bool replace_existing{};
};

struct TextureReencodeStep final {
    std::string name;  // "acquire", "inspect", "transform[3]", ...
    bool ok{};
    std::string detail;
};

struct TextureReencodeWorkflowResult final {
    bool ok{};
    profiles::dmc3::TextureReencodeResult result;
    std::vector<TextureReencodeStep> steps;
    crusader::ExecutionReport report;
    std::string detail;
};

// File in, file out (or memory when request.output is empty).
[[nodiscard]] TextureReencodeWorkflowResult run_texture_reencode(
    const std::filesystem::path& input, const TextureReencodeRequest& request);

// Bytes in (acquire is a no-op), for embedding in viewers.
[[nodiscard]] TextureReencodeWorkflowResult run_texture_reencode(
    std::span<const std::byte> input, const TextureReencodeRequest& request);

}  // namespace dmc::rengine::spider::tarantula
