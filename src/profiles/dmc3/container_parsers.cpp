#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include "dmc_rengine/formats/container_parser.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool matches_magic(
    std::span<const std::byte> bytes,
    std::string_view magic) noexcept {
    if (bytes.size() < magic.size()) {
        return false;
    }
    for (std::size_t index = 0; index < magic.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(magic[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string_view pac_error_suffix(
    formats::PacParseError error) noexcept {
    switch (error) {
    case formats::PacParseError::none: return "none";
    case formats::PacParseError::truncated_header: return "truncated_header";
    case formats::PacParseError::invalid_magic: return "invalid_magic";
    case formats::PacParseError::slot_count_limit: return "slot_count_limit";
    case formats::PacParseError::truncated_offset_table:
        return "truncated_offset_table";
    case formats::PacParseError::slot_offset_before_payload:
        return "slot_offset_before_payload";
    case formats::PacParseError::slot_offset_out_of_bounds:
        return "slot_offset_out_of_bounds";
    case formats::PacParseError::invalid_document: return "invalid_document";
    }
    return "invalid_document";
}

[[nodiscard]] std::string_view pnst_error_suffix(
    formats::PnstParseError error) noexcept {
    switch (error) {
    case formats::PnstParseError::none: return "none";
    case formats::PnstParseError::truncated_header: return "truncated_header";
    case formats::PnstParseError::invalid_magic: return "invalid_magic";
    case formats::PnstParseError::slot_count_limit: return "slot_count_limit";
    case formats::PnstParseError::truncated_offset_table:
        return "truncated_offset_table";
    case formats::PnstParseError::slot_offset_before_payload:
        return "slot_offset_before_payload";
    case formats::PnstParseError::slot_offset_out_of_bounds:
        return "slot_offset_out_of_bounds";
    case formats::PnstParseError::invalid_document: return "invalid_document";
    }
    return "invalid_document";
}

class PacContainerParser final : public formats::IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "dmc3-pac-structural-v1";
    }

    [[nodiscard]] std::string_view format() const noexcept override {
        return "pac";
    }

    [[nodiscard]] bool supports_byte_identity_reuse() const noexcept override {
        return true;
    }

    [[nodiscard]] int probe(
        std::span<const std::byte> bytes,
        std::string_view) const noexcept override {
        return matches_magic(bytes, std::string_view{"PAC\0", 4U}) ? 100 : 0;
    }

    [[nodiscard]] formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        auto parsed = formats::PacParser::parse(bytes);
        formats::ContainerParseResult result;
        result.recognized = probe(bytes, {}) == 100;
        if (parsed.document.has_value()) {
            result.document = std::move(*parsed.document);
        }
        if (!parsed.ok()) {
            result.diagnostics.push_back(formats::ParseDiagnostic{
                .severity = result.recognized
                    ? formats::ParseSeverity::error
                    : formats::ParseSeverity::warning,
                .code = std::string{"dmc3.pac."} +
                    std::string{pac_error_suffix(parsed.error)},
                .message = parsed.message.empty()
                    ? "PAC structural parse failed."
                    : std::move(parsed.message),
                .offset = 0U,
            });
        }
        return result;
    }
};

class PnstContainerParser final : public formats::IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "dmc3-pnst-structural-v1";
    }

    [[nodiscard]] std::string_view format() const noexcept override {
        return "pnst";
    }

    [[nodiscard]] bool supports_byte_identity_reuse() const noexcept override {
        return true;
    }

    [[nodiscard]] int probe(
        std::span<const std::byte> bytes,
        std::string_view) const noexcept override {
        return matches_magic(bytes, "PNST") ? 100 : 0;
    }

    [[nodiscard]] formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        auto parsed = formats::PnstParser::parse(bytes);
        formats::ContainerParseResult result;
        result.recognized = probe(bytes, {}) == 100;
        if (parsed.document.has_value()) {
            result.document = std::move(*parsed.document);
        }
        if (!parsed.ok()) {
            result.diagnostics.push_back(formats::ParseDiagnostic{
                .severity = result.recognized
                    ? formats::ParseSeverity::error
                    : formats::ParseSeverity::warning,
                .code = std::string{"dmc3.pnst."} +
                    std::string{pnst_error_suffix(parsed.error)},
                .message = parsed.message.empty()
                    ? "PNST structural parse failed."
                    : std::move(parsed.message),
                .offset = 0U,
            });
        }
        return result;
    }
};

} // namespace

formats::ContainerParserRegistry make_container_parser_registry() {
    formats::ContainerParserRegistry registry;
    static_cast<void>(
        registry.register_parser(std::make_unique<PacContainerParser>()));
    static_cast<void>(
        registry.register_parser(std::make_unique<PnstContainerParser>()));
    return registry;
}

} // namespace dmc::rengine::profiles::dmc3
