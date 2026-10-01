#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// P records (particle emitters). Three definition classes are ported:
//   3 CPtclSprt00  textured camera-facing quads (4 vertices per particle)
//   1 CPtclPoly00  untextured free triangles (3 vertices per particle)
//   4 CPtclLine01  untextured two-segment streaks (4 vertices per particle)
// Classes 0 (Line00), 2 (Poly01), 5 (Line02) and the Poly00 path +0xFA != 1
// are not ported. Reverse authority: dmc3.exe factory 0x140236AA0, per-class
// init / update (vtables 0x1404E29C8, 0x1404E2C68, 0x1404E2728), key track
// 0x1402D30E0 / lerp 0x1402D3200, transform integrator 0x140312260, draw
// composer 0x140312F10; every constant was checked against an emulated run of
// those functions (docs/research/dmc3-particle-p-records-2026-10-01.md).
//
// Every object is a burst: particles spawn once in a hollow box (0x140312450),
// move under friction and gravity, and fade along a 3-segment key track. A
// layer re-draws the emitter's particles with its own local transform, colour
// track and blend.
namespace dmc::rengine::profiles::dmc3::fx::particle {

// One key of a colour track: s16 segment length (ticks) and 4 RGBA groups,
// one per vertex index w of the primitive.
struct TrackKey final {
    std::int16_t duration{};
    std::array<std::uint8_t, 16> color{};
};
struct Track final {
    std::array<TrackKey, 4> keys{};
    bool enabled{};
    bool loop{};
    std::uint8_t ease{};  // table +0x52: 0 one colour, 1 two, 2 four (0x1405CED70)
};

// Local transform integrator inputs (0x140312260): s16 per tick.
struct TransformMotion final {
    std::array<std::int16_t, 3> translation{};  // x 1/16 per tick
    std::array<std::int16_t, 3> rotation{};     // x 2*pi/65536 per tick
    std::array<std::int16_t, 3> scale{};        // x 1/4096 per tick
};

struct LayerDef final {
    std::array<float, 3> translation{};
    std::array<float, 3> rotation{};  // radians, Rz*Ry*Rx
    std::array<float, 3> scale{1.0F, 1.0F, 1.0F};
    TransformMotion motion{};
    Track track{};
    std::uint8_t blend{0x44U};  // layer +0x45: GS ALPHA bits (A,B,C,D) = 0x44 alpha, 0x48 additive
};

struct Def final {
    // Definition class byte (+0x01).
    std::uint8_t cls{3U};
    std::string name;
    std::uint16_t animation{0xFFFFU};  // A record id (Sprt00 def +0x106)
    bool random_frame{};               // Sprt00 +0x128 == 1: one random A frame, no playback
    bool world_gravity{};              // gravity vector given in world space
    std::int32_t life{};               // ticks (def +0x68)
    std::uint8_t blend{0x44U};         // def +0x65: GS ALPHA bits (A,B,C,D); 0x44 alpha, 0x48 additive
    std::array<float, 3> translation{};
    std::array<float, 3> rotation{};
    std::array<float, 3> scale{1.0F, 1.0F, 1.0F};
    TransformMotion motion{};
    Track track{};
    std::array<float, 3> friction{};        // per tick
    std::array<float, 3> friction_decay{};  // per tick
    float gravity{};
    std::array<float, 3> spread{};          // s16 / 16: box edge
    std::array<float, 3> hollow{};          // s16 / 32: inner half extent
    std::array<float, 3> push{};            // s16 / 16
    std::array<float, 3> bias{};            // s16 / 16
    float half_width{};                     // Sprt00: u16 +0x108 / 16
    float half_height{};                    // Sprt00: u16 +0x10A / 16
    float shard{};                          // Poly00: s16 +0xFC / 16, triangle corner spread
    std::array<float, 2> segment{};         // Line01: s16 +0xE4 / +0xE6 / 16, segment lengths
    std::uint32_t count{12U};               // particles
    std::vector<LayerDef> layers;
};

// Parses a record of a ported class (3, 1 with +0xFA == 1, 4).
[[nodiscard]] std::optional<Def> parse(std::span<const std::uint8_t> record);
[[nodiscard]] std::optional<Def> parse_sprt00(std::span<const std::uint8_t> record);
[[nodiscard]] std::optional<Def> parse_poly00(std::span<const std::uint8_t> record);
[[nodiscard]] std::optional<Def> parse_line01(std::span<const std::uint8_t> record);

// Sprite animation timing the particle needs from its A record.
struct Animation final {
    std::uint8_t frame_time{};
    std::uint8_t last_frame{};
    bool loop{};
    std::uint8_t loop_frame{};
    std::uint32_t frame_count{1U};
};

struct Camera final {
    Vec3 right{1.0F, 0.0F, 0.0F};
    Vec3 up{0.0F, 1.0F, 0.0F};
    Vec3 forward{0.0F, 0.0F, 1.0F};
};

// One drawn primitive. Sprt00 quads list the corners bottom-left,
// bottom-right, top-right, top-left of the *image* (atlas) rectangle, so
// uv = (u0,v1),(u1,v1),(u1,v0),(u0,v0). Poly00 triangles repeat the third
// vertex in corners[3]. Line01 emits two lines per particle: corners[0] ->
// corners[1]. `rgba` is the colour of each corner (RGBA bytes).
struct Quad final {
    std::array<Vec3, 4> corners{};
    std::array<std::array<std::uint8_t, 4>, 4> rgba{};
    std::uint32_t layer{};  // 0 = the emitter itself
    bool triangle{};
    bool line{};
    bool additive{};
};

class Simulation final {
public:
    Simulation(const Def& def, const Animation& animation, std::uint32_t seed);

