#pragma once

#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/stage_txt.hpp"

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace dmc::rengine::integration {

using ParsedResourceData = std::variant<
    std::monostate,
    formats::hits::ScanResult,
    formats::dca::ScanResult,
    formats::lig2::ScanResult,
    formats::stage_txt::LexResult>;

// One typed parser execution result retained by the canonical resource session.
// Binary Inspector, Stage Ops and other consumers must reuse this result instead
// of invoking a second independent parser path over the same source bytes.
struct ParsedResourceResult final {
    std::string parser_id;
    ParsedResourceData data;

    [[nodiscard]] bool valid() const noexcept {
        return !parser_id.empty() &&
            !std::holds_alternative<std::monostate>(data);
    }

    [[nodiscard]] bool recognized() const noexcept {
        return std::visit(
            [](const auto& value) -> bool {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    return false;
                } else {
                    return value.recognized;
                }
            },
            data);
    }

    template <class T>
    [[nodiscard]] const T* get_if() const noexcept {
        return std::get_if<T>(&data);
    }
};

} // namespace dmc::rengine::integration
