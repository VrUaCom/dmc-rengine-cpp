#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace dmc::rengine::gdspaces {

// A DMC3 authoring record is line-oriented text in the encoding the studio
// wrote it in: ASCII with Shift-JIS comments. `st001.pac` slot 4 comments its
// uv line in Japanese, so a rule that only admits ASCII would call that record
// binary and hide it.
//
// This is the single place that decides what "text" means here, so the
// classifier and any reader that shows the text to an operator can never
// disagree about which records are readable.
struct TextRecordView final {
    bool recognized{false};

    // The encoding the record is validated against, named so a consumer
    // decodes it with the same answer this validation assumed.
    std::string_view encoding;

    // Length excluding the trailing NUL padding the container adds to reach
    // its alignment. That padding is stored, but it is not text.
    std::size_t text_bytes{};
    std::size_t padding_bytes{};
};

class TextRecord final {
public:
    static constexpr std::string_view k_encoding = "shift-jis";

    // Validates the whole record rather than sampling it, and returns on the
    // first byte that cannot be part of a text stream — so a multi-megabyte
    // binary record costs one comparison, not a scan.
    [[nodiscard]] static TextRecordView inspect(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::gdspaces
