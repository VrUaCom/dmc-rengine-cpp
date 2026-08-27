#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

// Where a name shown for a slot came from.
//
// A relative-slot container stores no names, so every name a tool shows is
// something the tool decided. The difference between the two kinds below is
// the whole point: one is a placeholder we invented, the other is a string the
// container itself carries. Printing them identically is what makes a listing
// look authoritative when it is not.
enum class SlotNameOrigin : std::uint8_t {
    // `slot_0003` — an index, formatted. Says nothing about the payload.
    parser_placeholder,
    // The suffix was replaced with a type the payload's own bytes declare.
    byte_derived_suffix,
    // A line from a name list stored inside the container, attributed to this
    // slot by position. Never asserted as recovered truth.
    container_manifest,
    // The slot carries nothing. Distinct from a placeholder, because there is
    // no payload here to have a name for — a sparse container is intact, and a
    // reader that cannot tell this from "we did not know" will read the
    // sparseness as damage.
    absent_slot,
};

[[nodiscard]] constexpr std::string_view to_string(
    SlotNameOrigin origin) noexcept {
    switch (origin) {
    case SlotNameOrigin::parser_placeholder: return "parser-placeholder";
    case SlotNameOrigin::byte_derived_suffix: return "byte-derived-suffix";
    case SlotNameOrigin::container_manifest: return "container-manifest";
    case SlotNameOrigin::absent_slot: return "absent-slot";
    }
    return "parser-placeholder";
}

struct SlotNameAttribution final {
    std::uint32_t slot_index{};
    std::string name;
    SlotNameOrigin origin{SlotNameOrigin::parser_placeholder};
    // True when a type the payload independently declares agrees with the
    // name's own extension. That agreement is the only thing that separates a
    // manifest line that is probably right from one that is merely present.
    bool corroborated_by_payload{false};
};

// The CRLF name list some DMC3 containers store in their own slot 0.
//
// `st001.pac` slot 0 is exactly `st001.ptx\r\nst001.scm\r\nst001.sch\r\n`, and
// `st114.pac` carries the same three lines for its own stage. The apparent
// mapping is line *i* to slot *i + 1* — the manifest occupies slot 0, so the
// first name it lists belongs to the slot after it.
//
// This is the same convention the extracted corpus's external `.index` files
// use, where a container directive line occupies position 0 and names follow.
// Two conventions agreeing on the mechanism is why the mapping is recorded,
// and one sample of each is why it is attributed rather than asserted.
class SlotNameManifest final {
public:
    // Product-side bound. A real manifest is a handful of short lines.
    static constexpr std::size_t k_max_manifest_bytes = 4096U;
    static constexpr std::size_t k_max_name_bytes = 64U;

    // The slot the manifest itself occupies, and therefore the offset between
    // a line's position and the slot it names.
    static constexpr std::uint32_t k_manifest_slot = 0U;

    // Reads a manifest from a candidate slot-0 payload. Returns an empty list
    // for anything that is not one, including text that is text but not a
    // name list — a `# GAME` block is text and names nothing.
    [[nodiscard]] static std::vector<std::string> parse(
        std::span<const std::byte> slot_zero);

    // The slot a manifest line names.
    [[nodiscard]] static constexpr std::uint32_t slot_for_line(
        std::size_t line_index) noexcept {
        return k_manifest_slot + 1U + static_cast<std::uint32_t>(line_index);
    }

    // The extension a manifest line carries, lowercased, without the dot.
    // Empty when the line has none.
    [[nodiscard]] static std::string extension_of(std::string_view name);

    // Renders a sidecar manifest for an expanded container.
    //
    // The extracted corpus carries external `.index` files next to unpacked
    // containers: a directive line naming the container, then one line per
    // slot. This project has read thirteen of them and written none, so a
    // folder it unpacks loses everything it knew the moment it is closed.
    //
    // What is written here is that shape plus the one thing those files do not
    // record: where each name came from. A reader who cannot tell a
    // placeholder from a name the container itself carried is exactly the
    // reader this project keeps trying not to create.
    //
    // Format, CRLF-terminated to match the corpus:
    //
    //     PAC
    //     0<TAB>slot_0000.txt<TAB>byte-derived-suffix
    //     1<TAB>st001.ptx<TAB>container-manifest<TAB>payload-agrees
    //     2<TAB>slot_0002.empty<TAB>absent-slot
    //
    // The slot index leads every line, so a sparse container reads correctly:
    // an absent slot is a line that says so, never a line that is missing.
    [[nodiscard]] static std::string render_sidecar(
        std::string_view container_format,
        std::span<const SlotNameAttribution> attributions);

    static constexpr std::string_view k_sidecar_extension = ".index";
    static constexpr std::string_view k_corroborated = "payload-agrees";
};

} // namespace dmc::rengine::gdspaces
