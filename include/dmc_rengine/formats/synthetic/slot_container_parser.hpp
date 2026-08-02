#pragma once

#include "dmc_rengine/formats/container_parser.hpp"

namespace dmc::rengine::formats::synthetic {

// Test-only invented container format. It models slot identity, empty slots,
// names, offsets, and partial corruption without claiming compatibility with
// PAC, PNST, AFS, NBZ, or any proprietary game format.
class SlotContainerParser final : public IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] std::string_view format() const noexcept override;
    [[nodiscard]] int probe(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const noexcept override;
    [[nodiscard]] ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const override;
};

} // namespace dmc::rengine::formats::synthetic
