#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool starts_with(
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

[[nodiscard]] formats::ContainerParseResult adapt_result(
    formats::RelativeSlotParseResult parsed,
    std::string_view diagnostic_prefix) {
    formats::ContainerParseResult result;
    result.recognized = true;

    if (parsed.document.has_value()) {
        result.document = std::move(*parsed.document);
    }

    if (!parsed.ok()) {
        std::string code{diagnostic_prefix};
        code += ".structural_error";
        result.diagnostics.push_back(formats::ParseDiagnostic{
            .severity = formats::ParseSeverity::error,
            .code = std::move(code),
            .message = parsed.message.empty()
                ? "The selected DMC3 relative-slot parser rejected the supplied bytes."
                : std::move(parsed.message),
            .offset = 0U,
        });
    }

    return result;
}

class PacContainerAdapter final : public formats::IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "dmc3-pac-clean-v1";
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
        return starts_with(bytes, std::string_view{"PAC\0", 4U}) ? 100 : 0;
    }

    [[nodiscard]] formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        return adapt_result(
            formats::PacParser::parse(bytes),
            "dmc3.pac");
    }
};

class PnstContainerAdapter final : public formats::IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "dmc3-pnst-clean-v1";
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
        return starts_with(bytes, "PNST") ? 100 : 0;
    }

    [[nodiscard]] formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        return adapt_result(
            formats::PnstParser::parse(bytes),
            "dmc3.pnst");
    }
};

} // namespace

formats::ContainerParserRegistry make_container_parser_registry() {
    formats::ContainerParserRegistry registry;
    static_cast<void>(
        registry.register_parser(std::make_unique<PacContainerAdapter>()));
    static_cast<void>(
        registry.register_parser(std::make_unique<PnstContainerAdapter>()));
    return registry;
}

} // namespace dmc::rengine::profiles::dmc3
