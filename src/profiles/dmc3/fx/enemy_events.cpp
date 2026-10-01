#include "dmc_rengine/profiles/dmc3/fx/enemy_events.hpp"

namespace dmc::rengine::profiles::dmc3::fx::enemy {
namespace {

using runtime::Kind;
using runtime::MatrixMode;
constexpr auto Given = Placement::GivenPosition;
constexpr auto Yaw = Placement::ActorPositionYaw;
constexpr auto Att = Placement::AttachedObject;
constexpr auto Trans = Placement::ObjectTranslation;
constexpr auto Fields = Placement::ActorFields;
constexpr auto M0 = MatrixMode::Copy;
constexpr auto M1 = MatrixMode::PositionOnly;

constexpr EventSpawn k00[] = {{Kind::G, 75, Att, 5, M0, 1.0F}};
constexpr EventSpawn k01[] = {{Kind::V, 105, Given, 0, M0, 1.0F}};
constexpr EventSpawn k03[] = {{Kind::E, 42, Att, 1, M1, 2.0F}, {Kind::V, 42, Att, 1, M1, 2.0F}};
constexpr EventSpawn k06[] = {{Kind::V, 26, Att, 0, M1, 1.0F}};
constexpr EventSpawn k08[] = {{Kind::V, 70, Fields, 0, M0, 1.0F}};
constexpr EventSpawn k09[] = {{Kind::V, 84, Given, 0, M0, 1.0F}};
constexpr EventSpawn k0a[] = {{Kind::V, 273, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k0b[] = {{Kind::G, 152, Att, 0, M1, 1.0F}};
constexpr EventSpawn k0c[] = {{Kind::V, 274, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k0d[] = {{Kind::V, 281, Trans, 2, M0, 1.0F}};
constexpr EventSpawn k0e[] = {{Kind::P, 44, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k0f[] = {{Kind::V, 288, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k10[] = {{Kind::V, 290, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k11[] = {{Kind::V, 282, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k13[] = {{Kind::G, 152, Att, 0, M1, 1.0F}};
constexpr EventSpawn k15[] = {{Kind::V, 499, Att, 1, M0, 1.0F}};
constexpr EventSpawn k16[] = {{Kind::V, 516, Att, 0, M0, 1.0F}};
constexpr EventSpawn k18[] = {{Kind::V, 389, Att, 9, M0, 1.0F}};
constexpr EventSpawn k19[] = {{Kind::V, 532, Att, 1, M0, 1.0F}};
constexpr EventSpawn k1a[] = {{Kind::V, 440, Att, 1, M0, 1.0F}};
constexpr EventSpawn k1b[] = {{Kind::V, 623, Att, 0, M0, 1.0F}};
constexpr EventSpawn k1c[] = {{Kind::V, 703, Att, 0, M0, 1.0F}};
constexpr EventSpawn k1e[] = {{Kind::V, 672, Att, 36, M0, 1.0F}};
constexpr EventSpawn k1f[] = {{Kind::V, 702, Att, 1, M0, 1.0F}};
constexpr EventSpawn k21[] = {{Kind::V, 620, Att, 0, M0, 1.0F}};
constexpr EventSpawn k23[] = {{Kind::V, 848, Att, 0, M0, 1.0F}};
constexpr EventSpawn k24[] = {{Kind::V, 346, Att, 36, M1, 1.0F}};
constexpr EventSpawn k29[] = {{Kind::V, 672, Att, 36, M0, 1.0F}};
constexpr EventSpawn k2b[] = {{Kind::V, 412, Att, 1, M0, 1.0F}};
constexpr EventSpawn k2c[] = {{Kind::V, 223, Att, 5, M0, 1.0F}};
constexpr EventSpawn k66[] = {{Kind::P, 32, Given, 0, M0, 1.0F}};
constexpr EventSpawn k67[] = {{Kind::P, 35, Yaw, 0, M0, 1.0F}};
constexpr EventSpawn k68[] = {{Kind::V, 15, Att, 2, M0, 1.0F}, {Kind::G, 13, Att, 2, M0, 1.0F}};
constexpr EventSpawn k69[] = {{Kind::V, 132, Att, 9, M0, 1.0F}};
constexpr EventSpawn k6a[] = {{Kind::V, 15, Att, 3, M0, 1.0F}, {Kind::G, 13, Att, 3, M0, 1.0F}};
constexpr EventSpawn kc8[] = {{Kind::V, 315, Att, 4, M0, 1.0F}, {Kind::P, 32, Att, 5, M1, 1.0F},
                              {Kind::P, 32, Att, 21, M1, 1.0F}};
constexpr EventSpawn kc8t[] = {{Kind::P, 302, Att, 5, M1, 1.0F}, {Kind::P, 302, Att, 21, M1, 1.0F}};
constexpr EventSpawn kc9[] = {{Kind::V, 316, Att, 3, M0, 1.0F}, {Kind::P, 32, Att, 4, M1, 1.0F},
                              {Kind::P, 32, Att, 3, M1, 1.0F}, {Kind::P, 32, Att, 6, M1, 1.0F},
                              {Kind::P, 32, Att, 10, M1, 1.0F}};
constexpr EventSpawn kc9t[] = {{Kind::P, 302, Att, 4, M1, 1.0F}, {Kind::P, 302, Att, 3, M1, 1.0F},
                               {Kind::P, 302, Att, 6, M1, 1.0F}, {Kind::P, 302, Att, 10, M1, 1.0F}};
constexpr EventSpawn kca[] = {{Kind::V, 317, Att, 7, M0, 1.0F}, {Kind::V, 317, Att, 8, M0, 1.0F},
                              {Kind::P, 32, Att, 7, M1, 1.0F}, {Kind::P, 32, Att, 8, M1, 1.0F},
                              {Kind::P, 32, Att, 9, M1, 1.0F}};
constexpr EventSpawn kcat[] = {{Kind::P, 302, Att, 7, M1, 1.0F}, {Kind::P, 302, Att, 8, M1, 1.0F},
                               {Kind::P, 302, Att, 9, M1, 1.0F}};
constexpr EventSpawn kcb[] = {{Kind::V, 318, Att, 11, M0, 1.0F}, {Kind::V, 318, Att, 12, M0, 1.0F},
                              {Kind::P, 32, Att, 11, M1, 1.0F}, {Kind::P, 32, Att, 12, M1, 1.0F},
                              {Kind::P, 32, Att, 13, M1, 1.0F}};
constexpr EventSpawn kcbt[] = {{Kind::P, 302, Att, 11, M1, 1.0F}, {Kind::P, 302, Att, 12, M1, 1.0F},
                               {Kind::P, 302, Att, 13, M1, 1.0F}};
constexpr EventSpawn kcc[] = {{Kind::V, 319, Att, 1, M0, 1.0F}, {Kind::P, 32, Att, 14, M1, 1.0F},
                              {Kind::P, 32, Att, 15, M1, 1.0F}, {Kind::P, 32, Att, 18, M1, 1.0F}};
constexpr EventSpawn kcct[] = {{Kind::P, 302, Att, 14, M1, 1.0F}, {Kind::P, 302, Att, 15, M1, 1.0F},
                               {Kind::P, 302, Att, 18, M1, 1.0F}};
constexpr EventSpawn kcd[] = {{Kind::P, 32, Att, 2, M1, 1.0F}};
constexpr EventSpawn kcdt[] = {{Kind::P, 302, Att, 2, M1, 1.0F}};
constexpr EventSpawn kce[] = {{Kind::P, 32, Att, 16, M1, 1.0F}, {Kind::P, 32, Att, 19, M1, 1.0F},
                              {Kind::P, 32, Att, 17, M1, 1.0F}, {Kind::P, 32, Att, 20, M1, 1.0F}};
constexpr EventSpawn kcet[] = {{Kind::P, 302, Att, 16, M1, 1.0F}, {Kind::P, 302, Att, 19, M1, 1.0F},
                               {Kind::P, 302, Att, 17, M1, 1.0F}, {Kind::P, 302, Att, 20, M1, 1.0F}};

constexpr EventCase kCases[] = {
    {0x00, 0xFF, k00}, {0x01, 0xFF, k01}, {0x03, 0xFF, k03}, {0x06, 0xFF, k06}, {0x08, 0xFF, k08},
    {0x09, 0xFF, k09}, {0x0A, 0xFF, k0a}, {0x0B, 0xFF, k0b}, {0x0C, 0xFF, k0c}, {0x0D, 0xFF, k0d},
    {0x0E, 0xFF, k0e}, {0x0F, 0xFF, k0f}, {0x10, 0xFF, k10}, {0x11, 0xFF, k11}, {0x13, 0xFF, k13},
    {0x15, 0xFF, k15}, {0x16, 0xFF, k16}, {0x18, 0xFF, k18}, {0x19, 0xFF, k19}, {0x1A, 0xFF, k1a},
    {0x1B, 0xFF, k1b}, {0x1C, 0xFF, k1c}, {0x1E, 0xFF, k1e}, {0x1F, 0xFF, k1f}, {0x21, 0xFF, k21},
    {0x23, 0xFF, k23}, {0x24, 0xFF, k24}, {0x25, 0xFF, k24}, {0x26, 0xFF, k24}, {0x29, 0xFF, k29},
    {0x2B, 0xFF, k2b}, {0x2C, 0xFF, k2c}, {0x66, 0xFF, k66}, {0x67, 0xFF, k67}, {0x68, 0xFF, k68},
    {0x69, 0xFF, k69}, {0x6A, 0xFF, k6a},
    {0xC8, 0x1A, kc8t}, {0xC8, 0xFF, kc8}, {0xC9, 0x1A, kc9t}, {0xC9, 0xFF, kc9},
    {0xCA, 0x1A, kcat}, {0xCA, 0xFF, kca}, {0xCB, 0x1A, kcbt}, {0xCB, 0xFF, kcb},
    {0xCC, 0x1A, kcct}, {0xCC, 0xFF, kcc}, {0xCD, 0x1A, kcdt}, {0xCD, 0xFF, kcd},
    {0xCE, 0x1A, kcet}, {0xCE, 0xFF, kce},
};

constexpr DeathStep kDeath[] = {
    {0.0F, {0x69, 0xC8}, 2}, {5.0F, {0xCA, 0xCB}, 2}, {10.0F, {0xC9, 0}, 1},
    {15.0F, {0xCD, 0}, 1},   {20.0F, {0xCC, 0}, 1},   {25.0F, {0xCE, 0}, 1},
};

constexpr CommandTables kTables[] = {
    {"CEm000", 0x140093E90U, 0x1405A3300U, 0x1405A31E8U}, {"CEm001", 0x140099100U, 0x1405A4370U, 0x1405A4278U},
    {"CEm002", 0x14009DD30U, 0x1405A54B0U, 0x1405A53A8U}, {"CEm003", 0x1400A2C60U, 0x1405A6410U, 0x1405A6318U},
    {"CEm004", 0x1400A7B80U, 0x1405A7050U, 0x1405A6F68U}, {"CEm005", 0x1400A9230U, 0x1405A79E0U, 0x140CA80E0U},
    {"CEm006", 0x1400AE800U, 0x1405A8320U, 0x1405A8218U}, {"CEm007", 0x1400B0F10U, 0x1405A8FD0U, 0x1405A8EB8U},
};

constexpr AttackEvent kAttacks[] = {
    {0, 62, 40.0F, 3, 193}, {0, 62, 50.0F, 3, 193}, {0, 66, 30.0F, 3, 235},
    {0, 68, 40.0F, 3, 191}, {0, 68, 50.0F, 3, 191}, {0, 82, 76.0F, 3, 229},
    {0, 84, 180.0F, 3, 231}, {0, 85, 100.0F, 3, 237}, {0, 87, 30.0F, 3, 237},
};

}  // namespace

std::span<const EventCase> event_cases() noexcept { return kCases; }

const EventCase* find_event(std::uint16_t code, std::uint8_t enemy_type) noexcept {
    for (const auto& c : kCases) {
        if (c.code != code) continue;
        if (c.enemy_type == 0xFFU || c.enemy_type == enemy_type) return &c;
    }
    return nullptr;
}

std::span<const DeathStep> em000_death_schedule() noexcept { return kDeath; }
std::span<const CommandTables> command_tables() noexcept { return kTables; }
std::span<const AttackEvent> em000_attack_events() noexcept { return kAttacks; }

}  // namespace dmc::rengine::profiles::dmc3::fx::enemy
