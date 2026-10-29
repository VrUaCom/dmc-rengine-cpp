// DMC3 light pipeline (profiles/dmc3/light_pipeline.hpp): reading LIG2 tables, the candidate rules
// per list and mask, the score, the top-four order, the lc / lv matrices, the
// named header fields, the model's light rig and the stage's light rig.
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>

#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/profiles/dmc3/light_pipeline.hpp"

using namespace dmc::rengine::profiles::dmc3::lights;

namespace {

struct RecordSpec {
    std::uint8_t flags{};
    std::uint8_t colour_modifier{};
    std::uint8_t type{};
    std::array<float, 3> position{};
    float aux_w{};
    float falloff{};
    std::array<std::uint16_t, 3> rgb{};
};

void put_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFF);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8);
}

void put_f32(std::vector<std::uint8_t>& bytes, std::size_t offset, float value) {
    std::uint32_t bits{};
    std::memcpy(&bits, &value, sizeof bits);
    for (std::size_t i = 0; i < 4; ++i) bytes[offset + i] = static_cast<std::uint8_t>(bits >> (8 * i));
}

// Writes a LIG2 table: header, then 48 records of 0x30 bytes (all zero unless set).
struct TableBytes {
    std::vector<std::uint8_t> bytes = std::vector<std::uint8_t>(kTableSize, 0);

    explicit TableBytes(std::uint16_t version = 3) {
        bytes[0] = 'L';
        bytes[1] = 'I';
        bytes[2] = 'G';
        bytes[3] = '2';
        put_u16(bytes, 0x04, version);
    }

    TableBytes& set(std::size_t index, const RecordSpec& spec) {
        const std::size_t base = kHeaderSize + index * kRecordSize;
        bytes[base + 0x00] = spec.flags;
        bytes[base + 0x01] = spec.colour_modifier;
        bytes[base + 0x02] = spec.type;
        put_f32(bytes, base + 0x10, spec.position[0]);
        put_f32(bytes, base + 0x14, spec.position[1]);
        put_f32(bytes, base + 0x18, spec.position[2]);
        put_f32(bytes, base + 0x1C, spec.aux_w);
        put_f32(bytes, base + 0x20, spec.falloff);
        put_u16(bytes, base + 0x24, spec.rgb[0]);
        put_u16(bytes, base + 0x26, spec.rgb[1]);
        put_u16(bytes, base + 0x28, spec.rgb[2]);
        return *this;
    }

    [[nodiscard]] LightTable read() const {
        auto table = read_light_table(bytes);
        assert(table.has_value());
        return *table;
    }
};

bool near(float a, float b, float eps = 1e-6f) { return std::fabs(a - b) <= eps; }

// --- reading -----------------------------------------------------------------

void test_read_fields() {
    TableBytes t(7);
    for (std::size_t i = 0; i < 6; ++i) put_u16(t.bytes, 0x06 + i * 2, static_cast<std::uint16_t>(0x100 + i));
    t.set(4, {.flags = 0x05, .colour_modifier = 2, .type = 1, .position = {1.5f, -2.0f, 3.25f},
              .aux_w = 4.0f, .falloff = 0.125f, .rgb = {10, 20, 0xFFFF}});
    const LightTable table = t.read();
    assert(table.version == 7);
    for (std::size_t i = 0; i < 6; ++i) assert(table.header_words[i] == 0x100 + i);
    for (std::size_t i = 0; i < kRecordCount; ++i) {
        assert(table.records[i].index == i);
        assert(table.records[i].active == (i == 4));
    }
    const LightRecord& r = table.records[4];
    assert(r.flags == 0x05 && r.colour_modifier == 2 && r.type == 1);
    assert(r.position[0] == 1.5f && r.position[1] == -2.0f && r.position[2] == 3.25f);
    assert(r.aux_w == 4.0f && r.falloff == 0.125f);
    assert(r.rgb[0] == 10 && r.rgb[1] == 20 && r.rgb[2] == 0xFFFF);
}

void test_read_size_and_magic() {
    TableBytes t;
    assert(read_light_table(t.bytes).has_value());

    std::vector<std::uint8_t> shorter(t.bytes.begin(), t.bytes.end() - 1);
    assert(!read_light_table(shorter).has_value());
    assert(!read_light_table(std::span<const std::uint8_t>{}).has_value());

    std::vector<std::uint8_t> longer = t.bytes;
    longer.resize(kTableSize + 100, 0xAB);
    assert(read_light_table(longer).has_value());

    for (std::size_t i = 0; i < 4; ++i) {
        TableBytes bad;
        bad.bytes[i] ^= 0x20;
        assert(!read_light_table(bad.bytes).has_value());
    }
}

void test_read_non_finite() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    const RecordSpec ok{.flags = 1, .type = 0, .position = {1, 2, 3}, .aux_w = 1, .falloff = 0.1f};

    for (std::size_t field = 0; field < 5; ++field) {
        for (float bad : {nan, inf, -inf}) {
            RecordSpec spec = ok;
            if (field < 3) spec.position[field] = bad;
            if (field == 3) spec.aux_w = bad;
            if (field == 4) spec.falloff = bad;
            TableBytes t;
            t.set(10, spec);
            assert(!read_light_table(t.bytes).has_value());
        }
    }

    // A non-finite float has non-zero bytes, so its record is active: an
    // inactive (all zero) record only holds zeros, which are finite.
    TableBytes only_nan;
    only_nan.set(20, {.position = {nan, 0, 0}});
    assert(!read_light_table(only_nan.bytes).has_value());

    const LightTable empty = TableBytes().read();
    for (const LightRecord& r : empty.records) {
        assert(!r.active);
        assert(r.position[0] == 0 && r.position[1] == 0 && r.position[2] == 0);
        assert(r.aux_w == 0 && r.falloff == 0);
    }
}

