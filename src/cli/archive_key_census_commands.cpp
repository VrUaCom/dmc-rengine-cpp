#include "archive_key_census_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

// SafeProductValidation: a census input is operator-supplied metadata, so the
// reader needs its own bounded limits rather than trusting the file.
constexpr std::size_t k_max_census_input_bytes = 64U * 1024U * 1024U;
constexpr std::size_t k_max_census_rows = 1U << 20U;

struct CensusRow final {
    std::string path;
    bool directory{};
};

struct CensusInput final {
    std::vector<CensusRow> rows;
    std::string source_sha256;
    std::string source_name;
};

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::string output;
    output.reserve(value.size() + 8U);
    for (const auto character : value) {
        switch (character) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default:
            if (static_cast<unsigned char>(character) < 0x20U) {
                static constexpr char digits[] = "0123456789abcdef";
                const auto value8 = static_cast<unsigned char>(character);
                output += "\\u00";
                output.push_back(digits[(value8 >> 4U) & 0x0FU]);
                output.push_back(digits[value8 & 0x0FU]);
            } else {
                output.push_back(character);
            }
        }
    }
    return output;
}

// Minimal RFC 4180 subset: comma separator, optional double quotes, "" escape.
[[nodiscard]] std::vector<std::string> split_csv_line(std::string_view line) {
    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;
    for (std::size_t index = 0U; index < line.size(); ++index) {
        const auto character = line[index];
        if (character == '"') {
            if (quoted && index + 1U < line.size() && line[index + 1U] == '"') {
                current.push_back('"');
                ++index;
            } else {
                quoted = !quoted;
            }
            continue;
        }
        if (character == ',' && !quoted) {
            fields.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(character);
    }
    fields.push_back(current);
    return fields;
}

[[nodiscard]] std::string trim(std::string_view value) {
    std::size_t begin = 0U;
    std::size_t end = value.size();
    const auto space = [](char character) {
        return character == ' ' || character == '\t' || character == '\r';
    };
    while (begin < end && space(value[begin])) {
        ++begin;
    }
    while (end > begin && space(value[end - 1U])) {
        --end;
    }
    return std::string{value.substr(begin, end - begin)};
}

[[nodiscard]] std::optional<CensusInput> read_census_csv(
    const std::string& path,
    std::string& error) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        error = "unable to open the central-directory CSV";
        return std::nullopt;
    }

    std::string line;
    if (!std::getline(stream, line)) {
        error = "the central-directory CSV is empty";
        return std::nullopt;
    }

    const auto header = split_csv_line(trim(line));
    std::size_t column_path = header.size();
    std::size_t column_directory = header.size();
    std::size_t column_sha = header.size();
    std::size_t column_source = header.size();
    for (std::size_t index = 0U; index < header.size(); ++index) {
        const auto field = trim(header[index]);
        if (field == "path") {
            column_path = index;
        } else if (field == "is_directory") {
            column_directory = index;
        } else if (field == "source_sha256") {
            column_sha = index;
        } else if (field == "source_name") {
            column_source = index;
        }
    }
    if (column_path == header.size()) {
        error = "the central-directory CSV has no \"path\" column";
        return std::nullopt;
    }

    CensusInput input;
    std::size_t consumed = line.size();
    while (std::getline(stream, line)) {
        consumed += line.size() + 1U;
        if (consumed > k_max_census_input_bytes) {
            error = "the central-directory CSV exceeds the census input budget";
            return std::nullopt;
        }
        if (trim(line).empty()) {
            continue;
        }
        if (input.rows.size() >= k_max_census_rows) {
            error = "the central-directory CSV exceeds the census row budget";
            return std::nullopt;
        }

        const auto fields = split_csv_line(line);
        if (fields.size() <= column_path) {
            continue;
        }
        CensusRow row;
        row.path = fields[column_path];
        row.directory = column_directory < fields.size() &&
            trim(fields[column_directory]) == "true";
        input.rows.push_back(std::move(row));

        if (input.source_sha256.empty() && column_sha < fields.size()) {
            input.source_sha256 = trim(fields[column_sha]);
        }
        if (input.source_name.empty() && column_source < fields.size()) {
            input.source_name = trim(fields[column_source]);
        }
    }

    if (input.rows.empty()) {
        error = "the central-directory CSV declares no entries";
        return std::nullopt;
    }
    return input;
}

