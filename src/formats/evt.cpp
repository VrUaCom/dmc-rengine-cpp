#include "dmc_rengine/formats/evt.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::formats::evt {
namespace {

[[nodiscard]] bool has_error(
    const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

void add_diagnostic(
    ParseResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] bool all_zero(std::span<const std::byte> bytes) noexcept {
    return std::all_of(
        bytes.begin(), bytes.end(),
        [](std::byte value) { return value == std::byte{0}; });
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult Parser::parse(std::span<const std::byte> bytes) {
    ParseResult result;
    const binary::Reader reader(bytes);

    if (!reader.matches(0U, std::string_view{"EVT\0", 4U})) {
        return result;
    }
    result.recognized = true;
    result.document.source_bytes.assign(bytes.begin(), bytes.end());

    if (bytes.size() < header_size) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.truncated-header",
            "EVT payload is shorter than the 0x20-byte corpus-confirmed header.",
            bytes.size());
        return result;
    }

    const auto version = reader.u32_le(0x04U);
    const auto terminal_offset = reader.u32_le(0x08U);
    if (!version || !terminal_offset) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.header-fields",
            "EVT version or terminal-command offset is truncated.",
            0x04U);
        return result;
    }

    result.document.header.version = *version;
    result.document.header.terminal_command_offset = *terminal_offset;
    result.document.header.preserved_reserved_bytes.assign(
        bytes.begin() + 0x0CU,
        bytes.begin() + static_cast<std::ptrdiff_t>(header_size));

    if (*version != corpus_version) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.unconfirmed-version",
            "EVT version differs from the 0x00010001 value observed in the current EventTbl13-21 corpus; raw value is preserved.",
            0x04U);
    }
    if (!all_zero(std::span<const std::byte>{
            result.document.header.preserved_reserved_bytes.data(),
            result.document.header.preserved_reserved_bytes.size()})) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.header-reserved-nonzero",
            "EVT header +0x0C..+0x1F is non-zero outside the current corpus; bytes are preserved.",
            0x0CU);
    }

    const auto terminal = static_cast<std::size_t>(*terminal_offset);
    if (terminal < header_size || (terminal & 3U) != 0U ||
        terminal > bytes.size() || bytes.size() - terminal < 4U) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.terminal-offset-invalid",
            "EVT terminal-command offset is not a bounded 4-byte-aligned command address.",
            0x08U);
        return result;
    }

    std::size_t cursor = header_size;
    while (cursor <= terminal) {
        const auto raw_header = reader.u32_le(cursor);
        if (!raw_header) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.command-header-truncated",
                "EVT command header is truncated.",
                cursor);
            return result;
        }
        if ((*raw_header & 0xFFFF0000U) != 0U) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.command-header-upper-bits",
                "EVT command descriptor uses non-zero upper 16 bits; the recovered opcode/arity grammar cannot decode it losslessly.",
                cursor);
            return result;
        }

        Command command;
        command.offset = cursor;
        command.raw_header = *raw_header;
        command.opcode = static_cast<std::uint8_t>(*raw_header & 0xFFU);
        command.argument_count = static_cast<std::uint8_t>((*raw_header >> 8U) & 0xFFU);

        const auto command_size = static_cast<std::size_t>(command.serialized_size());
        if (command_size > bytes.size() - cursor ||
            (cursor < terminal && command_size > terminal - cursor)) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.command-range",
                "EVT command arguments escape the command stream or cross the declared terminal-command boundary.",
                cursor);
            return result;
        }

        command.arguments.reserve(command.argument_count);
        for (std::size_t argument = 0U;
             argument < command.argument_count;
             ++argument) {
            const auto value = reader.u32_le(cursor + 4U + argument * 4U);
            if (!value) {
                add_diagnostic(
                    result, ParseSeverity::error,
                    "evt.argument-truncated",
                    "EVT command argument is truncated.",
                    cursor + 4U + argument * 4U);
                return result;
            }
            command.arguments.push_back(*value);
        }
        result.document.commands.push_back(std::move(command));

        if (cursor == terminal) break;
        cursor += command_size;
    }

    if (result.document.commands.empty() ||
        result.document.commands.back().offset != terminal) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.terminal-unreachable",
            "EVT command walk does not land exactly on the declared terminal-command offset.",
            terminal);
        return result;
    }

    const auto& terminal_command = result.document.commands.back();
    if (terminal_command.opcode != terminal_opcode ||
        terminal_command.argument_count != 0U) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.terminal-command",
            "EVT terminal offset does not contain the corpus-confirmed opcode 0x20 with zero arguments.",
            terminal);
        return result;
    }

    if (result.document.commands.size() < 2U ||
        result.document.commands[result.document.commands.size() - 2U].opcode !=
            pre_terminal_opcode ||
        result.document.commands[result.document.commands.size() - 2U].argument_count != 0U) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.pre-terminal-variant",
            "EVT stream does not end in the 0x01,0x20 zero-argument pair observed in all current EventTbl13-21 samples.",
            terminal);
    }

    const auto trailing_begin = terminal + 4U;
    result.document.trailing_padding.assign(
        bytes.begin() + static_cast<std::ptrdiff_t>(trailing_begin),
        bytes.end());
    if (!all_zero(std::span<const std::byte>{
            result.document.trailing_padding.data(),
            result.document.trailing_padding.size()})) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.trailing-nonzero",
            "EVT bytes after the terminal command are non-zero; trailing bytes are preserved without semantic interpretation.",
            trailing_begin);
    }
    if ((bytes.size() & 0x1FU) != 0U) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.file-alignment-variant",
            "EVT payload size is not 0x20-aligned as in the current EventTbl13-21 corpus.",
            bytes.size());
    }

    return result;
}

} // namespace dmc::rengine::formats::evt
