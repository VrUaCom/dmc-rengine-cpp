#include "dmc_rengine/profiles/dmc3/mod_authored_child_bridge.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/mod.hpp"

#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] ModAuthoredChildResult failure(
    ModAuthoredChildStatus status,
    std::string detail) {
    return ModAuthoredChildResult{
        .status = status,
        .image = std::nullopt,
        .detail = std::move(detail),
    };
}

} // namespace

ModAuthoredChildResult ModAuthoredChildBridge::build(
    const gdspaces::ResourcePayload& source,
    const formats::mod::WriteResult& written,
    const std::uint64_t revision) {
    if (!source.readable() || source.bytes.empty() ||
        source.resource.id.size !=
            static_cast<std::uint64_t>(source.bytes.size()) ||
        source.resource.container) {
        return failure(
            ModAuthoredChildStatus::invalid_source,
            "MOD authored-child bridging requires a readable, non-container source whose ResourceId size matches its bytes.");
    }

    const auto source_span = std::span<const std::byte>{
        source.bytes.data(), source.bytes.size()};
    const auto source_parse = formats::mod::Parser::parse(source_span);
    if (!source_parse.ok()) {
        return failure(
            ModAuthoredChildStatus::source_reparse_failed,
            "The supplied child source is not a canonical MOD image.");
    }

    if (!written.ok()) {
        return failure(
            ModAuthoredChildStatus::writer_failed,
            "The supplied MOD write result did not pass its canonical writer/reopen gate.");
    }
    if (!written.receipt.valid() ||
        !written.receipt.source_image_matches_document ||
        !written.receipt.unauthorized_bytes_unchanged ||
        !written.receipt.output_reparse_ok) {
        return failure(
            ModAuthoredChildStatus::receipt_invalid,
            "MOD writer receipt is incomplete or lacks the preservation/reopen guarantees required for reintegration.");
    }

    if (written.bytes.size() != source.bytes.size() ||
        written.receipt.byte_count !=
            static_cast<std::uint64_t>(source.bytes.size())) {
        return failure(
            ModAuthoredChildStatus::size_changed,
            "PAC/PNST reintegration currently requires a same-size MOD child image.");
    }

    const auto source_sha = sha256_of(source_span);
    if (source_sha != written.receipt.source_sha256) {
        return failure(
            ModAuthoredChildStatus::source_hash_mismatch,
            "MOD writer receipt source SHA-256 does not bind to the exact child source bytes.");
    }

    const auto output_span = std::span<const std::byte>{
        written.bytes.data(), written.bytes.size()};
    const auto output_sha = sha256_of(output_span);
    if (output_sha != written.receipt.output_sha256) {
        return failure(
            ModAuthoredChildStatus::output_hash_mismatch,
            "MOD writer receipt output SHA-256 does not bind to the supplied authored bytes.");
    }

    const auto output_parse = formats::mod::Parser::parse(output_span);
    if (!output_parse.ok() || !written.reparsed.ok()) {
        return failure(
            ModAuthoredChildStatus::output_reparse_failed,
            "The authored MOD image does not independently reopen through the canonical parser.");
    }

    AuthoredChildImage image{
        .resource = source.resource.id,
        .source_sha256 = source_sha,
        .output_sha256 = output_sha,
        .revision = revision,
        .writer_mode = "mod-preserve-layout-v1",
        .bytes = written.bytes,
    };
    if (!image.valid()) {
        return failure(
            ModAuthoredChildStatus::invalid_authored_image,
            "Validated MOD writer output could not form a valid generic authored-child envelope.");
    }

    return ModAuthoredChildResult{
        .status = ModAuthoredChildStatus::ok,
        .image = std::move(image),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
