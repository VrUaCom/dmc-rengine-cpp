#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_authoring_detail {

[[nodiscard]] inline bool parse_u64(
    std::string_view text,
    std::uint64_t& value) noexcept {
    if (text.empty()) return false;
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    return result.ec == std::errc{} && result.ptr == last;
}

[[nodiscard]] inline bool read_file(
    const std::filesystem::path& path,
    std::vector<std::byte>& bytes) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) return false;
    const auto end = stream.tellg();
    if (end < 0) return false;
    bytes.resize(static_cast<std::size_t>(end));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    return static_cast<bool>(stream) || bytes.empty();
}

[[nodiscard]] inline bool write_new_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::error_code error;
    if (std::filesystem::exists(path, error) || error) return false;
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) return false;
    if (!bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    return static_cast<bool>(stream);
}

[[nodiscard]] inline std::vector<std::size_t> changed_offsets(
    std::span<const std::byte> before,
    std::span<const std::byte> after) {
    std::vector<std::size_t> offsets;
    const auto common = before.size() < after.size() ? before.size() : after.size();
    for (std::size_t index = 0U; index < common; ++index) {
        if (before[index] != after[index]) offsets.push_back(index);
    }
    if (before.size() != after.size()) {
        const auto larger = before.size() > after.size() ? before.size() : after.size();
        for (std::size_t index = common; index < larger; ++index) {
            offsets.push_back(index);
        }
    }
    return offsets;
}

} // namespace scm_authoring_detail

inline void print_scm_authoring_help() {
    std::cout
        << "  scm-set-alpha-control <input.scm> <object-index> <0..255> <output.scm>\n"
        << "                             Bounded preserve-layout SCM alpha authoring with exact-byte guard\n";
}

inline int try_run_scm_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::set_alpha_control;

    if (argc <= 1 || std::string_view{argv[1]} != "scm-set-alpha-control") {
        return -1;
    }
    if (argc != 6) {
        std::cerr
            << "usage: scm-set-alpha-control <input.scm> <object-index> <0..255> <output.scm>\n";
        return 1;
    }

    std::uint64_t object_index_raw = 0U;
    std::uint64_t alpha_raw = 0U;
    if (!scm_authoring_detail::parse_u64(argv[3], object_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[4], alpha_raw) ||
        alpha_raw > 0xFFU) {
        std::cerr << "scm-set-alpha-control: invalid object index or alpha value\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[5]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-alpha-control: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-alpha-control: canonical parse failed\n";
        return 4;
    }
    const auto object_index = static_cast<std::size_t>(object_index_raw);
    if (object_index >= parsed.document.objects.size()) {
        std::cerr << "scm-set-alpha-control: object index out of range\n";
        return 5;
    }

    auto document = parsed.document;
    const auto old_alpha = document.objects[object_index].alpha_control;
    const auto object_offset = document.objects[object_index].record_offset;
    const auto authored_offset = object_offset + 0x01U;
    const auto new_alpha = static_cast<std::uint8_t>(alpha_raw);
    if (old_alpha == new_alpha) {
        std::cerr << "scm-set-alpha-control: requested value is already present\n";
        return 6;
    }

    const auto edit = set_alpha_control(document, object_index, new_alpha);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-alpha-control: typed edit rejected\n";
        return 7;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr << "scm-set-alpha-control: preserve-layout writer rejected output\n";
        return 8;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr << "scm-set-alpha-control: preserve-layout size changed\n";
        return 9;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    if (diffs.size() != 1U || diffs[0] != authored_offset) {
        std::cerr
            << "scm-set-alpha-control: exact-byte guard rejected unexpected output diff\n";
        return 10;
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        reparsed.document.objects[object_index].alpha_control != new_alpha) {
        std::cerr << "scm-set-alpha-control: canonical output reparse mismatch\n";
        return 11;
    }

    if (!scm_authoring_detail::write_new_file(
            output, std::span<const std::byte>{written.bytes})) {
        std::cerr
            << "scm-set-alpha-control: cannot create output (existing files are never overwritten)\n";
        return 12;
    }

    std::cout
        << "SCM_ALPHA_EDIT_OK"
        << " object=" << object_index
        << " old=" << static_cast<unsigned>(old_alpha)
        << " new=" << static_cast<unsigned>(new_alpha)
        << " objectOffset=" << object_offset
        << " changedOffset=" << diffs[0]
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " reparse=PASS"
        << " exactByteGuard=PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