void test_find_light_table() {
    TableBytes t(5);
    t.set(1, {.flags = 3, .type = 1, .rgb = {1, 2, 3}});
    std::vector<std::uint8_t> container(0x40, 0x11);
    container.insert(container.end(), t.bytes.begin(), t.bytes.end());
    container.resize(container.size() + 0x30, 0x22);

    const auto found = find_light_table(container);
    assert(found.has_value());
    assert(found->version == 5);
    assert(found->records[1].active && found->records[1].rgb[2] == 3);

    std::vector<std::uint8_t> none(0x1000, 0x11);
    assert(!find_light_table(none).has_value());
    assert(!find_light_table(std::span<const std::uint8_t>{}).has_value());
}

// --- eligibility -------------------------------------------------------------

constexpr std::array<unsigned, 8> kMasks{1, 2, 3, 4, 5, 6, 7, 11};

// Selected for (flags, list) on a type 1 light, per mask in kMasks (each with
// `extra` added, e.g. 0x10).
std::array<bool, 8> selected_per_mask(std::uint8_t flags, ListKind list, std::uint8_t type = 1,
                                      std::uint8_t modifier = 0, unsigned extra = 0) {
    TableBytes t;
    t.set(0, {.flags = flags, .colour_modifier = modifier, .type = type, .position = {0, 0, 0},
              .falloff = 0.0f, .rgb = {1, 1, 1}});
    const LightTable table = t.read();
    std::array<bool, 8> out{};
    for (std::size_t i = 0; i < kMasks.size(); ++i) {
        const unsigned mask = kMasks[i] | extra;
        const auto sel = select_lights(table, {0, 0, 0}, mask, list);
        const std::size_t count = candidate_count(table, mask, list);
        assert(count == static_cast<std::size_t>(sel.count));
        out[i] = sel.count == 1;
        if (out[i]) assert(sel.index[0] == 0);
    }
    return out;
}

using Row = std::array<bool, 8>;
constexpr bool o = true, x = false;

void test_eligibility_main() {
    //                       masks: 1  2  3  4  5  6  7  11
    assert(selected_per_mask(0, ListKind::main) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(1, ListKind::main) == (Row{x, x, x, x, o, o, o, x}));
    assert(selected_per_mask(2, ListKind::main) == (Row{x, x, x, x, x, o, o, x}));
    assert(selected_per_mask(3, ListKind::main) == (Row{x, x, x, x, o, x, o, x}));
    assert(selected_per_mask(8, ListKind::main) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(0x10, ListKind::main) == (Row{x, x, x, x, x, x, x, x}));
    // Without mask bit 0x10, bit 2 adds nothing: 4 is 0, 5 is 1, 6 is 2, 7 is 3.
    for (std::uint8_t low = 0; low < 4; ++low) {
        assert(selected_per_mask(low | 4, ListKind::main) == selected_per_mask(low, ListKind::main));
    }
    // Bit 3 is only cleared for the secondary list.
    assert(selected_per_mask(0x0B, ListKind::main) == (Row{x, x, x, x, x, x, x, x}));
}