struct CensusPass final {
    std::string scope;
    std::size_t considered{};
    std::size_t rejected{};
    std::size_t unique_keys{};
    std::size_t colliding_keys{};
    std::size_t colliding_entries{};
    std::vector<std::string> collision_examples;

    [[nodiscard]] std::size_t normalized_key_count() const noexcept {
        return considered - rejected;
    }

    [[nodiscard]] bool clean() const noexcept {
        return colliding_keys == 0U && rejected == 0U;
    }
};

// The census deliberately reuses the canonical ResourcePathPolicy normalizer
// rather than reimplementing 0x0E. A second normalizer here would make the
// census evidence about the census tool instead of about the product.
[[nodiscard]] CensusPass run_pass(
    const CensusInput& input,
    bool files_only) {
    CensusPass pass;
    pass.scope = files_only ? "files-only" : "all-central-entries";

    std::map<std::string, std::vector<const CensusRow*>, std::less<>> keys;
    for (const auto& row : input.rows) {
        if (files_only && row.directory) {
            continue;
        }
        ++pass.considered;

        if (!profiles::dmc3::ResourcePathPolicy::valid_input(row.path)) {
            ++pass.rejected;
            continue;
        }
        auto key = profiles::dmc3::ResourcePathPolicy::archive(row.path);
        if (key.empty()) {
            ++pass.rejected;
            continue;
        }
        keys[std::move(key)].push_back(&row);
    }

    pass.unique_keys = keys.size();
    for (const auto& [key, rows] : keys) {
        if (rows.size() < 2U) {
            continue;
        }
        ++pass.colliding_keys;
        pass.colliding_entries += rows.size();
        if (pass.collision_examples.size() < 32U) {
            pass.collision_examples.push_back(key);
        }
    }
    return pass;
}

void print_pass(const CensusPass& pass) {
    std::cout << "  scope                  : " << pass.scope << '\n'
              << "  entries considered     : " << pass.considered << '\n'
              << "  rejected by path policy: " << pass.rejected << '\n'
              << "  normalized_key_count   : " << pass.normalized_key_count()
              << '\n'
              << "  unique_normalized_keys : " << pass.unique_keys << '\n'
              << "  colliding keys         : " << pass.colliding_keys << '\n'
              << "  entries in collisions  : " << pass.colliding_entries << '\n'
              << "  result                 : "
              << (pass.clean() ? "ZERO COLLISIONS" : "COLLISIONS PRESENT")
              << '\n';
    for (const auto& key : pass.collision_examples) {
        std::cout << "    collision key: " << key << '\n';
    }
}

void write_pass_json(std::ostringstream& output, const CensusPass& pass) {
    output << "    {\n"
           << "      \"scope\": \"" << escape_json(pass.scope) << "\",\n"
           << "      \"entries_considered\": " << pass.considered << ",\n"
           << "      \"rejected_by_path_policy\": " << pass.rejected << ",\n"
           << "      \"normalized_key_count\": " << pass.normalized_key_count()
           << ",\n"
           << "      \"unique_normalized_key_count\": " << pass.unique_keys
           << ",\n"
           << "      \"colliding_key_count\": " << pass.colliding_keys << ",\n"
           << "      \"entries_in_collisions\": " << pass.colliding_entries
           << ",\n"
           << "      \"zero_collisions\": "
           << (pass.clean() ? "true" : "false") << "\n"
           << "    }";
}

