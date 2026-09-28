#pragma once

#include "scm_authoring_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_reintegration_detail {

namespace core = dmc::rengine::core;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] inline std::optional<unsigned int> parse_slot_index(
    std::string_view text) noexcept {
    if (text.empty()) return std::nullopt;
    unsigned long long value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc{} || result.ptr != last ||
        value > std::numeric_limits<unsigned int>::max()) {
        return std::nullopt;
    }
    return static_cast<unsigned int>(value);
}

[[nodiscard]] inline std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) return path.lexically_normal();
    auto canonical = std::filesystem::weakly_canonical(absolute, error);
    return error ? absolute.lexically_normal() : canonical;
}

[[nodiscard]] inline std::optional<gdspaces::ResourcePayload> read_local_resource(
    const std::filesystem::path& path,
    std::string_view source_id) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }
    const auto size = std::filesystem::file_size(absolute, error);
    if (error) return std::nullopt;

    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        return std::nullopt;
    }
    const gdspaces::ResourceId id{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = size,
    };
    return registry.read(id);
}

[[nodiscard]] inline std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

struct SlotSnapshot final {
    bool populated{};
    std::uint64_t source_offset{};
    std::vector<std::byte> bytes;
};

[[nodiscard]] inline std::optional<std::vector<SlotSnapshot>> snapshot_slots(
    const gdspaces::ContainerExpansion& expansion) {
    std::vector<SlotSnapshot> snapshots;
    snapshots.reserve(expansion.children.size());
    for (const auto& child : expansion.children) {
        SlotSnapshot snapshot{
            .populated = child.entry.populated,
            .source_offset = child.entry.offset,
            .bytes = {},
        };
        if (child.entry.populated) {
            if (!child.payload.readable()) return std::nullopt;
            snapshot.bytes = child.payload.bytes;
        }
        snapshots.push_back(std::move(snapshot));
    }
    return snapshots;
}

struct VerificationContext final {
    unsigned int slot_index{};
    std::uint64_t target_source_offset{};
    std::vector<std::byte> authored_scm;
    std::vector<SlotSnapshot> source_slots;
    std::string expected_format;
    std::uint32_t expected_slot_count{};
};

[[nodiscard]] inline bool validate_staged_reintegration(
    const std::filesystem::path& staged_path,
    const VerificationContext& context) {
    auto staged = read_local_resource(
        staged_path, "scm-reintegration-staged-validation");
    if (!staged.has_value() || !staged->readable()) return false;

    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{staged->bytes.data(), staged->bytes.size()},
        staged_path.filename().generic_string());
    if (!parsed.ok() || parsed.document.format != context.expected_format ||
        parsed.document.declared_slot_count != context.expected_slot_count) {
        return false;
    }

    const auto expansion = gdspaces::ContainerExpander::expand(*staged, parsed);
    if (!expansion.usable() ||
        expansion.children.size() != context.source_slots.size() ||
        context.slot_index >= expansion.children.size()) {
        return false;
    }

    bool target_seen = false;
    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        const auto& before = context.source_slots[index];
        const auto& after = expansion.children[index];
        if (before.populated != after.entry.populated) return false;
        if (!before.populated) continue;
        if (!after.payload.readable()) return false;

        const bool target_alias =
            before.source_offset == context.target_source_offset;
        if (target_alias) {
            target_seen = true;
            if (after.payload.bytes != context.authored_scm) return false;
            const auto authored_parse = formats::scm::Parser::parse(
                std::span<const std::byte>{
                    after.payload.bytes.data(), after.payload.bytes.size()});
            if (!authored_parse.ok()) return false;
            continue;
        }

        if (before.bytes != after.payload.bytes) return false;
    }
    return target_seen;
}

} // namespace scm_reintegration_detail

inline void print_scm_reintegration_help() {
    std::cout
        << "  verify-scm-reintegration <source.scm> <authored.scm> <parent.pac|pnst> <slot-index> <output-parent>\n"
        << "                             Provenance-bound SCM child replacement with staged reopen, target-alias rematerialization, exact non-target physical-span preservation and SCM reparse\n";
}