void test_eligibility_main_0x10() {
    constexpr unsigned e = 0x10;
    //                    masks + 0x10: 1  2  3  4  5  6  7  11
    assert(selected_per_mask(0, ListKind::main, 1, 0, e) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(1, ListKind::main, 1, 0, e) == (Row{x, x, x, x, o, o, o, x}));
    assert(selected_per_mask(2, ListKind::main, 1, 0, e) == (Row{x, x, x, x, x, o, o, x}));
    assert(selected_per_mask(3, ListKind::main, 1, 0, e) == (Row{x, x, x, x, o, x, o, x}));
    // Flags 4 (low 0) pass only with mask bit 0x10 (and the main bit 4).
    assert(selected_per_mask(4, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
    assert(selected_per_mask(5, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
    assert(selected_per_mask(6, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
    // Flags 7 pass where 3 passes, and also with 0x10.
    assert(selected_per_mask(7, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
    for (std::size_t i = 0; i < kMasks.size(); ++i) {
        if (selected_per_mask(3, ListKind::main)[i]) assert(selected_per_mask(7, ListKind::main)[i]);
    }
    // Bit 4 of the flags (0x10) is not bit 2: low 0x10 matches no mode.
    assert(selected_per_mask(0x10, ListKind::main, 1, 0, e) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(0x14, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
    // Bit 3 is not cleared on main: 0x0B is low 0x0B, 0x0C still has bit 2.
    assert(selected_per_mask(0x0B, ListKind::main, 1, 0, e) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(0x0C, ListKind::main, 1, 0, e) == (Row{x, x, x, o, o, o, o, x}));
}

void test_eligibility_secondary() {
    //                       masks: 1  2  3  4  5  6  7  11
    assert(selected_per_mask(0, ListKind::secondary) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(1, ListKind::secondary) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(2, ListKind::secondary) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(3, ListKind::secondary) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(7, ListKind::secondary) == selected_per_mask(3, ListKind::secondary));
    assert(selected_per_mask(0x0B, ListKind::secondary) == selected_per_mask(3, ListKind::secondary));
    assert(selected_per_mask(4, ListKind::secondary) == (Row{x, x, x, x, x, x, x, x}));
    // 0x08 never passes: bit 3 is cleared and bit 2 is not set.
    assert(selected_per_mask(0x08, ListKind::secondary) == (Row{x, x, x, x, x, x, x, x}));

    constexpr unsigned e = 0x10;
    //                    masks + 0x10: 1  2  3  4  5  6  7  11
    assert(selected_per_mask(0, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(1, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(2, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(3, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(4, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(7, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));
    assert(selected_per_mask(0x0B, ListKind::secondary, 1, 0, e) ==
           selected_per_mask(3, ListKind::secondary, 1, 0, e));
    assert(selected_per_mask(0x08, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, x}));
    assert(selected_per_mask(0x0C, ListKind::secondary, 1, 0, e) == (Row{x, x, x, x, x, x, x, o}));

    // Beyond kMasks: the low mode still decides under the secondary bit.
    TableBytes t;
    t.set(0, {.flags = 0x0B, .type = 1});  // like 3: needs mask & 1
    t.set(1, {.flags = 0x02, .type = 1});  // needs mask & 2
    t.set(2, {.flags = 0x01, .type = 1});  // needs mask & 3
    const LightTable table = t.read();
    assert(candidate_count(table, 8 | 1, ListKind::secondary) == 2);   // 0 and 2
    assert(candidate_count(table, 8 | 2, ListKind::secondary) == 2);   // 1 and 2
    assert(candidate_count(table, 8, ListKind::secondary) == 0);
    assert(candidate_count(table, 4 | 3, ListKind::secondary) == 0);   // no bit 8
    assert(candidate_count(table, 8 | 3, ListKind::main) == 0);        // no bit 4
    assert(candidate_count(table, 4 | 3, ListKind::main) == 2);        // 0x0B never on main

    // Flags 4 needs mask bit 0x10 and still the list bit.
    TableBytes four;
    four.set(0, {.flags = 0x04, .type = 1});
    const LightTable ft = four.read();
    assert(candidate_count(ft, 8 | 3, ListKind::secondary) == 0);
    assert(candidate_count(ft, 8 | 0x10, ListKind::secondary) == 1);
    assert(candidate_count(ft, 4 | 0x10, ListKind::secondary) == 0);
    assert(candidate_count(ft, 4 | 0x10, ListKind::main) == 1);
    assert(candidate_count(ft, 8 | 0x10, ListKind::main) == 0);
    assert(candidate_count(ft, 0x10, ListKind::main) == 0);
}

void test_never_selected() {
    const Row none{x, x, x, x, x, x, x, x};
    for (const int f : {1, 2, 3, 4, 7}) {
        const auto flags = static_cast<std::uint8_t>(f);
        assert(selected_per_mask(flags, ListKind::main, 2, 0, 0x10) == none);
        assert(selected_per_mask(flags, ListKind::secondary, 1, 1, 0x10) == none);
        assert(selected_per_mask(flags, ListKind::main, 2) == none);
        assert(selected_per_mask(flags, ListKind::secondary, 2) == none);
        assert(selected_per_mask(flags, ListKind::main, 1, 1) == none);
        assert(selected_per_mask(flags, ListKind::secondary, 0x80) == none);
        assert(selected_per_mask(flags, ListKind::main, 1, 0x80) == none);
    }
    // Type 0 is a candidate like type 1.
    TableBytes t;
    t.set(0, {.flags = 3, .type = 0, .falloff = 0.0f});
    assert(candidate_count(t.read(), 7, ListKind::main) == 1);
}

// --- score -------------------------------------------------------------------

void test_score() {
    TableBytes t;
    t.set(0, {.flags = 3, .type = 0, .position = {10, 0, 0}, .falloff = 0.005f});
    LightTable table = t.read();
    auto sel = select_lights(table, {0, 0, 0}, 5);
    assert(sel.count == 1 && sel.index[0] == 0);
    assert(near(sel.score[0], 0.5f));
    assert(near(score(table.records[0], {0, 0, 0}), 0.5f));

    // Distance from the query, not the origin: delta (6, -8, 0) is 10 long.
    sel = select_lights(table, {4, 8, 0}, 5);
    assert(sel.count == 1 && near(sel.score[0], 0.5f));

    TableBytes far;
    far.set(0, {.flags = 3, .type = 0, .position = {20, 0, 0}, .falloff = 0.005f});
    table = far.read();
    assert(select_lights(table, {0, 0, 0}, 5).count == 0);
    assert(score(table.records[0], {0, 0, 0}) == 0.0f);
    assert(candidate_count(table, 5, ListKind::main) == 1);  // score not applied

    TableBytes edge;  // exactly 0 is dropped too
    edge.set(0, {.flags = 3, .type = 0, .position = {2, 0, 0}, .falloff = 0.25f});
    assert(select_lights(edge.read(), {0, 0, 0}, 5).count == 0);

    TableBytes directional;
    directional.set(0, {.flags = 3, .type = 1, .position = {1000, 0, 0}, .falloff = 1.0f});
    sel = select_lights(directional.read(), {0, 0, 0}, 5);
    assert(sel.count == 1 && sel.score[0] == 1.0f);
}

// --- ordering ----------------------------------------------------------------

void test_ordering() {
    // At distance 1 the score is 1 - falloff.
    TableBytes t;
    t.set(3, {.flags = 3, .type = 0, .position = {1, 0, 0}, .falloff = 0.4f});   // 0.6
    t.set(7, {.flags = 3, .type = 0, .position = {0, 1, 0}, .falloff = 0.1f});   // 0.9
    t.set(12, {.flags = 3, .type = 0, .position = {0, 0, 1}, .falloff = 0.5f});  // 0.5
    t.set(30, {.flags = 3, .type = 0, .position = {-1, 0, 0}, .falloff = 0.3f}); // 0.7
    t.set(41, {.flags = 3, .type = 0, .position = {0, -1, 0}, .falloff = 0.2f}); // 0.8
    const auto sel = select_lights(t.read(), {0, 0, 0}, 5);
    assert(sel.count == 4);
    assert((sel.index == std::array<int, 4>{7, 41, 30, 3}));
    assert(near(sel.score[0], 0.9f) && near(sel.score[1], 0.8f));
    assert(near(sel.score[2], 0.7f) && near(sel.score[3], 0.6f));
    for (int i = 0; i < 3; ++i) assert(sel.score[i] > sel.score[i + 1]);

    // Two equal scores: the later index goes first.
    TableBytes two;
    two.set(8, {.flags = 3, .type = 0, .position = {1, 0, 0}, .falloff = 0.25f});
    two.set(20, {.flags = 3, .type = 0, .position = {0, 0, -1}, .falloff = 0.25f});
    const auto pair = select_lights(two.read(), {0, 0, 0}, 5);
    assert(pair.count == 2 && pair.index[0] == 20 && pair.index[1] == 8);
    assert(pair.score[0] == 0.75f && pair.score[1] == 0.75f);

    // Five equal scores at 0..4: the first four visited are kept, latest
    // first; index 4 does not beat slot 3 and is skipped.
    TableBytes eq;
    for (std::size_t i = 0; i < 5; ++i) eq.set(i, {.flags = 3, .type = 1});
    const auto tie = select_lights(eq.read(), {0, 0, 0}, 5);
    assert(tie.count == 4);
    assert((tie.index == std::array<int, 4>{3, 2, 1, 0}));

    // Spread over the table the same holds: the four lowest indices, reversed.
    TableBytes spread;
    for (std::size_t i : {2u, 9u, 17u, 25u, 47u}) spread.set(i, {.flags = 3, .type = 1});
    const auto st = select_lights(spread.read(), {0, 0, 0}, 5);
    assert((st.index == std::array<int, 4>{25, 17, 9, 2}));

    // A type 0 at score 1 (falloff 0) ties with type 1 by index too.
    TableBytes mixed;
    mixed.set(5, {.flags = 3, .type = 1});
    mixed.set(6, {.flags = 3, .type = 0, .falloff = 0.0f, .rgb = {1, 0, 0}});
    const auto m = select_lights(mixed.read(), {0, 0, 0}, 5);
    assert(m.count == 2 && m.index[0] == 6 && m.index[1] == 5);
}

void test_ordering_threshold() {
    // Four kept lights (0.9, 0.8, 0.7, 0.6); a later 0.6 equals slot 3 and is
    // skipped, a later 0.65 is above it and replaces slot 3.
    const auto light = [](float falloff) {
        return RecordSpec{.flags = 3, .type = 0, .position = {1, 0, 0}, .falloff = falloff};
    };
    TableBytes t;
    t.set(0, light(0.1f));
    t.set(1, light(0.2f));
    t.set(2, light(0.3f));
    t.set(3, light(0.4f));
    t.set(4, light(0.4f));  // equal to slot 3
    auto sel = select_lights(t.read(), {0, 0, 0}, 5);
    assert(sel.count == 4);
    assert((sel.index == std::array<int, 4>{0, 1, 2, 3}));

    t.set(5, light(0.35f));  // 0.65 > 0.6
    sel = select_lights(t.read(), {0, 0, 0}, 5);
    assert(sel.count == 4);
    assert((sel.index == std::array<int, 4>{0, 1, 2, 5}));
    assert(near(sel.score[3], 0.65f));

    // Below slot 3 never enters; a better late light rises to its place.
    t.set(6, light(0.5f));   // 0.5
    t.set(7, light(0.05f));  // 0.95
    sel = select_lights(t.read(), {0, 0, 0}, 5);
    assert((sel.index == std::array<int, 4>{7, 0, 1, 2}));
}

// --- single light ------------------------------------------------------------

void test_single_light() {
    const auto light = [](float falloff) {
        return RecordSpec{.flags = 3, .type = 0, .position = {1, 0, 0}, .falloff = falloff};
    };
    TableBytes t;
    t.set(0, light(0.1f));  // 0.9
    t.set(1, light(0.2f));  // 0.8
    t.set(2, light(0.3f));  // 0.7
    t.set(3, light(0.4f));  // 0.6
    const LightTable table = t.read();

    // Not a candidate itself (flags 0, a colour modifier): no eligibility test.
    LightRecord single{};
    single.index = 40;
    single.active = true;
    single.flags = 0;
    single.colour_modifier = 1;
    single.type = 0;
    single.position = {0, 3, 0};
    single.aux_w = 2.0f;
    single.falloff = 0.1f;  // delta (0, 3, 0): 1 - 9 * 0.1 = 0.1
    single.rgb = {7, 8, 9};

    const auto base = select_lights(table, {0, 0, 0}, 5);
    assert((base.index == std::array<int, 4>{0, 1, 2, 3}));

    // mask & 2: slot 0 even with the lowest score; the old slot 3 is dropped.
    const auto with = select_lights(table, {0, 0, 0}, 5 | 2, ListKind::main, &single);
    assert(with.count == 4);
    assert((with.index == std::array<int, 4>{40, 0, 1, 2}));
    assert(near(with.score[0], 0.1f));
    assert(near(with.score[1], 0.9f) && near(with.score[2], 0.8f) && near(with.score[3], 0.7f));
    assert(with.lc[0] == 7 && with.lc[1] == 8 && with.lc[2] == 9 && with.lc[3] == 0.1f);
    assert(with.lv[0] == 0 && with.lv[1] == 3 && with.lv[2] == 0 && with.lv[3] == 2.0f);
    for (std::size_t k = 0; k < 12; ++k) {
        assert(with.lc[4 + k] == base.lc[k]);
        assert(with.lv[4 + k] == base.lv[k]);
    }

    // Placed whatever its score, even 0, and counted.
    LightRecord dark = single;
    dark.falloff = 1.0f;  // 1 - 9 < 0 -> 0
    const auto zero = select_lights(table, {0, 0, 0}, 5 | 2, ListKind::main, &dark);
    assert(zero.count == 4 && zero.index[0] == 40 && zero.score[0] == 0.0f);

    // With fewer lights nothing is dropped and the count grows.
    const auto alone = select_lights(TableBytes().read(), {0, 0, 0}, 2, ListKind::main, &single);
    assert(alone.count == 1);
    assert((alone.index == std::array<int, 4>{40, -1, -1, -1}));
    for (std::size_t k = 4; k < 16; ++k) assert(alone.lc[k] == 0.0f && alone.lv[k] == 0.0f);

    // Type 1 single: score 1, lc w 1, lv w 0.
    LightRecord sun = single;
    sun.type = 1;
    sun.index = 41;
    const auto s1 = select_lights(table, {0, 0, 0}, 5 | 2, ListKind::secondary, &sun);
    assert(s1.count == 1 && s1.index[0] == 41 && s1.score[0] == 1.0f);  // secondary: no bit 8
    assert(s1.lc[3] == 1.0f && s1.lv[3] == 0.0f);

    // Without mask & 2, or when null, nothing changes.
    const auto without = select_lights(table, {0, 0, 0}, 5, ListKind::main, &single);
    assert(without.count == base.count && without.index == base.index && without.score == base.score);
    assert(without.lc == base.lc && without.lv == base.lv);
    const auto with_mask = select_lights(table, {0, 0, 0}, 5 | 2);
    const auto null = select_lights(table, {0, 0, 0}, 5 | 2, ListKind::main, nullptr);
    assert(null.count == with_mask.count && null.index == with_mask.index);
    assert(null.lc == with_mask.lc && null.lv == with_mask.lv);
    assert(with_mask.index == base.index);
}

// --- matrices ----------------------------------------------------------------

void test_matrices() {
    TableBytes t;
    t.set(5, {.flags = 3, .type = 0, .position = {1, 2, 3}, .aux_w = 7.0f, .falloff = 0.01f,
              .rgb = {100, 200, 300}});
    t.set(9, {.flags = 3, .type = 1, .position = {4, 5, 6}, .aux_w = 8.0f, .falloff = 0.5f,
              .rgb = {10, 20, 30}});
    const auto sel = select_lights(t.read(), {1, 1, 1}, 5);
    assert(sel.count == 2);
    // Type 1 scores 1; type 0: delta (0, 1, 2), 1 - 5 * 0.01 = 0.95.
    assert(sel.index[0] == 9 && sel.index[1] == 5);
    assert(sel.index[2] == -1 && sel.index[3] == -1);
    assert(sel.score[0] == 1.0f && near(sel.score[1], 0.95f));
    assert(sel.score[2] == 0.0f && sel.score[3] == 0.0f);

    const std::array<float, 16> lc{10, 20, 30, 1,  100, 200, 300, 0.01f,  0, 0, 0, 0,  0, 0, 0, 0};
    const std::array<float, 16> lv{3, 4, 5, 0,  0, 1, 2, 7,  0, 0, 0, 0,  0, 0, 0, 0};
    assert(sel.lc == lc);
    assert(sel.lv == lv);

    const auto empty = select_lights(t.read(), {0, 0, 0}, 0);
    assert(empty.count == 0);
    assert((empty.index == std::array<int, 4>{-1, -1, -1, -1}));
    for (float v : empty.lc) assert(v == 0.0f);
    for (float v : empty.lv) assert(v == 0.0f);
}

// --- header fields (revision 3) ----------------------------------------------

void test_header_fields() {
    assert(TableBytes(3).read().static_light_count == 3);
    assert(TableBytes(0x00FF).read().static_light_count == -1);
    assert(TableBytes(0x0380).read().static_light_count == -128);  // low byte only
    const LightTable v = TableBytes(0x0380).read();
    assert(v.version == 0x0380);  // version keeps the whole u16

    TableBytes t(3);
    put_u16(t.bytes, 0x06, 32);
    put_u16(t.bytes, 0x08, 33);
    put_u16(t.bytes, 0x0A, 34);
    put_u16(t.bytes, 0x0C, 220);
    put_u16(t.bytes, 0x0E, 210);
    put_u16(t.bytes, 0x10, 200);
    const LightTable table = t.read();
    assert((table.ambient == std::array<float, 3>{220.0f, 210.0f, 200.0f}));
    assert((table.ambient_mask2 == std::array<float, 3>{32.0f, 33.0f, 34.0f}));
    // header_words are kept as before.
    assert((table.header_words == std::array<std::uint16_t, 6>{32, 33, 34, 220, 210, 200}));

    TableBytes big;
    put_u16(big.bytes, 0x0C, 0xFFFF);
    assert(big.read().ambient[0] == 65535.0f);  // no scaling
    assert((TableBytes().read().ambient == std::array<float, 3>{}));
}

// --- model rig (revision 3) --------------------------------------------------

// A SelectedLights with `count` lights whose rows are given directly.
SelectedLights rows(int count, const std::array<std::array<float, 4>, 4>& lc,
                    const std::array<std::array<float, 4>, 4>& lv) {
    SelectedLights s{};
    s.count = count;
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t k = 0; k < 4; ++k) {
            s.lc[i * 4 + k] = lc[i][k];
            s.lv[i * 4 + k] = lv[i][k];
        }
        if (static_cast<int>(i) < count) s.index[i] = static_cast<int>(i);
    }
    return s;
}

float length(const std::array<float, 3>& v) { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }

void test_model_rig_count() {
    const LightTable table = TableBytes().read();
    const std::array<std::array<float, 4>, 4> lc{{{1, 1, 1, 0}, {2, 2, 2, 0}, {3, 3, 3, 0}, {4, 4, 4, 0}}};
    const std::array<std::array<float, 4>, 4> lv{{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 1, 0, 0}}};

    // Limited by lights.count.
    for (int c = 0; c <= 4; ++c) assert(model_rig(table, rows(c, lc, lv), 7).count == std::min(c, 3));
    // Slot 3 is never used.
    const ModelRig four = model_rig(table, rows(4, lc, lv), 7, 10);
    assert(four.count == 3);
    assert((four.colour[2] == std::array<float, 3>{3, 3, 3}));
    // Limited by limit; 0 or below gives 0.
    assert(model_rig(table, rows(4, lc, lv), 7, 2).count == 2);
    assert(model_rig(table, rows(4, lc, lv), 7, 1).count == 1);
    assert(model_rig(table, rows(1, lc, lv), 7, 2).count == 1);
    for (int limit : {0, -1, -100}) {
        const ModelRig none = model_rig(table, rows(4, lc, lv), 7, limit);
        assert(none.count == 0);
        for (std::size_t i = 0; i < 3; ++i) {
            assert((none.colour[i] == std::array<float, 3>{}));
            assert((none.direction[i] == std::array<float, 3>{}));
        }
    }
    // Unused entries stay zero.
    const ModelRig two = model_rig(table, rows(4, lc, lv), 7, 2);
    assert((two.colour[1] == std::array<float, 3>{2, 2, 2}));
    assert((two.colour[2] == std::array<float, 3>{}));
    assert((two.direction[2] == std::array<float, 3>{}));
}

void test_model_rig_attenuation() {
    const LightTable table = TableBytes().read();
    const std::array<float, 4> c{100, 200, 50, 0.25f};
    // lv[3] = 0 keeps the full colour; 0.005 at distance 10 halves it; at
    // distance 20 it gives 0 (clamped, not negative).
    const std::array<std::array<float, 4>, 4> lc{{c, c, c, {}}};
    const std::array<std::array<float, 4>, 4> lv{{{10, 0, 0, 0}, {6, 8, 0, 0.005f}, {0, 0, -20, 0.005f}, {}}};
    const ModelRig rig = model_rig(table, rows(3, lc, lv), 7);
    assert(rig.count == 3);
    assert((rig.colour[0] == std::array<float, 3>{100, 200, 50}));
    assert(near(rig.colour[1][0], 50) && near(rig.colour[1][1], 100) && near(rig.colour[1][2], 25));
    assert(rig.colour[2][0] == 0.0f && rig.colour[2][1] == 0.0f && rig.colour[2][2] == 0.0f);
    assert(!std::signbit(rig.colour[2][0]));

    // The colour ignores the score, and lc[3] is not used.
    SelectedLights s = rows(1, {{{40, 50, 60, 0.9f}, {}, {}, {}}}, {{{0, 3, 4, 0}, {}, {}, {}}});
    s.score[0] = 0.1f;
    const ModelRig a = model_rig(table, s, 7);
    s.score[0] = 1.0f;
    s.lc[3] = 123.0f;
    const ModelRig b = model_rig(table, s, 7);
    assert((a.colour[0] == std::array<float, 3>{40, 50, 60}));
    assert(a.colour == b.colour);

    // Through select_lights: a type 0 light keeps aux_w in lv[3], which attenuates.
    TableBytes t;
    t.set(0, {.flags = 3, .type = 0, .position = {10, 0, 0}, .aux_w = 0.005f, .falloff = 0.001f,
              .rgb = {200, 100, 40}});
    t.set(1, {.flags = 3, .type = 1, .position = {0, 0, 7}, .aux_w = 5.0f, .falloff = 1.0f,
              .rgb = {128, 128, 128}});
    const auto sel = select_lights(t.read(), {0, 0, 0}, 5);
    assert(sel.count == 2 && sel.index[0] == 1 && sel.index[1] == 0);
    const ModelRig r = model_rig(t.read(), sel, 5);
    assert(r.count == 2);
    assert((r.colour[0] == std::array<float, 3>{128, 128, 128}));  // type 1: lv[3] = 0
    assert(near(r.colour[1][0], 100, 1e-4f) && near(r.colour[1][1], 50, 1e-4f) &&
           near(r.colour[1][2], 20, 1e-4f));
    assert((r.direction[0] == std::array<float, 3>{0, 0, 1}));
    assert((r.direction[1] == std::array<float, 3>{1, 0, 0}));
}

void test_model_rig_direction() {
    const LightTable table = TableBytes().read();
    const std::array<float, 4> c{1, 1, 1, 0};
    const std::array<std::array<float, 4>, 4> lc{{c, c, c, c}};
    const std::array<std::array<float, 4>, 4> lv{{{3, 4, 0, 0}, {-1, 2, -2, 0.001f}, {0, 0, 0, 0.5f}, {}}};
    const ModelRig rig = model_rig(table, rows(3, lc, lv), 7);
    assert(near(rig.direction[0][0], 0.6f) && near(rig.direction[0][1], 0.8f) && rig.direction[0][2] == 0.0f);
    assert(near(rig.direction[1][0], -1.0f / 3) && near(rig.direction[1][1], 2.0f / 3) &&
           near(rig.direction[1][2], -2.0f / 3));
    assert(near(length(rig.direction[0]), 1.0f) && near(length(rig.direction[1]), 1.0f));
    // A zero lv gives a zero direction (and attenuation 1 at distance 0).
    assert((rig.direction[2] == std::array<float, 3>{}));
    assert((rig.colour[2] == std::array<float, 3>{1, 1, 1}));
}

void test_model_rig_ambient() {
    TableBytes t;
    put_u16(t.bytes, 0x06, 32);
    put_u16(t.bytes, 0x08, 32);
    put_u16(t.bytes, 0x0A, 32);
    put_u16(t.bytes, 0x0C, 220);
    put_u16(t.bytes, 0x0E, 210);
    put_u16(t.bytes, 0x10, 200);
    const LightTable table = t.read();
    const SelectedLights none{};
    const std::array<float, 3> normal{220, 210, 200}, mask2{32, 32, 32};
    assert(model_rig(table, none, 7).ambient == normal);
    assert(model_rig(table, none, 0).ambient == normal);
    assert(model_rig(table, none, 2).ambient == mask2);
    assert(model_rig(table, none, 2 | 4).ambient == normal);   // exactly 2 only
    assert(model_rig(table, none, 2, 0).ambient == mask2);     // independent of count
    assert(model_rig(table, none, 7).count == 0);
}

// --- stage rig (revision 4) --------------------------------------------------

const float kK = std::bit_cast<float>(std::uint32_t{0x3B808081u});  // about 1/255

void test_stage_rig_count() {
    const LightTable table = TableBytes().read();
    const std::array<std::array<float, 4>, 4> lc{{{1, 1, 1, 0}, {2, 2, 2, 0}, {3, 3, 3, 0}, {4, 4, 4, 0}}};
    const std::array<std::array<float, 4>, 4> lv{{{1, 0, 0, 1}, {0, 1, 0, 2}, {0, 0, 1, 3}, {1, 1, 0, 4}}};
    const std::array<float, 3> q{0, 0, 0};

    // Limited by lights.count, and by 4 (slot 3 is used, unlike the model rig).
    for (int c = 0; c <= 4; ++c) assert(stage_rig(table, rows(c, lc, lv), q, 7).count == c);
    const StageRig four = stage_rig(table, rows(4, lc, lv), q, 7, 10);
    assert(four.count == 4);
    assert((four.position[3] == std::array<float, 3>{1, 1, 0}) && four.falloff[3] == 4.0f);
    assert(four.colour[3][0] == 4 * kK);
    SelectedLights over = rows(4, lc, lv);
    over.count = 9;
    assert(stage_rig(table, over, q, 7, 10).count == 4);
    // Limited by limit; 0 or below gives 0.
    assert(stage_rig(table, rows(4, lc, lv), q, 7, 3).count == 3);
    assert(stage_rig(table, rows(4, lc, lv), q, 7, 1).count == 1);
    assert(stage_rig(table, rows(2, lc, lv), q, 7, 3).count == 2);
    for (int limit : {0, -1, -100}) {
        const StageRig none = stage_rig(table, rows(4, lc, lv), q, 7, limit);
        assert(none.count == 0);
        for (std::size_t i = 0; i < 4; ++i) {
            assert((none.position[i] == std::array<float, 3>{}));
            assert((none.colour[i] == std::array<float, 3>{}));
            assert(none.falloff[i] == 0.0f);
        }
    }
    // Unused entries stay zero.
    const StageRig two = stage_rig(table, rows(4, lc, lv), q, 7, 2);
    assert(two.colour[1][0] == 2 * kK && two.falloff[1] == 2.0f);
    for (std::size_t i = 2; i < 4; ++i) {
        assert((two.position[i] == std::array<float, 3>{}));
        assert((two.colour[i] == std::array<float, 3>{}));
        assert(two.falloff[i] == 0.0f);
    }
}

void test_stage_rig_position_falloff() {
    const LightTable table = TableBytes().read();
    const std::array<std::array<float, 4>, 4> lc{};
    const std::array<std::array<float, 4>, 4> lv{{{1, -2, 3, 0.25f}, {-10, 0, 5, 0}, {0, 0, 0, 7}, {}}};
    const StageRig rig = stage_rig(table, rows(3, lc, lv), {100, 200, 300}, 7);
    assert(rig.count == 3);
    assert((rig.position[0] == std::array<float, 3>{101, 198, 303}));
    assert((rig.position[1] == std::array<float, 3>{90, 200, 305}));
    assert((rig.position[2] == std::array<float, 3>{100, 200, 300}));
    assert(rig.falloff[0] == 0.25f && rig.falloff[1] == 0.0f && rig.falloff[2] == 7.0f);
    assert(rig.falloff[3] == 0.0f);

    // Through select_lights: the light's own position comes back; falloff is
    // lv[3], which is aux_w for type 0 and 0 for type 1.
    TableBytes t;
    t.set(0, {.flags = 3, .type = 0, .position = {12, 4, -6}, .aux_w = 0.003f, .falloff = 0.001f,
              .rgb = {255, 0, 10}});
    t.set(1, {.flags = 3, .type = 1, .position = {0, 50, 0}, .aux_w = 9.0f, .falloff = 1.0f,
              .rgb = {1, 2, 3}});
    const std::array<float, 3> query{2, 4, -6};
    const auto sel = select_lights(t.read(), query, 5);
    assert(sel.count == 2 && sel.index[0] == 1 && sel.index[1] == 0);
    const StageRig r = stage_rig(t.read(), sel, query, 5);
    assert(r.count == 2);
    assert((r.position[0] == std::array<float, 3>{0, 50, 0}) && r.falloff[0] == 0.0f);
    assert((r.position[1] == std::array<float, 3>{12, 4, -6}) && r.falloff[1] == 0.003f);
    assert(r.colour[1][0] == 255 * kK && r.colour[1][1] == 0.0f && r.colour[1][2] == 10 * kK);
}

void test_stage_rig_colour() {
    const LightTable table = TableBytes().read();
    assert(std::bit_cast<std::uint32_t>(kK) == 0x3B808081u);
    const std::array<std::array<float, 4>, 4> lc{{{255, 255, 255, 0.5f}, {128, 0, 64, 99}, {}, {}}};
    const std::array<std::array<float, 4>, 4> lv{};
    SelectedLights s = rows(2, lc, lv);
    const StageRig a = stage_rig(table, s, {0, 0, 0}, 7);
    const float full = kK * 255.0f;
    assert(a.colour[0][0] == full && a.colour[0][1] == full && a.colour[0][2] == full);
    assert(near(full, 1.0f, 1e-5f));
    assert(a.colour[1][0] == 128 * kK && a.colour[1][1] == 0.0f && a.colour[1][2] == 64 * kK);
    // lc[3] is ignored, and so is the score.
    s.lc[3] = 1000.0f;
    s.lc[7] = -5.0f;
    s.score = {0.1f, 0.2f, 0, 0};
    const StageRig b = stage_rig(table, s, {0, 0, 0}, 7);
    assert(a.colour == b.colour);
}

void test_stage_rig_ambient() {
    TableBytes t;
    put_u16(t.bytes, 0x06, 32);
    put_u16(t.bytes, 0x08, 32);
    put_u16(t.bytes, 0x0A, 32);
    put_u16(t.bytes, 0x0C, 220);
    put_u16(t.bytes, 0x0E, 210);
    put_u16(t.bytes, 0x10, 200);
    const LightTable table = t.read();
    const SelectedLights none{};
    const std::array<float, 3> normal{220, 210, 200}, mask2{32, 32, 32};
    const std::array<float, 3> q{0, 0, 0};
    assert(stage_rig(table, none, q, 1).ambient == normal);
    for (unsigned mask : {0u, 2u, 5u, 7u}) assert(stage_rig(table, none, q, mask).ambient == mask2);
    assert(stage_rig(table, none, q, 1, 0).ambient == normal);  // independent of count
    // The opposite test from the model rig.
    assert(model_rig(table, none, 1).ambient == normal && model_rig(table, none, 2).ambient == mask2);
    assert(stage_rig(table, none, q, 2).ambient == mask2 && model_rig(table, none, 7).ambient == normal);
}

// --- helpers -----------------------------------------------------------------

void test_describe() {
    TableBytes t(3);
    for (std::size_t i = 0; i < 6; ++i) t.set(i * 2, {.flags = 1, .type = 0, .falloff = 0.1f});
    for (std::size_t i = 0; i < 3; ++i) t.set(20 + i, {.flags = 3, .type = 1});
    t.set(4, {.flags = 1, .colour_modifier = 1, .type = 0});
    t.set(21, {.flags = 3, .colour_modifier = 9, .type = 1});
    const LightTable table = t.read();
    assert(describe(table) ==
           "LIG2 v3: 9 of 48 records active (type 0: 6, type 1: 3; 2 with a colour modifier)");
    assert(describe(TableBytes(0).read()) ==
           "LIG2 v0: 0 of 48 records active (type 0: 0, type 1: 0; 0 with a colour modifier)");
    // Records with a modifier are not candidates.
    assert(candidate_count(table, 7, ListKind::main) == 7);
}

}  // namespace

// The pipeline's table is the format reader's: one 0x20 header, 48 records.
void test_format_ties() {
    namespace format = dmc::rengine::formats::lig2;
    static_assert(kHeaderSize == format::header_size && kRecordSize == format::record_size);
    static_assert(kRecordCount == format::expected_dmc3_record_count && kTableSize == format::expected_dmc3_file_size);
    // The holders' masks: a stage object picks point lights (low mode 3) and
    // the main list; a model picks directional ones (low mode 2) and both lists.
    LightRecord point{};
    point.active = true;
    point.flags = 3;
    LightRecord directional = point;
    directional.flags = 2;
    directional.type = 1;
    assert(is_candidate(point, stage_object_mask | 4U, ListKind::main));
    assert(!is_candidate(directional, stage_object_mask | 4U, ListKind::main));
    assert(is_candidate(directional, model_object_mask | 4U | 8U, ListKind::main));
    assert(!is_candidate(point, model_object_mask | 4U | 8U, ListKind::main));
    static_assert(va::select == 0x1402EE560ULL && va::stage_colour_scale == 0x14035D558ULL);
}

int main() {
    test_format_ties();
    test_read_fields();
    test_read_size_and_magic();
    test_read_non_finite();
    test_find_light_table();
    test_eligibility_main();
    test_eligibility_main_0x10();
    test_eligibility_secondary();
    test_never_selected();
    test_score();
    test_ordering();
    test_ordering_threshold();
    test_single_light();
    test_matrices();
    test_describe();
    test_header_fields();
    test_model_rig_count();
    test_model_rig_attenuation();
    test_model_rig_direction();
    test_model_rig_ambient();
    test_stage_rig_count();
    test_stage_rig_position_falloff();
    test_stage_rig_colour();
    test_stage_rig_ambient();
    return 0;
}
