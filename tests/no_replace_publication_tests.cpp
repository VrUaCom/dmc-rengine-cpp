// Publishing a file without replacing anything.
//
// The publisher committed by hard link, and reported the link's errno when it
// failed. That is a claim about permissions, and on a real phone it was a
// claim about the wrong thing entirely: the exporter had just created a
// directory and written a file in the very folder it then could not link in.
// Writes worked; links did not. Plenty of filesystems are like that —
// FUSE-backed emulated storage on Android, FAT and exFAT on removable cards —
// and every export to one of them failed with "Permission denied" and left a
// zero-byte file behind.
//
// So there is a second way to publish, and these pin that it keeps the
// guarantee the link gave: created atomically, never replacing.

#include "dmc_rengine/core/no_replace_publication.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

namespace core = dmc::rengine::core;

[[nodiscard]] std::vector<std::byte> payload(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const auto character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] std::string read_back(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string{
        std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
}

[[nodiscard]] std::filesystem::path fresh_directory(std::string_view name) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

/**
 * A directory on a different filesystem, when the machine has one.
 *
 * This is how a hard link is made genuinely impossible rather than mocked: a
 * link across filesystems cannot be created, which is the same refusal a phone
 * gives for its own reasons. Where no second filesystem exists the case is
 * skipped out loud, because a test that silently passes on the machine that
 * cannot run it is worse than one that says so.
 */
[[nodiscard]] std::optional<std::filesystem::path> other_filesystem() {
    const std::filesystem::path candidate{"/dev/shm"};
    std::error_code error;
    if (!std::filesystem::is_directory(candidate, error) || error) {
        return std::nullopt;
    }
    const auto here = std::filesystem::temp_directory_path();
    if (std::filesystem::space(candidate, error).capacity ==
            std::filesystem::space(here, error).capacity ||
        error) {
        // Same capacity is a decent proxy for the same mount; if they match,
        // this would not exercise the fallback and must not claim to.
        return std::nullopt;
    }
    const auto path = candidate / "dmc-rengine-publish-across";
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

void bytes_are_published_and_staging_is_cleaned_up() {
    const auto directory = fresh_directory("dmc-rengine-publish");
    const auto destination = directory / "resource.bin";

    const auto result = core::publish_bytes_no_replace(destination, payload("dante"));
    if (!result.ok()) std::cerr << result.detail << "\n";
    assert(result.ok());
    assert(read_back(destination) == "dante");

    // Nothing else may survive beside it: a leftover staging directory is a
    // reservation that would refuse the next publication to the same name.
    std::size_t entries = 0;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        static_cast<void>(entry);
        ++entries;
    }
    assert(entries == 1U);

    std::filesystem::remove_all(directory);
}

void an_existing_destination_is_never_replaced() {
    const auto directory = fresh_directory("dmc-rengine-publish-exists");
    const auto destination = directory / "resource.bin";
    {
        std::ofstream seed(destination, std::ios::binary);
        seed << "original";
    }

    const auto result = core::publish_bytes_no_replace(destination, payload("new"));
    assert(!result.ok());
    assert(result.status == core::NoReplacePublicationStatus::destination_exists);
    // The point of the whole exercise: the bytes already there are still there.
    assert(read_back(destination) == "original");

    std::filesystem::remove_all(directory);
}

void publication_succeeds_where_a_hard_link_is_impossible() {
    const auto across = other_filesystem();
    if (!across.has_value()) {
        std::cout << "  (skipped: this machine has no second filesystem to "
                     "make a hard link impossible)\n";
        return;
    }

    const auto directory = fresh_directory("dmc-rengine-publish-staged");
    const auto staged = directory / "payload.bin";
    {
        std::ofstream stream(staged, std::ios::binary);
        stream << "dante-model-bytes";
    }
    const auto destination = *across / "resource.bin";
    std::filesystem::remove(destination);

    // A hard link from one filesystem to another cannot be made. Before the
    // fallback this returned publication-failed and the export was lost.
    const auto result =
        core::publish_validated_file_no_replace(staged, destination);
    if (!result.ok()) std::cerr << result.detail << "\n";
    assert(result.ok());
    assert(read_back(destination) == "dante-model-bytes");
    assert(std::filesystem::file_size(destination) == 17U);

    std::filesystem::remove_all(directory);
    std::filesystem::remove_all(*across);
}

void the_fallback_still_refuses_an_existing_destination() {
    const auto across = other_filesystem();
    if (!across.has_value()) {
        std::cout << "  (skipped: no second filesystem)\n";
        return;
    }

    const auto directory = fresh_directory("dmc-rengine-publish-staged2");
    const auto staged = directory / "payload.bin";
    {
        std::ofstream stream(staged, std::ios::binary);
        stream << "replacement";
    }
    const auto destination = *across / "resource.bin";
    {
        std::ofstream seed(destination, std::ios::binary);
        seed << "original";
    }

    // The fallback is a different mechanism, not a different guarantee.
    const auto result =
        core::publish_validated_file_no_replace(staged, destination);
    assert(!result.ok());
    assert(result.status == core::NoReplacePublicationStatus::destination_exists);
    assert(read_back(destination) == "original");

    std::filesystem::remove_all(directory);
    std::filesystem::remove_all(*across);
}

void a_refusal_names_both_mechanisms() {
    // Nothing can publish into a directory that does not exist, and the
    // message has to say what was tried. The old one named only the link,
    // which sent anyone reading it after a permission problem that was not
    // there.
    const auto directory = fresh_directory("dmc-rengine-publish-gone");
    const auto staged = directory / "payload.bin";
    {
        std::ofstream stream(staged, std::ios::binary);
        stream << "x";
    }
    const auto destination = directory / "missing" / "resource.bin";

    const auto result =
        core::publish_validated_file_no_replace(staged, destination);
    assert(!result.ok());
    assert(result.status == core::NoReplacePublicationStatus::publication_failed);
    assert(result.detail.find("Hard link:") != std::string::npos);
    assert(result.detail.find("Exclusive create:") != std::string::npos);

    std::filesystem::remove_all(directory);
}

} // namespace

int main() {
    bytes_are_published_and_staging_is_cleaned_up();
    an_existing_destination_is_never_replaced();
    publication_succeeds_where_a_hard_link_is_impossible();
    the_fallback_still_refuses_an_existing_destination();
    a_refusal_names_both_mechanisms();
    std::cout << "no_replace_publication_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
