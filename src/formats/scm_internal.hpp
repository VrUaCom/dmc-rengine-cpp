#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"

#include <algorithm>
#include <bit>
#include <cstring>
#include <string>
#include <type_traits>

namespace dmc::rengine::formats::scm::detail {

inline void diag(ParseResult& out, ParseSeverity severity, std::string code,
                 std::string message, std::uint64_t offset) {
    out.diagnostics.push_back({severity, std::move(code), std::move(message), offset});
}

struct Reader final {
    std::span<const std::byte> bytes;

    [[nodiscard]] bool has(std::uint64_t off, std::uint64_t size) const noexcept {
        return off <= bytes.size() && size <= bytes.size() - static_cast<std::size_t>(off);
    }

    template <class T>
    bool read(std::uint64_t off, T& value) const noexcept {
        static_assert(std::is_trivially_copyable_v<T>);
        if (!has(off, sizeof(T))) return false;
        std::memcpy(&value, bytes.data() + static_cast<std::size_t>(off), sizeof(T));
        if constexpr (std::endian::native == std::endian::big && sizeof(T) > 1U) {
            auto* p = reinterpret_cast<std::byte*>(&value);
            std::reverse(p, p + sizeof(T));
        }
        return true;
    }

    [[nodiscard]] bool zero(std::uint64_t off, std::uint64_t size) const noexcept {
        if (!has(off, size)) return false;
        const auto begin = bytes.begin() + static_cast<std::ptrdiff_t>(off);
        return std::all_of(begin, begin + static_cast<std::ptrdiff_t>(size),
                           [](std::byte v) { return v == std::byte{0}; });
    }

    [[nodiscard]] std::uint8_t u8(std::uint64_t off) const noexcept {
        return std::to_integer<std::uint8_t>(bytes[static_cast<std::size_t>(off)]);
    }
};

[[nodiscard]] inline bool has_error(const ParseResult& out) noexcept {
    return std::any_of(out.diagnostics.begin(), out.diagnostics.end(),
                       [](const ParseDiagnostic& d) { return d.severity == ParseSeverity::error; });
}

void validate_serialized_document(std::span<const std::byte> bytes,
                                  const std::vector<ObjectShape>& shapes,
                                  ParseResult& out);

} // namespace dmc::rengine::formats::scm::detail
