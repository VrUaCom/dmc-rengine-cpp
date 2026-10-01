#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "dmc_rengine/profiles/dmc3/fx/effect_runtime.hpp"

// Enemy effect events of the em000..em008 family
// (docs/research/dmc3-effect-triggers-2026-10-01.md,
//  docs/research/dmc3-em000-attack-effects-2026-10-01.md).
//
// CEm000..CEm007 derive from CNonPlayerDeath. Their interface at object +0x110
// has the event handler 0x1401C3130(iface, code, vec3* pos) in slot 16
// (+0x80). Codes < 0x384 spawn effects; larger codes are control events. The
// AI command classes (CComEm000 / 005 / 006 / 008) and the death code send the
// codes with constant arguments.
namespace dmc::rengine::profiles::dmc3::fx::enemy {

inline constexpr std::uint64_t kEventHandler = 0x1401C3130U;
inline constexpr std::uint16_t kFirstControlCode = 0x384U;

// Where a spawned effect is placed (no-player path of the handler).
enum class Placement : std::uint8_t {
    GivenPosition,     // the vec3 argument (callers pass the actor position)
    ActorPositionYaw,  // actor position obj+0x80, s16 yaw obj+0xC0 (x 2pi/65536)
    AttachedObject,    // effect +0xC8 = obj+0x6D8[object]->+0x110 (body joint world), +0xD4 = mode
    ObjectTranslation, // spawn at the translation of obj+0x6D8[object], not attached
    ActorFields,       // reads actor fields not reproduced by the fabricated run
};

struct EventSpawn final {
    runtime::Kind kind{};
    std::uint16_t id{};
    Placement placement{};
    std::uint8_t object{};  // entry of obj+0x6D8 = body joint k (> 22: cloth / later arrays)
    runtime::MatrixMode parent_mode{};
    float scale{1.0F};      // uniform scale applied to the spawn matrix (0x1403304F0)
};

// One handler case. `enemy_type` 0xFF: any; 0x1A: only when obj+0x670 == 0x1A
// (that type swaps P32 for P302 and skips the V in 0xC8..0xCE).
struct EventCase final {
    std::uint16_t code{};
    std::uint8_t enemy_type{0xFFU};
    std::span<const EventSpawn> spawns;
};

// Emulated per code on a fabricated actor; complete for the no-player branch.
[[nodiscard]] std::span<const EventCase> event_cases() noexcept;
[[nodiscard]] const EventCase* find_event(std::uint16_t code, std::uint8_t enemy_type = 0U) noexcept;

// Death of CEm000..CEm004 (0x140095E85, one copy per class): codes 0x69 and
// 0xC8 on entry, then an accumulating timer obj+0x2EFC. Started by control
// code 0x3E7 (handler 0x1401C5CD8 sets obj+0x2EF4), which the damage command
// 0x1400675C0 sends with 0x385 on the killing hit.
inline constexpr std::uint16_t kDeathControlCode = 0x3E7U;
// Body model bind 0x14030F850: obj+0x6D8[k]+0x110 = model+0x188 + 64 * k.
inline constexpr std::uint64_t kJointArrayBind = 0x14030F850U;
inline constexpr std::size_t kBodyJointEntries = 23U;
struct DeathStep final {
    float tick{};
    std::array<std::uint16_t, 2> codes{};
    std::uint8_t count{};
};
[[nodiscard]] std::span<const DeathStep> em000_death_schedule() noexcept;

// AI command layer. Interface slots used by commands: +0x20 play(cmd) via the
// class table at obj+0x2DF0, +0x28 play2(cmd) via obj+0x2DF8, +0x40 motion
// frame, +0x48 action finished, +0x80 event.
struct CommandTables final {
    const char* enemy_class{};
    std::uint64_t init{};    // class constructor
    std::uint64_t table1{};  // (bank, action) byte pairs indexed by command
    std::uint64_t table2{};
};
[[nodiscard]] std::span<const CommandTables> command_tables() noexcept;

// CEm000..CEm004 share one table: command n = bank 0 action n (n < 100);
// play2 maps through this list (bank 0).
inline constexpr std::array<std::uint8_t, 27> kEm000Play2Actions{
    16, 17, 17, 18, 19, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 41, 58, 59, 60, 61, 37, 33, 23, 62, 63, 64, 65};

// Frame-gated code 3 of the CComEm000 commands, by script action (bank 0):
// E42 + V42, x2, attached to object 1 by position.
struct AttackEvent final {
    std::uint8_t bank{};
    std::uint8_t action{};
    float frame{};
    std::uint16_t code{};
    std::uint16_t command_slot{};  // CComEm000 vtable 0x1404C6A18 update slot
};
[[nodiscard]] std::span<const AttackEvent> em000_attack_events() noexcept;

}  // namespace dmc::rengine::profiles::dmc3::fx::enemy