inline int try_run_scm_reintegration_command(int argc, char** argv) {
    namespace detail = scm_reintegration_detail;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    if (argc <= 1 || std::string_view{argv[1]} != "verify-scm-reintegration") {
        return -1;
    }
    if (argc != 7) {
        std::cerr
            << "usage: verify-scm-reintegration <source.scm> <authored.scm> <parent.pac|pnst> <slot-index> <output-parent>\n";
        return 1;
    }

    const std::filesystem::path source_scm_path{argv[2]};
    const std::filesystem::path authored_scm_path{argv[3]};
    const std::filesystem::path parent_path{argv[4]};
    const std::filesystem::path output_path{argv[6]};
    const auto slot_index = detail::parse_slot_index(argv[5]);
    if (!slot_index.has_value()) {
        std::cerr << "verify-scm-reintegration: invalid slot index\n";
        return 2;
    }
    if (detail::normalized_absolute(parent_path) ==
        detail::normalized_absolute(output_path)) {
        std::cerr
            << "verify-scm-reintegration: source parent and output parent must be distinct paths\n";
        return 2;
    }

    std::vector<std::byte> source_scm;
    std::vector<std::byte> authored_scm;
    if (!scm_authoring_detail::read_file(source_scm_path, source_scm) ||
        !scm_authoring_detail::read_file(authored_scm_path, authored_scm)) {
        std::cerr << "verify-scm-reintegration: cannot read SCM input\n";
        return 3;
    }
    if (source_scm == authored_scm) {
        std::cerr
            << "verify-scm-reintegration: authored SCM is byte-identical to source; controlled edit evidence is required\n";
        return 4;
    }

    const auto source_parse = formats::scm::Parser::parse(
        std::span<const std::byte>{source_scm});
    const auto authored_parse = formats::scm::Parser::parse(
        std::span<const std::byte>{authored_scm});
    if (!source_parse.ok() || !authored_parse.ok()) {
        std::cerr
            << "verify-scm-reintegration: source or authored SCM failed canonical parse\n";
        return 5;
    }

    auto parent = detail::read_local_resource(parent_path, "scm-reintegration-parent");
    if (!parent.has_value() || !parent->readable()) {
        std::cerr << "verify-scm-reintegration: parent container is not readable\n";
        return 6;
    }

    const auto registry = dmc3::make_container_parser_registry();
    const auto parent_parse = registry.parse(
        std::span<const std::byte>{parent->bytes.data(), parent->bytes.size()},
        parent->resource.id.logical_path);
    if (!parent_parse.ok() ||
        (parent_parse.document.format != "PAC" &&
         parent_parse.document.format != "PNST")) {
        std::cerr
            << "verify-scm-reintegration: parent is not a canonical PAC/PNST relative-slot container\n";
        return 7;
    }

    const auto source_expansion =
        detail::gdspaces::ContainerExpander::expand(*parent, parent_parse);
    if (!source_expansion.usable() ||
        *slot_index >= source_expansion.children.size()) {
        std::cerr
            << "verify-scm-reintegration: target slot is outside the parsed topology\n";
        return 8;
    }
    const auto& source_child = source_expansion.children[*slot_index];
    if (!source_child.entry.populated || !source_child.payload.readable()) {
        std::cerr
            << "verify-scm-reintegration: target slot is empty or unreadable\n";
        return 8;
    }
    if (source_child.payload.bytes != source_scm) {
        std::cerr
            << "verify-scm-reintegration: parent target slot does not exactly match source SCM bytes\n";
        return 9;
    }

    const auto source_slots = detail::snapshot_slots(source_expansion);
    if (!source_slots.has_value()) {
        std::cerr
            << "verify-scm-reintegration: cannot snapshot populated source slots for exact-preservation validation\n";
        return 9;
    }

    const auto source_child_sha = detail::sha256_of(
        std::span<const std::byte>{source_scm});
    const auto authored_child_sha = detail::sha256_of(
        std::span<const std::byte>{authored_scm});
    dmc3::AuthoredChildImage authored{
        .resource = source_child.payload.resource.id,
        .source_sha256 = source_child_sha,
        .output_sha256 = authored_child_sha,
        .revision = 1U,
        .writer_mode = "scm-provenance-bound-complete-child-image",
        .bytes = authored_scm,
    };
    const std::vector<dmc3::AuthoredChildImage> authored_children{
        std::move(authored)};
    const auto rebuilt = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        *parent, source_expansion, authored_children);
    if (!rebuilt.ok() || !rebuilt.receipt.has_value()) {
        std::cerr << "verify-scm-reintegration: canonical packed reflow failed";
        if (!rebuilt.detail.empty()) std::cerr << ": " << rebuilt.detail;
        std::cerr << '\n';
        return 10;
    }

    std::size_t target_alias_count = 0U;
    for (const auto& snapshot : *source_slots) {
        if (snapshot.populated &&
            snapshot.source_offset == source_child.entry.offset) {
            ++target_alias_count;
        }
    }

    detail::VerificationContext verification{
        .slot_index = *slot_index,
        .target_source_offset = source_child.entry.offset,
        .authored_scm = authored_scm,
        .source_slots = *source_slots,
        .expected_format = rebuilt.receipt->output_topology.format,
        .expected_slot_count = rebuilt.receipt->output_topology.declared_slot_count,
    };

    const auto validator = [verification](const std::filesystem::path& staged) {
        return detail::validate_staged_reintegration(staged, verification);
    };
    const auto publication = detail::core::publish_bytes_no_replace(
        output_path,
        std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()},
        validator,
        ".dmc-rengine-scm-reintegration.staging");
    if (!publication.ok()) {
        std::cerr
            << "verify-scm-reintegration: output publication failed ("
            << detail::core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) std::cerr << ": " << publication.detail;
        std::cerr << '\n';
        return 11;
    }

    std::cout
        << "SCM_REINTEGRATION_VERIFIED"
        << " format=" << rebuilt.receipt->output_topology.format
        << " slot=" << *slot_index
        << " targetAliases=" << target_alias_count
        << " sourceScmSha256=" << source_child_sha
        << " authoredScmSha256=" << authored_child_sha
        << " sourceParentSha256=" << rebuilt.receipt->source_sha256
        << " outputParentSha256=" << rebuilt.receipt->output_sha256
        << " sourceScmBytes=" << source_scm.size()
        << " authoredScmBytes=" << authored_scm.size()
        << " sourceParentBytes=" << rebuilt.receipt->source_topology.container_size
        << " outputParentBytes=" << rebuilt.receipt->output_topology.container_size
        << " targetAliasRematerialization=PASS"
        << " nonTargetPhysicalSpansExact=PASS"
        << " authoredScmReparse=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
