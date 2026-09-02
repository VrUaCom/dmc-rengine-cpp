#include "dmc_rengine/gdspaces/text_record.hpp"

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool is_single(unsigned char value) noexcept {
    return value == 0x09U || value == 0x0AU || value == 0x0DU ||
        (value >= 0x20U && value <= 0x7EU) ||
        // Half-width katakana occupy a single byte in Shift-JIS.
        (value >= 0xA1U && value <= 0xDFU);
}

[[nodiscard]] bool is_lead(unsigned char value) noexcept {
    return (value >= 0x81U && value <= 0x9FU) ||
        (value >= 0xE0U && value <= 0xEFU);
}

[[nodiscard]] bool is_trail(unsigned char value) noexcept {
    return (value >= 0x40U && value <= 0x7EU) ||
        (value >= 0x80U && value <= 0xFCU);
}

} // namespace

TextRecordView TextRecord::inspect(std::span<const std::byte> bytes) noexcept {
    TextRecordView view;
    view.encoding = k_encoding;

    // Records are padded to their container alignment with NUL. That padding
    // is not part of the text and must not be judged as if it were.
    auto end = bytes.size();
    while (end > 0U &&
           std::to_integer<unsigned char>(bytes[end - 1U]) == 0x00U) {
        --end;
    }
    view.text_bytes = end;
    view.padding_bytes = bytes.size() - end;

    if (end < 4U) {
        return view;
    }

    bool has_line_break = false;
    for (std::size_t index = 0; index < end;) {
        const auto value = std::to_integer<unsigned char>(bytes[index]);
        if (is_single(value)) {
            has_line_break = has_line_break || value == 0x0AU || value == 0x0DU;
            ++index;
            continue;
        }
        if (is_lead(value) && index + 1U < end &&
            is_trail(std::to_integer<unsigned char>(bytes[index + 1U]))) {
            index += 2U;
            continue;
        }
        return view;
    }

    // A single printable run with no terminator is far more likely to be the
    // opening of a binary record than a text file, so require the line
    // structure every observed authoring record has.
    view.recognized = has_line_break;
    return view;
}

} // namespace dmc::rengine::gdspaces