    // One 60 Hz tick (dt 1). Returns false once the effect expired.
    bool update(const Matrix4& world);
    [[nodiscard]] bool expired() const noexcept { return expired_; }
    [[nodiscard]] std::uint32_t frame() const noexcept { return frame_; }

    // Primitives of every layer for the current state.
    void quads(const Matrix4& world, const Camera& camera, std::vector<Quad>* out) const;

    // Test hooks: the emitter state the EXE keeps (object +0xB80 / +0x120).
    struct State final {
        std::vector<std::array<float, 3>> velocity;
        std::vector<std::array<float, 3>> position;  // Sprt00
        // Poly00 uses the first three vertices, Line01 all four (the packet).
        std::vector<std::array<std::array<float, 3>, 4>> vertex;
        std::array<float, 3> translation{}, rotation{}, scale{};
        std::array<std::uint8_t, 16> color{};
        float life{};
    };
    [[nodiscard]] const State& state() const noexcept { return emitter_; }
    void set_particles(std::vector<std::array<float, 3>> position,
                       std::vector<std::array<float, 3>> velocity);
    void set_vertices(std::vector<std::array<std::array<float, 3>, 4>> vertex,
                      std::vector<std::array<float, 3>> velocity);
    [[nodiscard]] const std::vector<State>& layer_states() const noexcept { return layers_; }

private:
    struct TrackState final {
        std::uint8_t index{};
        bool done{};
        float remaining{};
    };
    void step_track(const Track& track, TrackState* state, std::array<std::uint8_t, 16>* color) const;

    Def def_;
    Animation animation_;
    State emitter_;
    TrackState emitter_track_;
    std::vector<State> layers_;
    std::vector<TrackState> layer_tracks_;
    std::array<float, 3> friction_{};
    std::uint32_t frame_{};
    float anim_timer_{};
    bool expired_{};
    std::uint32_t rng_{};
    std::size_t groups_{4U};  // colour groups the class lerps (0x1402D30E0 edx)
};

}  // namespace dmc::rengine::profiles::dmc3::fx::particle
