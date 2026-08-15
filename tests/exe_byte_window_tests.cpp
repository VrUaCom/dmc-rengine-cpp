#include "dmc_rengine/exe/byte_window.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
    namespace exe = dmc::rengine::exe;

    exe::PeImage image;
    image.kind = exe::PeKind::pe32_plus;
    image.machine = exe::PeMachine::amd64;
    image.image_base = 0x140000000ULL;
    image.size_of_headers = 0x200U;
    image.sections.push_back(exe::PeSection{
        .name = ".text",
        .virtual_size = 0x300U,
        .virtual_address = 0x1000U,
        .raw_size = 0x200U,
        .raw_offset = 0x400U,
        .characteristics = 0U,
    });

    std::vector<std::byte> bytes(0x800U, std::byte{0});
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(index & 0xFFU);
    }

    const auto header = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x40U,
        0x10U);
    assert(header.ok());
    assert(header.window->rva == 0x40U);
    assert(header.window->file_offset == 0x40U);
    assert(header.window->section_name == "<headers>");
    assert(header.window->bytes.size() == 0x10U);
    assert(header.window->bytes.front() == std::byte{0x40});

    const auto text = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x1080U,
        0x20U);
    assert(text.ok());
    assert(text.window->rva == 0x1080U);
    assert(text.window->file_offset == 0x480U);
    assert(text.window->section_name == ".text");
    assert(text.window->bytes.size() == 0x20U);
    assert(text.window->bytes.front() == std::byte{0x80});
    assert(text.window->bytes.back() == std::byte{0x9F});

    const auto zero = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x1080U,
        0U);
    assert(!zero.ok());
    assert(zero.error == exe::ExeByteWindowError::zero_size);

    const auto too_large = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x1080U,
        exe::k_max_exe_byte_window_size + 1U);
    assert(!too_large.ok());
    assert(too_large.error == exe::ExeByteWindowError::size_limit_exceeded);

    const auto below_base = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base - 1U,
        1U);
    assert(!below_base.ok());
    assert(below_base.error == exe::ExeByteWindowError::va_below_image_base);

    const auto crosses_headers = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x1F0U,
        0x20U);
    assert(!crosses_headers.ok());
    assert(crosses_headers.error == exe::ExeByteWindowError::crosses_raw_mapping);

    const auto crosses_text_raw = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x11F0U,
        0x20U);
    assert(!crosses_text_raw.ok());
    assert(crosses_text_raw.error == exe::ExeByteWindowError::crosses_raw_mapping);

    // The section has virtual_size 0x300 but only 0x200 raw bytes. A VA in
    // the virtual-only tail is deliberately rejected for file-byte acquisition.
    const auto virtual_only = exe::ExeByteWindowExtractor::extract(
        bytes,
        image,
        image.image_base + 0x1250U,
        0x10U);
    assert(!virtual_only.ok());
    assert(virtual_only.error == exe::ExeByteWindowError::unmapped_rva);

    auto truncated = bytes;
    truncated.resize(0x490U);
    const auto truncated_result = exe::ExeByteWindowExtractor::extract(
        truncated,
        image,
        image.image_base + 0x1080U,
        0x20U);
    assert(!truncated_result.ok());
    assert(truncated_result.error == exe::ExeByteWindowError::file_range_out_of_bounds);

    return 0;
}
