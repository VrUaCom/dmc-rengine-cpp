#include "archive_key_census_commands.hpp"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

// SafeProductValidation: these fixtures are synthetic. They exercise the census
// mechanism only and never assert anything about a real retail archive.
void write_text(const std::filesystem::path& path, std::string_view text) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    assert(stream.good());
}

[[nodiscard]] std::string read_text(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    assert(stream);
    return std::string{
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()};
}

[[nodiscard]] int run(const std::vector<std::string>& arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    return dmc::rengine::cli::try_run_archive_key_census_command(
        static_cast<int>(argv.size()), argv.data());
}

constexpr std::string_view k_header =
    "source_name,source_sha256,central_index,path,is_directory\n";

void test_not_this_command() {
    assert(run({"dmc-rengine", "scan", "."}) == -1);
    assert(run({"dmc-rengine"}) == -1);
}

void test_usage() {
    assert(run({"dmc-rengine", "census-archive-keys"}) == 1);
    assert(run({"dmc-rengine", "census-archive-keys", "a", "b", "c"}) == 1);
}

void test_missing_file() {
    assert(run({"dmc-rengine", "census-archive-keys",
                "definitely-not-a-real-census-input.csv"}) == 1);
}

void test_clean_census_exits_zero() {
    const auto directory = std::filesystem::temp_directory_path() /
        "dmc-rengine-census-clean";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    const auto csv = directory / "surface.csv";
    write_text(
        csv,
        std::string{k_header} +
            "synthetic.nbz,aa,0,Root.afs/,true\n"
            "synthetic.nbz,aa,1,Root.afs/alpha.pac,false\n"
            "synthetic.nbz,aa,2,Root.afs/beta.pac,false\n"
            "synthetic.nbz,aa,3,Root.afs/sub/gamma.pac,false\n");

    const auto receipt = directory / "receipt.json";
    assert(run({"dmc-rengine", "census-archive-keys", csv.string(),
                receipt.string()}) == 0);

    const auto json = read_text(receipt);
    assert(json.find("\"central_entry_count\": 4") != std::string::npos);
    assert(json.find("\"clean\": true") != std::string::npos);
    assert(json.find("\"normalizer_flags\": \"0x0E\"") != std::string::npos);
    // Files-only pass drops the single directory entry.
    assert(json.find("\"entries_considered\": 3") != std::string::npos);
    assert(json.find("\"entries_considered\": 4") != std::string::npos);
    assert(json.find("\"colliding_key_count\": 0") != std::string::npos);

    std::filesystem::remove_all(directory);
}

void test_case_and_separator_collision_is_detected() {
    const auto directory = std::filesystem::temp_directory_path() /
        "dmc-rengine-census-collision";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    // 0x0E lowercases and folds '/' to '\\', so these three distinct central
    // names collapse onto one normalized key.
    const auto csv = directory / "surface.csv";
    write_text(
        csv,
        std::string{k_header} +
            "synthetic.nbz,bb,0,Root.afs/Alpha.pac,false\n"
            "synthetic.nbz,bb,1,root.afs/alpha.pac,false\n"
            "synthetic.nbz,bb,2,Root.afs\\\\ALPHA.PAC,false\n"
            "synthetic.nbz,bb,3,Root.afs/unique.pac,false\n");

    const auto receipt = directory / "receipt.json";
    // A non-clean census reports 2 so scripts can distinguish it from a
    // hard input failure, which reports 1.
    assert(run({"dmc-rengine", "census-archive-keys", csv.string(),
                receipt.string()}) == 2);

    const auto json = read_text(receipt);
    assert(json.find("\"clean\": false") != std::string::npos);
    assert(json.find("\"colliding_key_count\": 1") != std::string::npos);
    assert(json.find("\"entries_in_collisions\": 3") != std::string::npos);
    assert(json.find("\"unique_normalized_key_count\": 2") != std::string::npos);

    std::filesystem::remove_all(directory);
}

void test_repeated_separators_collapse() {
    const auto directory = std::filesystem::temp_directory_path() /
        "dmc-rengine-census-separators";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    // Leading/trailing separator stripping plus repeated-separator collapse
    // are part of the same recovered transformation.
    const auto csv = directory / "surface.csv";
    write_text(
        csv,
        std::string{k_header} +
            "synthetic.nbz,cc,0,Root.afs/alpha.pac,false\n"
            "synthetic.nbz,cc,1,/Root.afs//alpha.pac,false\n");

    assert(run({"dmc-rengine", "census-archive-keys", csv.string()}) == 2);

    std::filesystem::remove_all(directory);
}

void test_rejects_surface_without_path_column() {
    const auto directory = std::filesystem::temp_directory_path() /
        "dmc-rengine-census-no-path";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    const auto csv = directory / "surface.csv";
    write_text(csv, "source_name,central_index\nsynthetic.nbz,0\n");
    assert(run({"dmc-rengine", "census-archive-keys", csv.string()}) == 1);

    std::filesystem::remove_all(directory);
}

} // namespace

int main() {
    test_not_this_command();
    test_usage();
    test_missing_file();
    test_clean_census_exits_zero();
    test_case_and_separator_collision_is_detected();
    test_repeated_separators_collapse();
    test_rejects_surface_without_path_column();
    return 0;
}
