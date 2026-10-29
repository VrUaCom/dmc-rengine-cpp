#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

// DMC3 stage lights. The game's light table (LIG2, stage configuration PAC
// physical slot 1): 48 point / directional light records, the
// choice of the four lights that light a query point, and the colour and
// vector matrices a renderer uploads for them, and a model's three-light rig
// and the stage's four-light rig built from those matrices. First written clean-room for the
// Native Reader from its behaviour specification (DMC-Native-Reader
// docs/cleanroom/specs-v124/game_lights.md, revision 4).
namespace dmc::rengine::profiles::dmc3::lights {

// ---- Where the game does it (dmc3.exe SHA-256 e454272e...dcbdd082) ---------
// Evidence: docs/research/dmc3-light-pipeline-gates-2026-10-29.md; every
// address below is re-checked by tools/reverse/dmc3_gates.cpp.
namespace va {
inline constexpr std::uint64_t light_static_ctor = 0x14023ECB0ULL;   // CLightStatic (LIG2 object, 0xF28 bytes)
inline constexpr std::uint64_t light_static_vtable = 0x1404E3128ULL;
inline constexpr std::uint64_t scene_game_main_vtable = 0x1404E32C8ULL;  // CSceneGameMain
inline constexpr std::uint64_t light_mgr_vtable = 0x1404E3118ULL;       // CLightMgr at CSceneGameMain+0x40
inline constexpr std::uint64_t chain_vtable = 0x1404E3108ULL;           // CChain at CLightMgr+0x18 / +0x30
inline constexpr std::uint64_t stage_config_load = 0x14023C3D0ULL;      // copies cfg PAC slot 1 (0x920 bytes)
inline constexpr std::uint64_t link_static_lights = 0x140089000ULL;     // CLightStatic -> main CChain
inline constexpr std::uint64_t chain_append = 0x1402C8310ULL;
inline constexpr std::uint64_t chain_remove = 0x1402C8350ULL;
inline constexpr std::uint64_t chain_clear = 0x1402C83A0ULL;
inline constexpr std::uint64_t register_dynamic_light = 0x1402EE9F0ULL; // secondary list / single light
inline constexpr std::uint64_t select = 0x1402EE560ULL;                // top four of the lists
inline constexpr std::uint64_t score = 0x1402EE2A0ULL;
inline constexpr std::uint64_t model_lights = 0x1402FD040ULL;          // CDrawSCM: model rig
inline constexpr std::uint64_t model_pack = 0x1402FC850ULL;            // constants at model+0x280
inline constexpr std::uint64_t model_ambient = 0x1402FA580ULL;
inline constexpr std::uint64_t static_lights = 0x1402FC720ULL;        // object+0x300 seed
inline constexpr std::uint64_t stage_lights = 0x1402FD370ULL;          // stage rig, local positions
inline constexpr std::uint64_t stage_pack = 0x1402FCED0ULL;            // B+0xB0..+0x12F
inline constexpr std::uint64_t stage_ambient = 0x1402FA6D0ULL;         // B+0x140, raw
inline constexpr std::uint64_t stage_colour_scale = 0x14035D558ULL;    // float 0x3B808081 (1/255)
inline constexpr std::uint64_t light_mgr_global = 0x140C90E38ULL;      // CLightMgr*
inline constexpr std::uint64_t stage_holder_ctor = 0x140089320ULL;     // dword [+0x570] = 0x10000: mask 1, lights on
inline constexpr std::uint64_t model_holder_ctor = 0x140089270ULL;     // dword [+0x744] = 2: mask 2
inline constexpr std::uint64_t shadow_direction_query = 0x14031FA80ULL; // mask 0x1C: flag-4 records only (hypothesis: shadow)
} // namespace va

// Object light masks the holders start with (Gate 48 v0.3).
inline constexpr unsigned stage_object_mask = 1U;  // static select 1|4, frame |8; ambient +0x0C..+0x10
inline constexpr unsigned model_object_mask = 2U;  // select 2|4|8; ambient +0x06..+0x0A


inline constexpr std::size_t kRecordCount = 48;
inline constexpr std::size_t kHeaderSize = 0x20;
inline constexpr std::size_t kRecordSize = 0x30;
inline constexpr std::size_t kTableSize = kHeaderSize + kRecordCount * kRecordSize;  // 0x920

struct LightRecord {
    std::uint8_t index{};          // 0..47
    bool active{};                 // any of the record's 0x30 bytes is non-zero
    std::uint8_t flags{}, colour_modifier{}, type{};
    std::array<float, 3> position{};
    float aux_w{}, falloff{};
    std::array<std::uint16_t, 3> rgb{};
};

struct LightTable {
    std::uint16_t version{};
    std::array<std::uint16_t, 6> header_words{};  // meaning unknown; for display
    std::array<LightRecord, kRecordCount> records{};
    std::int8_t static_light_count{};             // +0x04 as signed 8-bit: lights a model keeps
    std::array<float, 3> ambient{};               // u16 at +0x0C, +0x0E, +0x10 as floats, unscaled
    std::array<float, 3> ambient_mask2{};         // u16 at +0x06, +0x08, +0x0A: ambient when mask == 2
};

// Which light list a selection fills: main needs mask bit 4, secondary bit 8.
enum class ListKind : std::uint8_t { main, secondary };

// Null when shorter than 0x920 bytes, the magic is not "LIG2", or an active
// record has a non-finite position, aux_w or falloff; extra bytes are ignored.
[[nodiscard]] std::optional<LightTable> read_light_table(std::span<const std::uint8_t> bytes) noexcept;
// The table at the first 4-aligned offset where read_light_table succeeds.
[[nodiscard]] std::optional<LightTable> find_light_table(std::span<const std::uint8_t> bytes) noexcept;
[[nodiscard]] std::optional<LightTable> read_light_table(std::span<const std::byte> bytes) noexcept;
[[nodiscard]] std::optional<LightTable> find_light_table(std::span<const std::byte> bytes) noexcept;

// Candidate: active, no colour modifier, type 0 or 1, the list's mask bit set,
// and, with f = flags (secondary: bit 3 cleared), either f & 4 with mask & 0x10,
// or low = f without bit 2 allowed: 1 needs mask & 3, 2 mask & 2, 3 mask & 1.
[[nodiscard]] bool is_candidate(const LightRecord& record, unsigned mask, ListKind list) noexcept;
// Type 0: max(0, 1 - |position - query|^2 * falloff); type 1: 1.
[[nodiscard]] float score(const LightRecord& record, std::array<float, 3> query) noexcept;

struct SelectedLights {
    int count{};                                  // 0..4
    std::array<int, 4> index{-1, -1, -1, -1};     // record index per slot
    std::array<float, 4> score{};
    std::array<float, 16> lc{};                   // 4 rows of 4: colour
    std::array<float, 16> lv{};                   // 4 rows of 4: vector
};

// Candidates in record order enter slot 3 only when scoring strictly above it,
// then rise past every slot scoring <= theirs (a later equal score goes above).
// With single given and mask & 2, it is scored and put in slot 0, shifting the
// others down (slot 3 dropped). lc row: (r, g, b, falloff | 1 for type 1);
// lv row: (position - query, aux_w | 0 for type 1). Unused rows zero, index -1.
[[nodiscard]] SelectedLights select_lights(const LightTable& table, std::array<float, 3> query,
                                           unsigned mask, ListKind list = ListKind::main,
                                           const LightRecord* single = nullptr) noexcept;

struct ModelRig {
    int count{};                                         // 0..3 lights
    std::array<std::array<float, 3>, 3> direction{};     // unit vectors
    std::array<std::array<float, 3>, 3> colour{};        // 0..255 units (128 = 1)
    std::array<float, 3> ambient{};                      // 0..255 units
};

// Uses the first min(lights.count, 3, max(limit, 0)) slots. Per slot: attenuation
// a = max(0, 1 - |lv.xyz|^2 * lv.w) when lv.w != 0, else 1; colour = lc.rgb * a
// (score and lc.w unused); direction = lv.xyz normalised (zero stays zero).
// Ambient is table.ambient_mask2 when mask == 2 exactly, else table.ambient.
[[nodiscard]] ModelRig model_rig(const LightTable& table, const SelectedLights& lights,
                                 unsigned mask, int limit = 3) noexcept;

struct StageRig {
    int count{};                                          // 0..4 lights
    std::array<std::array<float, 3>, 4> position{};       // light positions
    std::array<float, 4> falloff{};
    std::array<std::array<float, 3>, 4> colour{};         // per unit of facing
    std::array<float, 3> ambient{};                       // 0..255 units
};

// Uses the first min(lights.count, 4, max(limit, 0)) slots. Per slot: position =
// query + lv.xyz; falloff = lv.w; colour = lc.rgb * K, K the float with bits
// 0x3B808081 (about 1/255; lc.w unused).
// Ambient is table.ambient when mask == 1 exactly, else table.ambient_mask2.
[[nodiscard]] StageRig stage_rig(const LightTable& table, const SelectedLights& lights,
                                 std::array<float, 3> query, unsigned mask, int limit = 4) noexcept;

// The number of records that pass the candidate rules (no score applied).
[[nodiscard]] std::size_t candidate_count(const LightTable& table, unsigned mask, ListKind list) noexcept;
// One line: "LIG2 v3: 9 of 48 records active (type 0: 6, type 1: 3; 2 with a
// colour modifier)"; counts are over active records. Empty on allocation failure.
[[nodiscard]] std::string describe(const LightTable& table) noexcept;

} // namespace dmc::rengine::profiles::dmc3::lights
