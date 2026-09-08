#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

/**
 * The name a DMC3 HD extraction gives a payload.
 *
 * Recovered 2026-09-08 by reproducing a complete, independently produced
 * `em000` unpack — 302 payloads across three nesting levels — rather than by
 * reading any tool's source. Every path this renders is checked against that
 * archive in `tests/extraction_naming_tests.cpp`.
 *
 * The shape is positional, and the position is the *physical slot*:
 *
 *     depth 1   em000_002.clt
 *     depth 2   em000_pnst0035_000.mot
 *     depth 3   em000_pnst0041_pnst0001_012_V108.effect-v
 *
 * One `_pnst<parent slot>` segment per level descended, then the payload's own
 * index, then — where the container names its records — that record's name with
 * its separator removed.
 *
 * **The number is the physical slot, and empty slots leave gaps.** `em000`
 * declares 42 slots and fills 40; the archive holds `em000_024` and
 * `em000_026` with nothing between them. This is not in tension with the
 * recovered `.index` rule, and confusing the two is easy: a manifest *line* N
 * names the N-th populated payload, while an extracted *filename* carries the
 * slot. One is dense, the other is sparse, and they are only equal when the
 * container has no empty slots.
 *
 * At depths below the first the supplied corpus is dense throughout, so the
 * two readings coincide there and this makes no claim about which the deeper
 * numbers follow.
 */
struct ExtractionNameRequest final {
    /// The actor or stage stem — `em000`, taken from the container's own name.
    std::string stem;

    /**
     * The physical slot of each container descended into, outermost first.
     * Empty for a payload sitting directly in the opened container.
     */
    std::vector<std::uint32_t> parent_slots;

    /// This payload's own index within its immediate container.
    std::uint32_t index{};

    /// The extension, without a dot. Never a claim of an original filename.
    std::string extension;

    /**
     * The name the enclosing container stores for this record, if it does.
     *
     * An effect pack's manifest reads `V 108`; the extraction writes `V108`.
     * The space goes because a filename is not the place to preserve a
     * manifest's whitespace — the manifest is still the authority for what the
     * record is called.
     */
    std::optional<std::string> record_name;
};

class Dmc3ExtractionNaming final {
public:
    static constexpr std::size_t k_index_digits = 3U;
    static constexpr std::size_t k_parent_slot_digits = 4U;
    static constexpr std::string_view k_nesting_marker = "pnst";

    /** The leaf filename, e.g. `em000_pnst0041_pnst0001_012_V108.effect-v`. */
    [[nodiscard]] static std::string leaf(const ExtractionNameRequest& request);

    /**
     * The directory a container's children are written into, relative to the
     * archive root — `em000/em000_041.pnst/em000_pnst0041_001.pnst`.
     *
     * A container directory wears the same leaf name its payload would, so a
     * folder and the file it came from are recognizably the same resource.
     */
    [[nodiscard]] static std::string directory(
        std::string_view stem,
        const std::vector<std::uint32_t>& parent_slots,
        const std::vector<std::string>& parent_extensions);
};

} // namespace dmc::rengine::profiles::dmc3
