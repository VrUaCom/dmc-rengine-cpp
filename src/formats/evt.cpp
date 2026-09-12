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

    const auto packed_header = reader.u32_le(0x04U);
    const auto terminal_offset = reader.u32_le(0x08U);
    if (!packed_header || !terminal_offset) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.header-fields",
            "EVT packed revision/stream-count word or terminal-command offset is truncated.",
            0x04U);
        return result;
    }

    result.document.header.version = *packed_header;
    result.document.header.revision =
        static_cast<std::uint16_t>(*packed_header & 0xFFFFU);
    result.document.header.stream_count =
        static_cast<std::uint16_t>((*packed_header >> 16U) & 0xFFFFU);
    result.document.header.terminal_command_offset = *terminal_offset;
    result.document.header.preserved_reserved_bytes.assign(
        bytes.begin() + 0x0CU,
        bytes.begin() + static_cast<std::ptrdiff_t>(header_size));

    if (result.document.header.revision != corpus_revision) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.unconfirmed-revision",
            "EVT low16 revision differs from revision 1 observed in the supplied EventTbl corpora; raw value is preserved.",
            0x04U);
    }
    if (result.document.header.stream_count == 0U) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.stream-count-zero",
            "EVT high16 stream count is zero; the recovered stream-offset grammar requires at least one stream.",
            0x06U);
        return result;
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

    const auto expected_pre_terminal =
        result.document.header.stream_count == 1U
            ? single_stream_pre_terminal_opcode
            : multi_stream_pre_terminal_opcode;
    if (result.document.commands.size() < 2U ||
        result.document.commands[result.document.commands.size() - 2U].opcode !=
            expected_pre_terminal ||
        result.document.commands[result.document.commands.size() - 2U].argument_count != 0U) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.pre-terminal-variant",
            result.document.header.stream_count == 1U
                ? "Single-stream EVT does not end in the corpus-observed 0x01,0x20 zero-argument pair."
                : "Multi-stream EVT does not end in the corpus-observed 0x0E,0x20 zero-argument pair.",
            terminal);
    }

    const auto stream_table_begin = terminal + 4U;
    const auto additional_stream_count =
        static_cast<std::size_t>(result.document.header.stream_count - 1U);
    const auto stream_table_size = additional_stream_count * 4U;
    if (stream_table_begin > bytes.size() ||
        stream_table_size > bytes.size() - stream_table_begin) {
        add_diagnostic(
            result, ParseSeverity::error,
            "evt.stream-table-truncated",
            "EVT does not contain the high16 stream_count-1 internal offsets after the terminal command.",
            stream_table_begin);
        return result;
    }

    result.document.stream_offsets.reserve(result.document.header.stream_count);
    result.document.stream_offsets.push_back(static_cast<std::uint32_t>(header_size));
    for (std::size_t index = 0U; index < additional_stream_count; ++index) {
        const auto table_offset = stream_table_begin + index * 4U;
        const auto stream_offset = reader.u32_le(table_offset);
        if (!stream_offset) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.stream-offset-truncated",
                "EVT internal stream offset is truncated.",
                table_offset);
            return result;
        }

        if (*stream_offset < header_size || *stream_offset >= terminal ||
            (*stream_offset & 3U) != 0U ||
            *stream_offset <= result.document.stream_offsets.back()) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.stream-offset-invalid",
                "EVT internal stream offsets must be strictly increasing 4-byte-aligned command addresses before the terminal command.",
                table_offset);
            return result;
        }

        const auto command_it = std::find_if(
            result.document.commands.begin(), result.document.commands.end(),
            [stream_offset](const Command& command) {
                return command.offset == *stream_offset;
            });
        if (command_it == result.document.commands.end()) {
            add_diagnostic(
                result, ParseSeverity::error,
                "evt.stream-offset-not-command",
                "EVT internal stream offset does not land on a decoded command boundary.",
                table_offset);
            return result;
        }

        if (command_it->opcode != observed_stream_entry_opcode ||
            command_it->argument_count != 1U) {
            add_diagnostic(
                result, ParseSeverity::warning,
                "evt.stream-entry-variant",
                "EVT internal stream offset does not target the 0x57/argc1 entry descriptor observed throughout the complete GData.afs EventTbl00-09 corpus.",
                *stream_offset);
        }
        if (command_it == result.document.commands.begin() ||
            (command_it - 1)->opcode != observed_stream_boundary_opcode ||
            (command_it - 1)->argument_count != 0U) {
            add_diagnostic(
                result, ParseSeverity::warning,
                "evt.stream-boundary-variant",
                "EVT internal stream start is not immediately preceded by the 0x00/argc0 boundary observed throughout the complete GData.afs EventTbl00-09 corpus.",
                *stream_offset);
        }

        result.document.stream_offsets.push_back(*stream_offset);
    }

    const auto trailing_begin = stream_table_begin + stream_table_size;
    result.document.trailing_padding.assign(
        bytes.begin() + static_cast<std::ptrdiff_t>(trailing_begin),
        bytes.end());
    if (!all_zero(std::span<const std::byte>{
            result.document.trailing_padding.data(),
            result.document.trailing_padding.size()})) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.trailing-nonzero",
            "EVT bytes after the decoded stream-offset table are non-zero; remaining bytes are preserved without semantic interpretation.",
            trailing_begin);
    }
    if ((bytes.size() & 0x1FU) != 0U) {
        add_diagnostic(
            result, ParseSeverity::warning,
            "evt.file-alignment-variant",
            "EVT payload size is not 0x20-aligned as in the supplied EventTbl corpora.",
            bytes.size());
    }

    return result;
}

} // namespace dmc::rengine::formats::evt