[[nodiscard]] int run_census(
    const std::string& csv_path,
    const std::optional<std::string>& receipt_path) {
    std::string error;
    const auto input = read_census_csv(csv_path, error);
    if (!input.has_value()) {
        std::cerr << "census-archive-keys: " << error << '\n';
        return 1;
    }

    std::ifstream raw(csv_path, std::ios::binary);
    if (!raw) {
        std::cerr << "census-archive-keys: unable to reopen the CSV for hashing\n";
        return 1;
    }
    const std::string raw_bytes{
        std::istreambuf_iterator<char>(raw),
        std::istreambuf_iterator<char>()};
    const auto surface_digest = core::Sha256::compute(
        std::span<const std::byte>{
            reinterpret_cast<const std::byte*>(raw_bytes.data()),
            raw_bytes.size()})
        .hex();

    const auto files_only = run_pass(*input, true);
    const auto everything = run_pass(*input, false);

    std::cout << "Archive normalized-key census (profile flags 0x0E)\n"
              << "  source name            : "
              << (input->source_name.empty() ? "(not declared)"
                                             : input->source_name)
              << '\n'
              << "  archive sha256         : "
              << (input->source_sha256.empty() ? "(not declared)"
                                               : input->source_sha256)
              << '\n'
              << "  surface sha256         : " << surface_digest << '\n'
              << "  central entries        : " << input->rows.size() << "\n\n";
    print_pass(files_only);
    std::cout << '\n';
    print_pass(everything);

    const bool clean = files_only.clean() && everything.clean();
    std::cout << '\n'
              << "Census verdict: "
              << (clean ? "CLEAN — normalized_key_count == unique_normalized_key_count"
                        : "NOT CLEAN — duplicate normalized keys or rejected paths present")
              << '\n';

    if (receipt_path.has_value()) {
        std::ostringstream json;
        json << "{\n"
             << "  \"schema_version\": 1,\n"
             << "  \"kind\": \"archive-normalized-key-census\",\n"
             << "  \"normalizer\": \"profiles::dmc3::ResourcePathPolicy::archive\",\n"
             << "  \"normalizer_flags\": \"0x0E\",\n"
             << "  \"source_name\": \"" << escape_json(input->source_name)
             << "\",\n"
             << "  \"archive_sha256\": \"" << escape_json(input->source_sha256)
             << "\",\n"
             << "  \"surface_sha256\": \"" << surface_digest << "\",\n"
             << "  \"central_entry_count\": " << input->rows.size() << ",\n"
             << "  \"passes\": [\n";
        write_pass_json(json, files_only);
        json << ",\n";
        write_pass_json(json, everything);
        json << "\n  ],\n"
             << "  \"clean\": " << (clean ? "true" : "false") << "\n"
             << "}\n";

        const auto text = json.str();
        std::ofstream out(*receipt_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "census-archive-keys: unable to write the receipt\n";
            return 1;
        }
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        out.flush();
        if (!out) {
            std::cerr << "census-archive-keys: unable to flush the receipt\n";
            return 1;
        }
        std::cout << "Receipt written: " << *receipt_path << '\n';
    }

    return clean ? 0 : 2;
}

} // namespace

void print_archive_key_census_help() {
    std::cout
        << "  census-archive-keys <central-directory-csv> [receipt.json]\n"
        << "                            Run the 0x0E normalized-key collision census "
           "over an archive central-directory surface\n";
}

int try_run_archive_key_census_command(int argc, char** argv) {
    if (argc < 2) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command != "census-archive-keys") {
        return -1;
    }
    if (argc < 3 || argc > 4) {
        std::cerr << "census-archive-keys: expected <central-directory-csv> "
                     "[receipt.json]\n";
        return 1;
    }

    std::optional<std::string> receipt;
    if (argc == 4) {
        receipt = std::string{argv[3]};
    }
    return run_census(std::string{argv[2]}, receipt);
}

} // namespace dmc::rengine::cli
