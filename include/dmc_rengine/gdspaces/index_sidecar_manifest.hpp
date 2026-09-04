#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

// The external `.index` file an unpacked container folder carries.
//
// Every unpacked folder has one. They are not the game's — the literal
// `.index` occurs zero times in the executable in any case, so the runtime
// cannot name such a file — they are whatever tool did the extraction writing
// down what it called each slot. That makes them the largest source of real
// slot names available anywhere, and this project has been showing
// `slot_0000` beside them.
//
// The shape is recorded in two independent places in this repository, both
// written against files that were actually read: a directive line naming the
// container format occupies line 0, and one plain name per slot follows.
//
//     PNST
//     em035_057_000.txt
//     ...
//
// Two rules make this safe to act on.
//
// First, it refuses our own sidecar. This project writes a file under the same
// extension whose lines carry tab-separated columns, and reading that back as
// authority would be reading back our own decision as evidence — the exact
// failure this project keeps having to undo. A line containing a tab is
// therefore not a name, and the whole file is refused.
//
// Second, a name here is attributed as `external-index`, never as something
// the container stored. The container stored nothing; a tool wrote this.
class IndexSidecarManifest final {
public:
    // Product-side bounds. A real one is a directive and a handful of lines.
    static constexpr std::size_t k_max_bytes = 64U * 1024U;
    static constexpr std::size_t k_max_names = 4096U;
    static constexpr std::size_t k_max_name_bytes = 260U;

    // The line that must open the file, and the mapping it implies: line 1
    // names slot 0, because line 0 is the directive.
    static constexpr std::size_t k_directive_line = 0U;

    [[nodiscard]] static constexpr std::uint32_t slot_for_line(
        std::size_t line_index) noexcept {
        return static_cast<std::uint32_t>(line_index - 1U);
    }

    struct Entry final {
        std::uint32_t slot_index{};
        std::string name;
        std::size_t source_line{};
    };

    struct Document final {
        std::string container_directive;
        std::vector<Entry> entries;
    };

    // Returns nothing for anything that is not this shape, including a sidecar
    // this project wrote itself.
    [[nodiscard]] static std::optional<Document> parse(
        std::span<const std::byte> bytes);

    // True where the bytes are a sidecar this project rendered, rather than an
    // extraction tool's. Offered separately so a caller can say which it
    // refused and why.
    [[nodiscard]] static bool is_own_rendered_sidecar(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::gdspaces
