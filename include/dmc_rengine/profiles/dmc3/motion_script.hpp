#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

// Read-only port of the character motion script (IPlayer: pl000.pac slot 5;
// enemies: the script slot of their archive, e.g. em028 slot 10, em000 slot
// 38). Reverse authority: dmc-rengine-cpp
// docs/research/dmc3-player-motion-script-2026-09-24.md and
// docs/research/dmc3-enemy-motion-script-2026-09-24.md.
//
// Bind 0x1400594B0 reads three u16 header offsets: A (+0, scripts), B (+2,
// motion resources) and C (+4). Mode 0 (player) nests once more: banks =
// A + u16[A], resources = B + u16[B]; mode 1 (enemies) uses A and B as they
// are. Play 0x14005A290(obj, bank, action): sub = banks + u16[banks + 2 bank],
// script = sub + u16[sub + 2 action]. Interpreter 0x140058FE0 runs a block
// until opcode 0 ([00, ?, frame u16, flag, ?]: the next block runs once the
// motion frame passes `frame`, 0x7FFF = motion end). Opcode 1 (play, bytes 4/5
// = bank/action) resolves the MOT through B (0x14005A360): record = count u8,
// pad, count x {object, loop, flag, ?, id u16}; id / 100 picks the actor's
// motion PAC, id % 100 its slot. Opcode 3 ([03, b1..b5]) sets channel 0;
// 0x1401F01F0 reads b2: low 6 bits = weapon attach state (0 = unchanged).
namespace dmc::rengine::profiles::dmc3::motion {

// State `state` applies once the motion frame is past `after_frame`
// (-1: from the first frame).
struct WeaponStateKey final {
    float after_frame{-1.0F};
    std::uint8_t state{};
};

// Raw opcode-3 five-channel payload at one script time. This is the generic
// MotionScript contract used by CEm034; player weapon state remains a
// projection of channel byte 1 (b2 low 6 bits).
struct ScriptSignalKey final {
    float after_frame{-1.0F};
    std::array<std::uint8_t, 5> channels{};
};

// What one script does, walked like the interpreter (first block chain up to
// the motion end, a loop jump or a hand-over to another MOT).
struct ScriptSummary final {
    std::uint8_t play_bank{};    // opcode 1 byte 4
    std::uint8_t play_index{};   // opcode 1 byte 5
    std::uint16_t instructions{};
    std::uint16_t waits{};       // opcode 0 blocks
    std::uint16_t last_frame{};  // highest wait frame below 0x7FFF
    bool loops{};                // opcode 2 (backward jump)
    bool hands_over{};           // a second opcode 1
    std::vector<WeaponStateKey> states;
    std::vector<ScriptSignalKey> signals;
    std::array<std::uint16_t, 64> opcodes{};  // count per opcode (0..63)
};

// One motion resource of an action (table B, 0x14005A360).
struct MotionResource final {
    std::uint8_t object{};  // script object index (bind r8, obj+0x60)
    std::uint8_t loop{};    // 0 once, 1 loop, 2 ask the actor (obj+0xB7)
    std::uint8_t flag{};    // == 1 -> obj+0xB6
    std::uint8_t extra{};
    std::uint16_t id{};     // motion PAC group * 100 + slot

    [[nodiscard]] constexpr std::uint16_t group() const noexcept { return id / 100U; }
    [[nodiscard]] constexpr std::uint16_t slot() const noexcept { return id % 100U; }
};

struct ScriptAction final {
    std::size_t bank{};
    std::size_t action{};
};

class MotionScriptFile final {
public:
    [[nodiscard]] static std::optional<MotionScriptFile> parse(std::span<const std::uint8_t> bytes);

    // Structural identity for a lone file: header table, 0xFFFF-terminated
    // bank list, and every non-empty bank's first script starts with opcode 1.
    [[nodiscard]] static bool looks_like(std::span<const std::uint8_t> bytes);

    [[nodiscard]] std::size_t bank_count() const noexcept { return banks_.size(); }
    [[nodiscard]] std::size_t size_bytes() const noexcept { return bytes_.size(); }
    [[nodiscard]] std::size_t header_table() const noexcept { return table_; }
    // Mode 0 (player, one more table level) or mode 1 (enemies).
    [[nodiscard]] bool nested() const noexcept { return nested_; }
    [[nodiscard]] bool has_resources() const noexcept { return resources_ != 0U; }

    // Scripts in bank `bank` (entries before its 0xFFFF terminator).
    [[nodiscard]] std::size_t script_count(std::size_t bank) const noexcept;

    [[nodiscard]] std::optional<ScriptSummary> summarize(std::size_t bank, std::size_t index) const;

    // Weapon attach states of action `action` of bank `bank`.
    [[nodiscard]] std::vector<WeaponStateKey> weapon_states(std::size_t bank,
                                                            std::size_t action) const;

    // Full opcode-3 five-channel timeline for enemy/boss consumers.
    [[nodiscard]] std::vector<ScriptSignalKey> signals(std::size_t bank,
                                                       std::size_t action) const;

    // Motion resources an action plays (empty without table B).
    [[nodiscard]] std::vector<MotionResource> resources(std::size_t bank,
                                                        std::size_t action) const;

    // Actions whose resources play motion id `id` (group * 100 + slot).
    [[nodiscard]] std::vector<ScriptAction> actions_for(std::uint16_t id) const;

    // Weapon states for the MOT in slot `slot` of motion PAC `group`: the
    // first action of that bank that plays it (the action with the same
    // index first), falling back to action `slot`.
    [[nodiscard]] std::vector<WeaponStateKey> weapon_states_for_motion(std::size_t group,
                                                                       std::size_t slot) const;

    // Every motion id table B uses (sorted, unique).
    [[nodiscard]] std::vector<std::uint16_t> resource_ids() const;

private:
    std::vector<std::uint8_t> bytes_;
    std::vector<std::size_t> banks_;  // absolute bank table offsets
    std::size_t table_{};
    std::size_t resources_{};  // absolute table B (0 = none)
    bool nested_{};
};

// A MOT-only PAC inside a character archive: its archive slot and the MOT
// slots it fills.
struct MotionPack final {
    std::uint32_t archive_slot{};
    std::vector<std::uint32_t> slots;
};

// Which archive slot serves each id group (id / 100) of a script. The
// actor's motion PAC array is built by its class (em028 0x140131037:
// {slot 2, slot 3}; em000 0x1400982D9: {slot 35}); those are used when the
// archive name is known (`exe_confirmed`), every other group takes the first
// unused MOT PAC in archive order whose slots cover every id of the group,
// or stays unbound.
struct MotionGroupBinding final {
    std::uint16_t group{};
    std::vector<std::uint32_t> slots;           // MOT slots the script needs
    std::optional<std::uint32_t> archive_slot;  // nullopt: no pack covers it
    bool exe_confirmed{};
};

[[nodiscard]] std::vector<MotionGroupBinding> bind_motion_groups(
    const MotionScriptFile& script, std::span<const MotionPack> packs,
    std::string_view archive_name);

// Weapon state at `frame` from a timeline (0 when none applies yet).
[[nodiscard]] std::uint8_t weapon_state_at(const std::vector<WeaponStateKey>& keys,
                                           float frame) noexcept;

// "pl000_00_<N>.pac" (any directory, any case) -> N.
[[nodiscard]] std::optional<std::size_t> player_motion_bank(std::string_view archive_name) noexcept;

}  // namespace dmc::rengine::profiles::dmc3::motion
