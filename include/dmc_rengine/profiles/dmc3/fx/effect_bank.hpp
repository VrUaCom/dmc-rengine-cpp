#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Read-only port of the effect bank loader 0x1402C04C0(bank, mode).
// Reverse authority: docs/research/dmc3-effect-bank-loader-2026-09-24.md and
// docs/research/dmc3-effect-runtime-2026-10-01.md (E / V / G / P views).
//
// A bank is a PNST: slot 0 a text manifest, slot 1 a PNST of records. The
// loader tokenizes the manifest (0x140322AB0, the .tsc tokenizer) and for each
// `<kind> <id>` pair hands the next record to the kind's registrar (jump table
// on kind - 'A'):
//   A 0x140322990   C 0x1402D3BE0   E 0x1402E87A0   G 0x1402ECBD0
//   M 0x1402E35D0 (record + the next one: model and its 16-byte companion)
//   P 0x140314B80   T 0x140322F20   V 0x140325030
// Any other letter consumes one record without registering it; a `#` token
// (e.g. "# End") ends the manifest. Enemies load their bank with mode 2
// (em028 slot 9, em000 family slot 41), stages with mode 0.
namespace dmc::rengine::profiles::dmc3::fx::effect_bank {

struct Record final {
    char kind{};
    // Canonical loader identity is (kind, u16 id), not a filename or ordinal.
    std::uint16_t id{};
    std::uint32_t slot{};                    // record slot in the inner PNST
    std::span<const std::uint8_t> bytes;     // empty when the slot is absent
    std::span<const std::uint8_t> companion; // M only (the next slot)
};

struct Bank final {
    std::vector<Record> records;
    std::size_t record_slots{};   // inner PNST slot count
    std::size_t manifest_bytes{};
    bool terminated{};            // a '#' token ended the manifest
};

// Registrar address of a kind (0 when the loader ignores it).
[[nodiscard]] std::uint64_t registrar(char kind) noexcept;
// Neutral runtime-kind description. It does not claim an original filename
// or file extension; T/M mention only byte-confirmed payload families.
[[nodiscard]] std::string_view kind_name(char kind) noexcept;

// Structural identity: PNST, slot 0 a manifest whose first token is one
// letter A..Z followed by a decimal id, slot 1 a PNST.
[[nodiscard]] bool looks_like_bank(std::span<const std::uint8_t> bytes) noexcept;

[[nodiscard]] std::optional<Bank> parse_bank(std::span<const std::uint8_t> bytes);

// Texture records: 112-byte descriptor, then a DDS file (DXT, with mips).
inline constexpr std::size_t kTextureDescriptorSize = 112U;
[[nodiscard]] std::span<const std::uint8_t> texture_dds(const Record& record) noexcept;

// Sprite animation (kind A, 336 bytes; layout from the data, registrar
// 0x140322990): [1, texture id, frame time, last frame index, loop, 0], then
// 10-byte frames (u16 x, y, w, h, 0) in texture pixels.
struct SpriteFrame final {
    std::uint16_t x{}, y{}, w{}, h{};
};
struct SpriteAnimation final {
    std::uint8_t texture{};
    std::uint8_t frame_time{};
    bool loop{};
    std::uint8_t loop_frame{};  // +0x05: frame a looping animation restarts at
    std::vector<SpriteFrame> frames;
};
[[nodiscard]] std::optional<SpriteAnimation> sprite_animation(const Record& record);

// Runtime-facing views recovered from the canonical dmc3.exe consumers. These
// structs intentionally expose only fields whose access pattern is proven by
// the executable; unknown bytes remain outside the typed view.

// V: fixed 0x2C-byte entries. 0x140324680 builds a local transform from
// translation, degree rotation (converted to radians), and scale, then
// dispatches the referenced child:
//   0=P, 1=E, 2=G, 3=V.
struct VEntry final {
    std::uint8_t dispatch{};
    std::uint16_t id{};
    std::array<float, 3> translation{};
    std::array<float, 3> rotation_degrees{};
    std::array<float, 3> scale{};
};
struct VRuntimeView final {
    std::int16_t count{};
    std::vector<VEntry> entries;
};
[[nodiscard]] std::optional<VRuntimeView> v_runtime_view(const Record& record);

// E: 0x1402E3AA0 resolves +0x04 through the T texture manager. When +0x06 is
// one and +0x08 != 0xFFFF, 0x1402E494F binds that id through the A manager.
// Otherwise +0x0C..+0x12 are copied directly into the runtime rectangle.
struct ERuntimeView final {
    std::uint8_t mode{};
    std::uint16_t texture_id{0xFFFFU};
    bool uses_animation{};
    std::uint16_t animation_id{0xFFFFU};
    SpriteFrame rectangle{};
};
[[nodiscard]] std::optional<ERuntimeView> e_runtime_view(const Record& record);

// G: the generator consumer 0x1402EBC10 reads these offsets directly. Exact
// gameplay labels for several scalar fields are still intentionally withheld;
// the view keeps their source offsets explicit instead of guessing semantics.
struct GRuntimeView final {
    std::uint8_t mode{};
    std::uint16_t c_id{};
    std::int32_t value_18{};
    std::int32_t value_20{};
    std::uint8_t mode_30{};
    float value_38{};
    float value_3c{};
    std::uint16_t steps_40{};
    float value_50{};
    float value_54{};
    std::uint8_t value_59{};
};
[[nodiscard]] std::optional<GRuntimeView> g_runtime_view(const Record& record);

// P: 0x140312DC0 validates version 2, rebases the relative pointers stored
// from +0x10, and returns the root object. 0x140312840 dispatches on root+1
// with runtime subtype 0..5. The optional child target list is described by
// root+0xC0 and the relative offsets beginning at raw+0x18.
struct PRuntimeView final {
    std::uint32_t version{};
    std::uint8_t subtype{};
    std::uint32_t root_offset{};
    std::uint32_t list_offset{};
    std::vector<std::uint32_t> target_offsets;
};
[[nodiscard]] std::optional<PRuntimeView> p_runtime_view(const Record& record);
// EXE-confirmed E record fields used by the runtime presentation path. The
// fields intentionally keep their structural names: semantic effect names are
// not inferred from a texture or a MotionScript action.
struct EffectDescriptor final {
    std::uint8_t mode{};
    std::uint16_t texture{};
    std::uint8_t animation_gate{};
    std::uint16_t animation{0xFFFFU};
    SpriteFrame rectangle{}; // direct x/y/width/height when A is inactive
    // Lifetime in ticks (+0x80, 0x1402E4190 -> effect+0x8B0). The state-1
    // update 0x1402E47F0 decrements it by dt and retires the effect once it
    // is negative, unless +0x84 is set: then the effect lives until its
    // parent retires it (0x1402E7A40 on parent+0x60).
    std::int32_t lifetime_ticks{};
    bool lifetime_known{};
    bool held_by_parent{};
    // Geometry (init 0x1402E42EA, draw 0x1402E5D00 / 0x1402E69E0). Size A and
    // pivot B in effect-local units: size mode +0x2C = 0 takes
    // A = (+0x3C, +0x44, +0x4C), B = (+0x30, +0x34, +0x38); modes 1/2 draw A
    // from [min, max] pairs at +0x3C (per axis / uniform) with B = A * 0.5.
    // Reader uses the mean of retail random draws.
    bool geometry_known{};
    std::array<float, 3> size{};
    std::array<float, 3> pivot{};
    std::array<float, 3> scale{1.0F, 1.0F, 1.0F};   // +0xA8
    // Initial rotation D in degrees (+0x150 + 12*i: flag, min, max).
    std::array<float, 3> rotation_degrees{};
    std::uint8_t orientation{};                      // +0x1F5 (mode 2)
};
[[nodiscard]] std::optional<EffectDescriptor> effect_descriptor(const Record& record);

// V is a composite runtime record. Its entries are kept as a dependency graph
// so callers can compose the EXE child transforms without flattening a nested
// V or assigning a semantic name to a P/G subtype.
struct CompositeEntry final {
    std::uint8_t dispatch_kind{}; // 0=P, 1=E, 2=G, 3=V
    std::uint16_t id{};
    std::int16_t activation_offset{};
    std::array<float, 3> translation{};
    std::array<float, 3> rotation_degrees{};
    std::array<float, 3> scale{1.0F, 1.0F, 1.0F};
};
struct CompositeRecord final {
    std::vector<CompositeEntry> entries;
};
[[nodiscard]] std::optional<CompositeRecord> composite_record(const Record& record);

}  // namespace dmc::rengine::profiles::dmc3::fx::effect_bank
