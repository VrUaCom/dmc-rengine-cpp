#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::gdspaces {

// Which dialect of index a container carries in its first slot.
//
// A census of every text slot in every container of the corpus, at every
// nesting depth, finds 26 candidates and exactly four real ones. The rest are
// tagged binary records whose first bytes happen to be printable — `CAM`,
// `EVE`, `POS`, `HITS` — and a single-line read of those looks like text until
// you count the lines.
//
// The four real ones fall into two dialects, and both live in slot 0:
//
//   stage containers   3 lines, each a filename, naming slots 1..N
//                      st001.pac, st114.pac
//
//   effect containers  N lines of `<letter> <id>` closed by `# End`, naming
//                      the slots of the *sibling* container in slot 1
//                      st001_effect.pac, st114_effect.pac
//
// The distinction that matters is the last one: a stage index names its own
// siblings, and an effect index names the children of the slot beside it. A
// probe that returned "index found" without saying which would have a caller
// applying names one level off.
//
// A third category exists and must not be mistaken for either: slot-0 text
// that names nothing — `# END`, `# GAME`, `# DOOR 0`. Those are scene and
// config blocks, and they are text, and they are not indexes.
enum class ContainerIndexDialect : std::uint8_t {
    none,
    // Lines are filenames; line *i* names slot *i + 1* of this container.
    filename_list,
    // Lines are a kind letter and an identifier; they name the slots of the
    // container in the next slot, not of this one.
    kind_and_identifier,
};

[[nodiscard]] constexpr std::string_view to_string(
    ContainerIndexDialect dialect) noexcept {
    switch (dialect) {
    case ContainerIndexDialect::none: return "none";
    case ContainerIndexDialect::filename_list: return "filename-list";
    case ContainerIndexDialect::kind_and_identifier: return "kind-and-identifier";
    }
    return "none";
}

struct ContainerIndexProbeResult final {
    ContainerIndexDialect dialect{ContainerIndexDialect::none};
    // The slot the index itself occupies. Zero in both dialects, recorded
    // rather than assumed so a third dialect elsewhere cannot silently inherit
    // the assumption.
    std::uint32_t index_slot_index{};
    // How many slots the index names.
    std::uint32_t named_slot_count{};
    // Whether those slots belong to this container or to a sibling.
    bool names_a_sibling_container{false};
    // The sibling it names, meaningful only when the flag above is set.
    std::uint32_t named_sibling_slot_index{};

    [[nodiscard]] bool found() const noexcept {
        return dialect != ContainerIndexDialect::none;
    }
};

// Ask any container whether it carries an index, and of which kind.
//
// One entry point on purpose. Two dialects were being detected in two
// unrelated places under two sets of rules, so nothing could answer "does this
// archive have an index" without knowing in advance which kind to look for.
class ContainerIndexProbe final {
public:
    [[nodiscard]] static ContainerIndexProbeResult probe(
        std::span<const std::byte> container_bytes);
};

} // namespace dmc::rengine::gdspaces
