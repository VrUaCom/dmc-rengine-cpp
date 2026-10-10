#include "dmc_rengine/integration/resource_structure.hpp"

#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/formats/evt.hpp"
#include "dmc_rengine/formats/fon.hpp"
#include "dmc_rengine/formats/stage_cfg.hpp"
#include "dmc_rengine/profiles/dmc3/attachment_tables.hpp"
#include "dmc_rengine/profiles/dmc3/cloth_chain.hpp"
#include "dmc_rengine/profiles/dmc3/collision_shapes.hpp"
#include "dmc_rengine/profiles/dmc3/em000_family_contract.hpp"
#include "dmc_rengine/profiles/dmc3/environment_collision.hpp"
#include "dmc_rengine/profiles/dmc3/fx/effect_bank.hpp"
#include "dmc_rengine/profiles/dmc3/player_param_blocks.hpp"
#include "dmc_rengine/profiles/dmc3/fx/enemy_events.hpp"
#include "dmc_rengine/profiles/dmc3/fx/generator.hpp"
#include "dmc_rengine/profiles/dmc3/fx/particle.hpp"
#include "dmc_rengine/profiles/dmc3/motion_script.hpp"
#include "dmc_rengine/profiles/dmc3/player_attachment_contract.hpp"
#include "dmc_rengine/profiles/dmc3/stage_layout.hpp"
#include "dmc_rengine/profiles/dmc3/uv_scroll.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <numbers>
#include <utility>

namespace dmc::rengine::integration {
namespace {

namespace dmc3 = profiles::dmc3;
namespace motion = profiles::dmc3::motion;
namespace fx = profiles::dmc3::fx;

[[nodiscard]] std::span<const std::uint8_t> u8(std::span<const std::byte> bytes) noexcept {
    return {reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()};
}

[[nodiscard]] std::string_view as_text(std::span<const std::byte> bytes) noexcept {
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

[[nodiscard]] std::string num(double value) {
    char text[32];
    std::snprintf(text, sizeof text, "%g", value);
    return text;
}

template <class Range>
[[nodiscard]] std::string list(const Range& values) {
    std::string out;
    for (const auto v : values) {
        if (!out.empty()) out += ", ";
        out += num(static_cast<double>(v));
    }
    return out;
}

[[nodiscard]] std::string hex(std::uint64_t value) {
    char text[24];
    std::snprintf(text, sizeof text, "0x%llX", static_cast<unsigned long long>(value));
    return text;
}

[[nodiscard]] std::string degrees(const std::array<float, 3>& radians) {
    std::array<float, 3> d{};
    for (std::size_t i = 0U; i < 3U; ++i) d[i] = radians[i] * 180.0F / std::numbers::pi_v<float>;
    return list(d) + "°";
}

[[nodiscard]] std::string_view yes_no(bool value) noexcept { return value ? "yes" : "no"; }

[[nodiscard]] std::string_view blend_name(std::uint8_t blend) noexcept {
    return blend == 0x48U ? "additive" : blend == 0x44U ? "alpha" : "other";
}

// "dir/EM028.pac" -> "em028".
[[nodiscard]] std::string stem_of(std::string_view name) {
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string_view::npos) name.remove_prefix(slash + 1U);
    const auto dot = name.find('.');
    if (dot != std::string_view::npos) name = name.substr(0U, dot);
    std::string out{name};
    for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

void add_section(StructureView& view, StructureSection section) {
    if (view.sections.size() >= kMaxStructureSections) {
        view.truncated = true;
        return;
    }
    view.sections.push_back(std::move(section));
}

// ---- CLT / TSC / EVT ----------------------------------------------------

std::optional<StructureView> read_clt(std::span<const std::byte> bytes, std::string& detail) {
    const auto text = as_text(bytes);
    if (!motion::looks_like_clt(text)) {
        detail = "The text does not open like a cloth definition (';' comment, ClothNo block).";
        return std::nullopt;
    }
    const auto chains = motion::parse_clt(text);
    if (chains.empty()) {
        detail = "No ClothNo block could be read.";
        return std::nullopt;
    }
    static constexpr std::array<std::string_view, 6> axes{"X", "Y", "Z", "NX", "NY", "NZ"};
    StructureView view;
    view.format = "clt";
    view.reader = "profiles::dmc3::motion::parse_clt (runtime parser 0x1402CA345 / 0x1402CA42A)";
    std::size_t bones = 0U;
    for (std::size_t i = 0U; i < chains.size(); ++i) {
        const auto& c = chains[i];
        bones += c.bones.size();
        std::string chain;
        for (const auto& bone : c.bones) {
            if (!chain.empty()) chain += " → ";
            chain += std::to_string(bone.node) + " " + std::string{bone.axis < axes.size() ? axes[bone.axis] : "?"};
        }
        add_section(view, {"Cloth chain " + std::to_string(i) + " · " + std::to_string(c.bones.size()) + " bone(s)",
                           {{"Gravity", list(c.gravity)},
                            {"Spring force", num(c.spring_force)},
                            {"Max speed", num(c.max_speed)},
                            {"Stiffness", num(c.stiffness)},
                            {"Damping", num(c.damping)},
                            {"Wind", list(c.wind)},
                            {"Wind local / parent / type", std::string{yes_no(c.wind_local)} + " / " +
                                                               std::to_string(c.wind_parent) + " / " +
                                                               std::to_string(c.wind_type)},
                            {"Floor level", num(c.floor_level)},
                            {"Limit length", std::string{yes_no(c.limit_length)}},
                            {"Bones (node axis)", chain.empty() ? "none" : chain}}});
    }
    view.summary = std::to_string(chains.size()) + " cloth chain(s), " + std::to_string(bones) + " bone(s).";
    return view;
}

std::optional<StructureView> read_tsc(std::span<const std::byte> bytes, std::string& detail) {
    const auto text = as_text(bytes);
    if (!motion::looks_like_tsc(text)) {
        detail = "The text does not open with the .TSC tag line.";
        return std::nullopt;
    }
    const auto records = motion::parse_tsc(text);
    StructureView view;
    view.format = "tsc";
    view.reader = "profiles::dmc3::motion::parse_tsc (runtime parser 0x14030C1C0)";
    const auto dir = [](std::int16_t d, bool u) -> std::string {
        if (d == 0) return "stay";
        if (u) return d > 0 ? "left" : "right";
        return d > 0 ? "up" : "down";
    };
    for (const auto& r : records) {
        StructureSection section{"Scroll " + std::to_string(r.number) + " · ScrlType " + std::to_string(r.type),
                                 {{"Texture", r.texture < 0 ? std::string{"every mesh"} : std::to_string(r.texture)},
                                  {"Joint", r.joint < 0 ? std::string{"model default"} : std::to_string(r.joint)},
                                  {"Direction U, V", dir(r.direction[0], true) + ", " + dir(r.direction[1], false)},
                                  {"RateUV", list(r.rate)},
                                  {"TimeUV", list(r.time)},
                                  {"InterUV", list(r.interval)}}};
        if (r.has_turn_time) section.rows.push_back({"TurnTimeUV", list(r.turn_time)});
        if (r.has_minimum) section.rows.push_back({"MinimumUV", list(r.minimum)});
        if (r.has_random) section.rows.push_back({"RndUV", list(r.random)});
        add_section(view, std::move(section));
    }
    view.summary = std::to_string(records.size()) +
        " scroll record(s) up to '$' (what follows '$' is never read by the game).";
    return view;
}

std::optional<StructureView> read_evt(std::span<const std::byte> bytes, std::string& detail) {
    namespace evt = formats::evt;
    const auto parsed = evt::Parser::parse(bytes);
    if (!parsed.ok()) {
        detail = parsed.diagnostics.empty() ? std::string{"The EVT reader declined these bytes."}
                                            : parsed.diagnostics.front().message;
        return std::nullopt;
    }
    const auto& doc = parsed.document;
    StructureView view;
    view.format = "evt";
    view.reader = "formats::evt::Parser";
    std::string starts;
    for (const auto offset : doc.stream_offsets) starts += (starts.empty() ? "" : ", ") + hex(offset);
    add_section(view, {"Header",
                       {{"Revision", std::to_string(doc.header.revision)},
                        {"Streams", std::to_string(doc.header.stream_count)},
                        {"Terminal command at", hex(doc.header.terminal_command_offset)},
                        {"Stream starts", starts}}});
    StructureSection commands{"Commands (" + std::to_string(doc.commands.size()) + ")", {}};
    std::size_t named = 0U;
    for (const auto& command : doc.commands) {
        const auto d = command.descriptor();
        named += d.name != "unknown" ? 1U : 0U;
        if (commands.rows.size() >= 4U * kMaxStructureSections) {
            view.truncated = true;
            continue;
        }
        std::string args;
        for (const auto a : command.arguments) args += (args.empty() ? "" : ", ") + hex(a);
        std::string value{d.name};
        if (d.semantic_class != evt::OpcodeSemanticClass::unknown) {
            value += " (" + std::string{evt::to_string(d.semantic_class)} + ", " +
                std::string{evt::to_string(d.evidence)} + ")";
        }
        if (!args.empty()) value += " [" + args + "]";
        commands.rows.push_back({hex(command.offset) + "  op " + hex(command.opcode), std::move(value)});
    }
    add_section(view, std::move(commands));
    view.summary = std::to_string(doc.commands.size()) + " command(s) in " +
        std::to_string(doc.stream_offsets.size()) + " stream(s); " + std::to_string(named) +
        " carry a name from the opcode table.";
    return view;
}

// ---- HITS (stage collision) ---------------------------------------------

std::optional<StructureView> read_hits(std::span<const std::byte> bytes, std::string_view name, std::string& detail) {
    namespace ec = dmc3::environment_collision;
    const auto source = ec::parse(name, 0U, u8(bytes));
    if (!source) {
        detail = "The HITS reader declined these bytes.";
        return std::nullopt;
    }
    StructureView view;
    view.format = "hits";
    view.reader = "profiles::dmc3::environment_collision (cell walk 0x14005E880)";
    const auto v3 = [](const auto& v) { return num(v.x) + ", " + num(v.y) + ", " + num(v.z); };
    add_section(view, {"Grid",
                       {{"Bounds min", v3(source->bounds_min)},
                        {"Bounds max", v3(source->bounds_max)},
                        {"Cells", std::to_string(source->grid_count_x) + " × " + std::to_string(source->grid_count_y) +
                                      " × " + std::to_string(source->grid_count_z)},
                        {"Cell size (raw words)", list(source->cell_size_raw)},
                        {"Cell references", std::to_string(source->cell_reference_count)},
                        {"Triangle-plane records", std::to_string(source->triangles.size())}}});
    const auto kinds = ec::kinds(*source);
    StructureSection kind_rows{"Record kinds (" + std::to_string(kinds.size()) + ", by flags)", {}};
    for (std::size_t i = 0U; i < kinds.size(); ++i) {
        const auto& k = kinds[i];
        kind_rows.rows.push_back({"Kind " + std::to_string(i) + "  flags " + hex(k.flags),
                                  std::to_string(k.count) + " record(s): " + std::to_string(k.floors) + " floor, " +
                                      std::to_string(k.walls) + " wall, " + std::to_string(k.ceilings) +
                                      " ceiling"});
    }
    add_section(view, std::move(kind_rows));
    view.summary = std::to_string(source->triangles.size()) + " triangle-plane record(s) in " +
        std::to_string(kinds.size()) + " kind(s).";
    return view;
}

// ---- Effect bank (PNST manifest + records) --------------------------------

// What one bank record holds, read by the view of its kind. Shared by the
// bank's view and a record opened on its own (`fx-<kind>`).
void add_record_rows(const fx::effect_bank::Record& r, StructureSection& s) {
    namespace eb = fx::effect_bank;
    switch (r.kind) {
    case 'T': {
        const auto dds = eb::texture_dds(r);
        const auto parsed = codecs::dds_bcn::parse(
            std::span<const std::byte>{reinterpret_cast<const std::byte*>(dds.data()), dds.size()});
        if (parsed.ok()) {
            const auto& d = parsed.document;
            s.rows.push_back({"Texture", std::string{codecs::dds_bcn::format_name(d.format)} + ", " +
                                             std::to_string(d.width) + "×" + std::to_string(d.height) + ", " +
                                             std::to_string(d.mip_count) + " mip(s)"});
        }
        break;
    }
    case 'A':
        if (const auto a = eb::sprite_animation(r)) {
            s.rows.push_back({"Texture id", std::to_string(a->texture)});
            s.rows.push_back({"Frames", std::to_string(a->frames.size()) + " × " +
                                            std::to_string(a->frame_time) + " tick(s)" +
                                            (a->loop ? ", loops from frame " + std::to_string(a->loop_frame)
                                                     : std::string{", once"})});
            if (!a->frames.empty()) {
                const auto& f = a->frames.front();
                s.rows.push_back({"First frame (x, y, w, h)", std::to_string(f.x) + ", " + std::to_string(f.y) +
                                                                  ", " + std::to_string(f.w) + ", " +
                                                                  std::to_string(f.h)});
            }
        }
        break;
    case 'E':
        if (const auto e = eb::e_runtime_view(r)) {
            s.rows.push_back({"Mode", std::to_string(e->mode)});
            s.rows.push_back({"Texture id (T)", std::to_string(e->texture_id)});
            s.rows.push_back({"Animation (A)", e->uses_animation ? std::to_string(e->animation_id)
                                                                 : std::string{"none: fixed rectangle"}});
            if (!e->uses_animation) {
                const auto& f = e->rectangle;
                s.rows.push_back({"Rectangle (x, y, w, h)", std::to_string(f.x) + ", " + std::to_string(f.y) +
                                                                ", " + std::to_string(f.w) + ", " +
                                                                std::to_string(f.h)});
            }
        }
        break;
    case 'P':
        if (const auto p = fx::particle::parse(r.bytes)) {
            s.rows.push_back({"Class", std::to_string(p->cls) + (p->name.empty() ? "" : " · " + p->name)});
            s.rows.push_back({"Particles × life", std::to_string(p->count) + " × " + std::to_string(p->life) +
                                                      " tick(s)"});
            s.rows.push_back({"Blend", std::string{blend_name(p->blend)}});
            s.rows.push_back({"Gravity", num(p->gravity) + (p->world_gravity ? " (world)" : " (local)")});
            s.rows.push_back({"Spread / push", list(p->spread) + " / " + list(p->push)});
            if (p->animation != 0xFFFFU) {
                s.rows.push_back({"Sprite animation (A)", std::to_string(p->animation) +
                                                              (p->random_frame ? ", one random frame" : "")});
            }
            if (p->half_width > 0.0F) {
                s.rows.push_back({"Sprite half size", num(p->half_width) + " × " + num(p->half_height)});
            }
            s.rows.push_back({"Layers", std::to_string(p->layers.size())});
        } else if (const auto pv = eb::p_runtime_view(r)) {
            s.rows.push_back({"Version / subtype", std::to_string(pv->version) + " / " +
                                                       std::to_string(pv->subtype) + " (class not ported)"});
        }
        break;
    case 'G':
        if (const auto g = fx::generator::parse(r.bytes)) {
            static constexpr std::array<std::string_view, 4> child{"P", "E", "G", "V"};
            static constexpr std::array<std::string_view, 3> mode{"drift", "C clip", "still"};
            s.rows.push_back({"Motion", std::string{g->motion < mode.size() ? mode[g->motion] : "?"} +
                                            (g->motion == 1U ? " " + std::to_string(g->clip) : std::string{})});
            s.rows.push_back({"Spawns", std::string{g->child_kind < child.size() ? child[g->child_kind] : "?"} +
                                            " " + std::to_string(g->child_id)});
            s.rows.push_back({"First delay / interval", std::to_string(g->first_delay) + " / " +
                                                            std::to_string(g->interval) + " tick(s)" +
                                                            (g->interval_mask != 0U
                                                                 ? " + rand & " + hex(g->interval_mask)
                                                                 : std::string{})});
            s.rows.push_back({"Life", g->endless ? std::string{"endless"} : std::to_string(g->life) + " tick(s)"});
            s.rows.push_back({"Speed / deceleration", num(g->speed) + " / " + num(g->deceleration)});
            s.rows.push_back({"Scale", num(g->scale_start) + " → " + num(g->scale_end)});
            s.rows.push_back({"Follows parent", std::string{yes_no(g->follow_parent)}});
        }
        break;
    case 'C':
        if (const auto c = fx::generator::parse_clip(r.bytes)) {
            s.rows.push_back({"Clip points", std::to_string(c->points.size())});
        }
        break;
    case 'V':
        if (const auto v = eb::v_runtime_view(r)) {
            static constexpr std::array<std::string_view, 4> child{"P", "E", "G", "V"};
            std::string entries;
            for (const auto& e : v->entries) {
                entries += (entries.empty() ? "" : ", ") +
                    std::string{e.dispatch < child.size() ? child[e.dispatch] : "?"} + " " + std::to_string(e.id);
            }
            s.rows.push_back({"Children", std::to_string(v->entries.size()) + ": " + entries});
        }
        break;
    case 'M':
        s.rows.push_back({"Model", r.bytes.size() >= 4U
                                       ? std::string(reinterpret_cast<const char*>(r.bytes.data()), 3U) +
                                             " document + " + std::to_string(r.companion.size()) +
                                             "-byte companion"
                                       : std::string{"empty"}});
        break;
    default:
        break;
    }
}

std::optional<StructureView> read_effect_bank(std::span<const std::byte> bytes, std::string& detail) {
    namespace eb = fx::effect_bank;
    const auto data = u8(bytes);
    if (!eb::looks_like_bank(data)) {
        detail = "Not an effect bank: slot 0 must be a `<kind> <id>` manifest and slot 1 a PNST of records.";
        return std::nullopt;
    }
    const auto bank = eb::parse_bank(data);
    if (!bank) {
        detail = "The effect bank loader port (0x1402C04C0) declined these bytes.";
        return std::nullopt;
    }
    StructureView view;
    view.format = "pnst";
    view.reader = "profiles::dmc3::fx::effect_bank (loader 0x1402C04C0) with the E / P / G / V / A views";
    std::map<char, std::size_t> counts;
    for (const auto& r : bank->records) ++counts[r.kind];
    StructureSection overview{"Bank", {{"Records", std::to_string(bank->records.size())},
                                       {"Inner record slots", std::to_string(bank->record_slots)},
                                       {"Manifest bytes", std::to_string(bank->manifest_bytes)},
                                       {"Manifest ends with '#'", std::string{yes_no(bank->terminated)}}}};
    for (const auto& [kind, count] : counts) {
        overview.rows.push_back({std::string(1, kind) + " · registrar " + hex(eb::registrar(kind)),
                                 std::to_string(count) + " × " + std::string{eb::kind_name(kind)}});
    }
    add_section(view, std::move(overview));

    for (const auto& r : bank->records) {
        StructureSection s{std::string(1, r.kind) + " " + std::to_string(r.id) + " · slot " + std::to_string(r.slot),
                           {{"Bytes", std::to_string(r.bytes.size())}}};
        add_record_rows(r, s);
        add_section(view, std::move(s));
    }
    std::string kinds;
    for (const auto& [kind, count] : counts) kinds += (kinds.empty() ? "" : ", ") + std::to_string(count) + " " + kind;
    view.summary = std::to_string(bank->records.size()) + " effect record(s): " + kinds + ".";
    return view;
}

// One record of an effect bank, opened on its own. Its kind is not in its
// bytes — the bank's manifest letter is what the loader dispatches on — so it
// comes from the format the bank gave it (`fx-e`, `fx-p`, …).
std::optional<StructureView> read_effect_record(std::span<const std::byte> bytes, std::string_view format,
                                                std::string_view name, std::string& detail) {
    namespace eb = fx::effect_bank;
    if (format.size() != 4U || !format.starts_with("fx-")) {
        detail = "Not an effect record format.";
        return std::nullopt;
    }
    const char kind = static_cast<char>(std::toupper(static_cast<unsigned char>(format[3])));
    const auto data = u8(bytes);
    eb::Record record{.kind = kind, .id = 0U, .slot = 0U, .bytes = data, .companion = {}};
    StructureView view;
    view.format = std::string{format};
    view.reader = "profiles::dmc3::fx::effect_bank, the " + std::string(1, kind) +
        " view (registrar " + hex(eb::registrar(kind)) + ")";
    StructureSection section{name.empty() ? std::string(1, kind) + " record" : std::string{name},
                             {{"Kind", std::string{eb::kind_name(kind)}}, {"Bytes", std::to_string(bytes.size())}}};
    if (kind == 'M' && bytes.size() == 16U) {
        section.rows.push_back({"Role", "the 16-byte companion the M registrar takes with the model before it"});
    } else {
        add_record_rows(record, section);
    }
    const bool decoded = section.rows.size() > 2U;
    add_section(view, std::move(section));
    view.summary = std::string{eb::kind_name(kind)} + ", named by the bank's manifest" +
        (decoded ? "." : "; this kind's fields are not decoded yet.");
    return view;
}

// ---- Character collision tables ----------------------------------------------

std::optional<StructureView> read_collision_shapes(std::span<const std::byte> bytes, std::string_view format,
                                                   std::string& detail) {
    namespace col = dmc3::collision;
    if (!col::looks_like_shape_table(u8(bytes))) {
        detail = "Not a shape table: whole 80-byte records, type byte 0..6, finite values.";
        return std::nullopt;
    }
    const auto shapes = col::parse_shapes(u8(bytes));
    StructureView view;
    view.format = std::string{format};
    view.reader = "profiles::dmc3::collision::parse_shapes (ICollisionHandle 0x1404C65A0, setup 0x14005C260)";
    std::size_t spheres = 0U, boxes = 0U, capsules = 0U;
    for (std::size_t i = 0U; i < shapes.size(); ++i) {
        const auto& sh = shapes[i];
        StructureSection s{"Shape " + std::to_string(i), {}};
        switch (static_cast<col::ShapeType>(sh.type)) {
        case col::ShapeType::Sphere:
            ++spheres;
            s.title += " · sphere";
            s.rows = {{"Centre", list(sh.a)}, {"Radius", num(sh.radius)}};
            break;
        case col::ShapeType::Box:
            ++boxes;
            s.title += " · box";
            s.rows = {{"Centre", list(sh.a)}, {"Euler degrees", list(sh.b)}, {"Half size", list(sh.size)}};
            break;
        case col::ShapeType::Capsule:
            ++capsules;
            s.title += " · capsule";
            s.rows = {{"End A", list(sh.a)}, {"End B", list(sh.b)}, {"Radius", num(sh.radius)}};
            break;
        default:
            s.title += " · type " + std::to_string(sh.type);
            s.rows = {{"Values +0x10..", list(std::span<const float>{sh.raw.data(), 8U})}};
            break;
        }
        add_section(view, std::move(s));
    }
    view.summary = std::to_string(shapes.size()) + " shape record(s): " + std::to_string(spheres) + " sphere, " +
        std::to_string(boxes) + " box, " + std::to_string(capsules) + " capsule.";
    return view;
}

// ---- Motion script --------------------------------------------------------------

std::optional<StructureView> read_motion_script(std::span<const std::byte> bytes, std::string& detail) {
    const auto data = u8(bytes);
    const auto file = motion::MotionScriptFile::parse(data);
    if (!file || !motion::MotionScriptFile::looks_like(data)) {
        detail = "Not a motion script: three u16 table offsets whose banks end with 0xFFFF and play a motion.";
        return std::nullopt;
    }
    StructureView view;
    view.format = "motion-script";
    view.reader = "profiles::dmc3::motion::MotionScriptFile (bind 0x1400594B0, interpreter 0x140058FE0)";
    std::size_t scripts = 0U;
    for (std::size_t bank = 0U; bank < file->bank_count(); ++bank) {
        const auto count = file->script_count(bank);
        scripts += count;
        StructureSection s{"Bank " + std::to_string(bank) + " · " + std::to_string(count) + " action(s)", {}};
        for (std::size_t action = 0U; action < count; ++action) {
            if (s.rows.size() >= 64U) {
                s.rows.push_back({"…", std::to_string(count - action) + " more action(s)"});
                break;
            }
            const auto summary = file->summarize(bank, action);
            if (!summary) continue;
            std::string value = "plays " + std::to_string(summary->play_bank) + "/" +
                std::to_string(summary->play_index) + ", " + std::to_string(summary->instructions) + " op(s), " +
                std::to_string(summary->waits) + " wait(s)";
            if (summary->last_frame != 0U) value += ", to frame " + std::to_string(summary->last_frame);
            if (summary->loops) value += ", loops";
            if (summary->hands_over) value += ", hands over";
            if (!summary->states.empty()) value += ", " + std::to_string(summary->states.size()) + " weapon state(s)";
            const auto resources = file->resources(bank, action);
            if (!resources.empty()) {
                value += "; MOT";
                for (const auto& r : resources) value += " " + std::to_string(r.group()) + ":" + std::to_string(r.slot());
            }
            s.rows.push_back({"Action " + std::to_string(action), std::move(value)});
        }
        add_section(view, std::move(s));
    }
    view.summary = std::to_string(file->bank_count()) + " bank(s), " + std::to_string(scripts) + " action(s)" +
        (file->nested() ? " (player layout, nested tables)" : " (enemy layout)") +
        (file->has_resources() ? "; " + std::to_string(file->resource_ids().size()) + " motion id(s) in table B."
                               : ".");
    return view;
}

// ---- Stage layout text ("# GAME") --------------------------------------------

std::optional<StructureView> read_stage_layout(std::span<const std::byte> bytes, std::string& detail) {
    namespace sl = dmc3::stage_layout;
    const auto text = as_text(bytes);
    if (text.find("# GAME") == std::string_view::npos) {
        detail = "Only a stage's `# GAME` layout text has a structure reader; this text has no `# GAME` block.";
        return std::nullopt;
    }
    const auto layout = sl::parse_game(text);
    if (layout.sets.empty() && !layout.has_camera) {
        detail = "The `# GAME` block holds no SET block and no CONFIG camera.";
        return std::nullopt;
    }
    StructureView view;
    view.format = "txt";
    view.reader = "profiles::dmc3::stage_layout::parse_game (record parser near 0x140247720, BREAK 0x14024A540)";
    const auto v3 = [](const dmc3::Vec3& v) { return num(v.x) + ", " + num(v.y) + ", " + num(v.z); };
    if (layout.has_camera) add_section(view, {"CONFIG", {{"cam_init", v3(layout.camera)}}});
    std::map<std::string, std::size_t> kinds;
    for (std::size_t i = 0U; i < layout.sets.size(); ++i) {
        const auto& set = layout.sets[i];
        ++kinds[set.kind.empty() ? std::string{"(none)"} : set.kind];
        StructureSection s{"SET " + std::to_string(i) + (set.kind.empty() ? std::string{} : " · " + set.kind),
                           {{"Model", set.model < 0 ? std::string{"none"} : std::to_string(set.model)},
                            {"Position", v3(set.pos)},
                            {"Rotation (degrees, X then Y then Z)", v3(set.rot)},
                            {"Scale", v3(set.scale)}}};
        for (const auto& uv : set.uv) {
            s.rows.push_back({"UV scroll (part, texture, U, V)", list(uv)});
        }
        if (set.effect_kind != 0) {
            s.rows.push_back({"Effect", std::string(1, set.effect_kind) + " " + std::to_string(set.effect_id) +
                                            " at " + v3(set.effect_pos)});
        }
        if (set.kind == "BREAK") {
            static constexpr std::array<std::string_view, 3> remain{"fades out (alpha 64 - 2 per tick)", "stays (on)",
                                                                   "stays (on2)"};
            s.rows.push_back({"Broken model", set.broken_model < 0 ? std::string{"none: removed"}
                                                                   : std::to_string(set.broken_model)});
            if (set.broken_effect_kind != 0) {
                s.rows.push_back({"Break effect (once)", std::string(1, set.broken_effect_kind) + " " +
                                                             std::to_string(set.broken_effect_id) + " at " +
                                                             v3(set.broken_effect_pos)});
            }
            s.rows.push_back({"Remain", std::string{set.remain < remain.size() ? remain[set.remain] : "?"}});
        }
        add_section(view, std::move(s));
    }
    std::string by_kind;
    for (const auto& [kind, count] : kinds) by_kind += (by_kind.empty() ? "" : ", ") + std::to_string(count) + " " + kind;
    view.summary = std::to_string(layout.sets.size()) + " placed object(s)" +
        (by_kind.empty() ? std::string{} : ": " + by_kind) + (layout.has_camera ? "; initial camera set." : ".");
    return view;
}

// ---- Enemy effect events (em000..em008) ------------------------------------------

[[nodiscard]] std::string_view placement_name(fx::enemy::Placement placement) noexcept {
    switch (placement) {
    case fx::enemy::Placement::GivenPosition: return "at the given position";
    case fx::enemy::Placement::ActorPositionYaw: return "at the actor, its yaw";
    case fx::enemy::Placement::AttachedObject: return "attached to joint";
    case fx::enemy::Placement::ObjectTranslation: return "at the translation of joint";
    case fx::enemy::Placement::ActorFields: return "from actor fields";
    }
    return "?";
}

void add_enemy_events(StructureView& view, std::string_view stem) {
    namespace en = fx::enemy;
    if (!stem.starts_with("em00") || stem.size() != 5U || stem[4] < '0' || stem[4] > '8') return;
    StructureSection events{"Effect events (handler " + hex(en::kEventHandler) + ", codes below " +
                                hex(en::kFirstControlCode) + ")",
                            {}};
    for (const auto& c : en::event_cases()) {
        std::string value;
        for (const auto& spawn : c.spawns) {
            if (!value.empty()) value += "; ";
            value += std::string(1, fx::runtime::kind_letter(spawn.kind)) + std::to_string(spawn.id) + " " +
                std::string{placement_name(spawn.placement)};
            if (spawn.placement == en::Placement::AttachedObject ||
                spawn.placement == en::Placement::ObjectTranslation) {
                value += " " + std::to_string(spawn.object);
            }
            if (spawn.scale != 1.0F) value += " × " + num(spawn.scale);
        }
        std::string label = "Code " + hex(c.code);
        if (c.enemy_type != 0xFFU) label += " (type " + hex(c.enemy_type) + " only)";
        events.rows.push_back({std::move(label), value.empty() ? std::string{"no spawn"} : value});
    }
    add_section(view, std::move(events));
    if (stem > "em004") return;
    StructureSection death{"Death schedule (0x140095E85, started by control code " + hex(en::kDeathControlCode) + ")",
                           {}};
    for (const auto& step : en::em000_death_schedule()) {
        std::string codes;
        for (std::uint8_t i = 0U; i < step.count && i < step.codes.size(); ++i) {
            codes += (codes.empty() ? "" : ", ") + hex(step.codes[i]);
        }
        death.rows.push_back({"Tick " + num(step.tick), "code " + codes});
    }
    add_section(view, std::move(death));
    StructureSection attacks{"Attack effects (CComEm000, frame-gated code 3)", {}};
    for (const auto& a : en::em000_attack_events()) {
        attacks.rows.push_back({"Action " + std::to_string(a.bank) + "/" + std::to_string(a.action),
                                "frame " + num(a.frame) + ": code " + hex(a.code)});
    }
    add_section(view, std::move(attacks));
}

// ---- Character archive slot roles ---------------------------------------------

[[nodiscard]] std::uint32_t pac_slot_count(std::span<const std::byte> bytes) noexcept {
    if (bytes.size() < 8U || std::memcmp(bytes.data(), "PAC\0", 4U) != 0) return 0U;
    std::uint32_t count = 0U;
    std::memcpy(&count, bytes.data() + 4U, 4U);
    return count;
}

std::optional<StructureView> read_pac_roles(std::span<const std::byte> bytes, std::string_view name,
                                            std::string& detail) {
    const auto stem = stem_of(name);
    const auto slots = pac_slot_count(bytes);
    StructureView view;
    view.format = "pac";
    view.reader = "profiles::dmc3 character contracts (attachment_tables, em000_family, player_attachment)";
    StructureSection roles{"Slot roles of " + stem + ".pac (" + std::to_string(slots) + " slots)", {}};

    if (stem == "pl000" || stem == "pl001" || stem == "pl002" || stem == "pl003") {
        roles.rows = {
            {"Slot " + std::to_string(motion::kPlayerTextureSlot), "PTX: body and coat textures"},
            {"Slot " + std::to_string(motion::kPlayerBodySlot), "MOD: body"},
            {"Slot 5", "motion script (IPlayer)"},
            {"Slots 6 / 7", "attack index / shape table (0x1401EEFD1)"},
            {"Slot " + std::to_string(motion::kPlayerCoatSlot), "MOD: coat, root on body joint 3"},
            {"Slot " + std::to_string(motion::kPlayerCoatClothSlot), "CLT: coat chain"},
            {"Slot " + std::to_string(motion::kPlayerCoatConstraintSlot),
             "optional CCNS coat constraints (mod; the retail game never reads it)"},
        };
    } else if (stem.starts_with("plwp_")) {
        if (const auto record = dmc3::player_attachment::weapon_record_for_stem(stem)) {
            roles.rows = {
                {"Class", std::string{record->class_name}},
                {"Attach joint", std::to_string(record->joint)},
                {"Translation", list(record->translation)},
                {"Rotation", degrees(record->rotation_xyz_radians)},
                {"Attach table / state-0 record", hex(record->attach_table_va) + " / " + hex(record->state0_record_va)},
            };
            if (const auto second = dmc3::player_attachment::second_part_for_class(record->class_name)) {
                roles.rows.push_back({"Second part", "node " + std::to_string(second->second_node) + " on joint " +
                                                         std::to_string(second->joint) + ", " +
                                                         list(second->translation)});
            }
        }
    } else if (stem == "em028") {
        roles.rows = {{"Slot 1", "MOD: body"}, {"Slots 2 / 3", "motion PACs (0x140131037)"},
                      {"Slot 9", "effect bank (mode 2)"}, {"Slot 10", "motion script"},
                      {"Slots 11 / 12", "attack index / shape table (0x140130FA8)"}};
        for (const auto& part : motion::kEnemyPartConstraints) {
            if (part.pac_stem != stem) continue;
            std::string links;
            for (const auto& c : part.constraints) {
                links += (links.empty() ? "" : ", ") + std::to_string(c.child_node) + "→" + std::to_string(c.host_node);
            }
            roles.rows.push_back({"Slot " + std::to_string(part.part_slot), "part on body joints: " + links});
        }
    } else if (stem == "em000") {
        namespace fam = dmc3::em000_family;
        roles.rows = {{"Slot " + std::to_string(fam::motion_bank_slot), "motion PAC (0x1400982D9)"},
                      {"Slot 38", "motion script"},
                      {"Slots 39 / 40", "attack index / shape table (0x14009823F)"},
                      {"Slot " + std::to_string(fam::effect_bank_slot), "effect bank (mode 2)"}};
        for (const auto& cls : fam::classes) {
            std::string value = "body slot " + std::to_string(cls.body_slot) + ", weapon slots " +
                std::to_string(cls.weapon_slot_variant01) + "/" + std::to_string(cls.weapon_slot_variant23);
            for (std::size_t i = 0U; i < cls.cloth_count; ++i) {
                value += ", cloth " + std::to_string(cls.cloth[i].model_slot) + "←CLT " +
                    std::to_string(cls.cloth[i].clt_slot);
            }
            roles.rows.push_back({std::string{cls.class_name}, std::move(value)});
        }
    }
    for (const auto& cloth : motion::kEnemyClothSources) {
        if (cloth.pac_stem == stem) {
            roles.rows.push_back({"Slot " + std::to_string(cloth.model_slot),
                                  "cloth model driven by CLT slot " + std::to_string(cloth.clt_slot)});
        }
    }
    for (const auto& tsc : motion::kTscSources) {
        if (tsc.pac_stem != stem) continue;
        std::string models;
        for (std::uint32_t i = 0U; i < tsc.model_count; ++i) {
            models += (models.empty() ? "" : ", ") + std::to_string(tsc.model_slots[i]);
        }
        roles.rows.push_back({"Slot " + std::to_string(tsc.tsc_slot), "TSC scrolling model slot(s) " + models});
    }
    if (const auto bank = motion::player_motion_bank(name)) {
        roles.rows.push_back({"Motion bank", "player motion PAC group " + std::to_string(*bank)});
    }
    if (!roles.rows.empty()) {
        view.summary = std::to_string(roles.rows.size()) + " slot role(s) recovered from the executable for " + stem + ".";
        add_section(view, std::move(roles));
    }
    add_enemy_events(view, stem);
    if (view.sections.empty()) {
        detail = "No character contract names the slots of '" + stem + ".pac'.";
        return std::nullopt;
    }
    if (view.summary.empty()) {
        view.summary = "Effect events of " + stem + " recovered from the executable.";
    } else if (view.sections.size() > 1U) {
        view.summary += " Effect events listed.";
    }
    return view;
}

// ---- Stage configuration: POS, EVE, CAM, ITM, STE, EST, SEF ------------------------------------
//
// Purpose settled, schema open (format_registry limitations): the rows say
// what the bytes hold and name a field only where the files show what it is.

template <typename R>
[[nodiscard]] bool declined(const R& result, std::string_view what, std::string& detail) {
    if (result.ok()) return false;
    detail = result.diagnostics.empty() ? "The " + std::string{what} + " reader declined these bytes."
                                        : result.diagnostics.front().message;
    return true;
}

template <typename R>
void add_notes(StructureView& view, const R& result) {
    StructureSection notes{"Reader notes", {}};
    for (const auto& diagnostic : result.diagnostics) {
        notes.rows.push_back({hex(diagnostic.offset) + "  " + diagnostic.code, diagnostic.message});
    }
    if (!notes.rows.empty()) add_section(view, std::move(notes));
}

[[nodiscard]] std::string point(const formats::stage_cfg::Vec3& v) {
    return num(v.x) + ", " + num(v.y) + ", " + num(v.z);
}

std::optional<StructureView> read_pos(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_pos(bytes);
    if (declined(result, "POS", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "pos";
    view.reader = "formats::stage_cfg::read_pos (StageCfg placement; schema read from the files)";
    add_section(view, {"Header",
                       {{"Version", std::to_string(doc.header.version)},
                        {"Positions", std::to_string(doc.header.count)},
                        {"Room for", std::to_string(doc.capacity) + " records of 48 bytes"}}});
    for (std::size_t index = 0U; index < doc.records.size(); ++index) {
        const auto& record = doc.records[index];
        StructureSection section{"Position " + std::to_string(index) + "  @" + hex(record.offset),
                                 {{"Position", point(record.position)},
                                  {"Heading", num(record.heading_degrees) + "°"}}};
        if (record.tag != 0U) section.rows.push_back({"Field +0x00", std::to_string(record.tag)});
        add_section(view, std::move(section));
    }
    add_notes(view, result);
    view.summary = std::to_string(doc.records.size()) + " position(s) with a heading, of " +
        std::to_string(doc.capacity) + " the file has room for.";
    return view;
}

std::optional<StructureView> read_eve(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_eve(bytes);
    if (declined(result, "EVE", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "eve";
    view.reader = "formats::stage_cfg::read_eve (StageCfg event volumes; schema read from the files)";
    add_section(view, {"Header",
                       {{"Version", hex(doc.header.version)}, {"Volumes", std::to_string(doc.header.count)}}});
    std::map<std::uint32_t, std::size_t> kinds;
    for (std::size_t index = 0U; index < doc.records.size(); ++index) {
        const auto& record = doc.records[index];
        ++kinds[record.kind];
        StructureSection section{"Volume " + std::to_string(index) + "  @" + hex(record.offset),
                                 {{"Kind (+0x04)", std::to_string(record.kind)},
                                  {"Argument (+0x08)", std::to_string(record.argument) + "  (u16 " +
                                       std::to_string(record.argument & 0xFFFFU) + ", " +
                                       std::to_string(record.argument >> 16U) + ")"},
                                  {"Field +0x00", std::to_string(record.field0)},
                                  {"Floor height", num(record.area.corners[0].y)},
                                  {"Extent above it", num(record.extent)}}};
        for (std::size_t corner = 0U; corner < 4U; ++corner) {
            section.rows.push_back({"Corner " + std::to_string(corner), point(record.area.corners[corner])});
        }
        if (!record.area.level) section.rows.push_back({"Note", "the corners are not on one level"});
        add_section(view, std::move(section));
    }
    add_notes(view, result);
    view.summary = std::to_string(doc.records.size()) + " event volume(s): a floor quadrilateral and the height "
        "it reaches, in " + std::to_string(kinds.size()) + " kind(s).";
    return view;
}

std::optional<StructureView> read_cam(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_cam(bytes);
    if (declined(result, "CAM", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "cam";
    view.reader = "formats::stage_cfg::read_cam (StageCfg camera; parts found by shape, record layout open)";
    add_section(view, {"Header",
                       {{"Version", std::to_string(doc.header.version)},
                        {"Records", std::to_string(doc.header.count)},
                        {"Paths", std::to_string(doc.paths.size())},
                        {"Areas", std::to_string(doc.areas.size())},
                        {"Coefficient runs", std::to_string(doc.coefficient_runs.size())},
                        {"Bytes not explained", std::to_string(doc.unexplained_bytes)}}});
    // Two paths of one length followed by a coefficient per point read as one
    // camera move: where the camera goes, and where it looks.
    std::size_t pairs = 0U;
    for (std::size_t index = 0U; index < doc.paths.size(); ++index) {
        const auto& path = doc.paths[index];
        std::string title = "Path " + std::to_string(index) + "  @" + hex(path.offset);
        if (index + 1U < doc.paths.size() && doc.paths[index + 1U].points.size() == path.points.size()) {
            title += "  (first of a pair)";
            ++pairs;
        } else if (index > 0U && doc.paths[index - 1U].points.size() == path.points.size()) {
            title += "  (second of a pair)";
        }
        StructureSection section{title, {}};
        for (std::size_t p = 0U; p < path.points.size(); ++p) {
            section.rows.push_back({"Point " + std::to_string(p), point(path.points[p])});
        }
        add_section(view, std::move(section));
    }
    for (std::size_t index = 0U; index < doc.areas.size(); ++index) {
        const auto& area = doc.areas[index];
        StructureSection section{"Area " + std::to_string(index) + "  @" + hex(area.offset),
                                 {{"Floor height", num(area.quad.corners[0].y)}, {"Extent", num(area.extent)}}};
        for (std::size_t corner = 0U; corner < 4U; ++corner) {
            section.rows.push_back({"Corner " + std::to_string(corner), point(area.quad.corners[corner])});
        }
        add_section(view, std::move(section));
    }
    StructureSection runs{"Coefficient runs", {}};
    for (const auto& run : doc.coefficient_runs) {
        runs.rows.push_back({hex(run.offset), std::to_string(run.count) + " × " + num(run.value)});
    }
    if (!runs.rows.empty()) add_section(view, std::move(runs));
    add_notes(view, result);
    view.summary = std::to_string(doc.paths.size()) + " path(s), " + std::to_string(pairs) +
        " of them opening a pair of equal length; " + std::to_string(doc.areas.size()) + " area(s). " +
        "The record layout is open: these are the parts found by their shape.";
    return view;
}


std::optional<StructureView> read_itm(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_itm(bytes);
    if (declined(result, "ITM", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "itm";
    view.reader = "formats::stage_cfg::read_itm (StageCfg static item placements)";
    add_section(view, {"Header",
                       {{"Version", std::to_string(doc.header.version)},
                        {"Items", std::to_string(doc.header.count)}}});
    for (std::size_t index = 0U; index < doc.records.size(); ++index) {
        const auto& record = doc.records[index];
        add_section(view, {"Item " + std::to_string(index) + "  @" + hex(record.offset),
                           {{"Item id", std::to_string(record.item_id)},
                            {"Position", point(record.position)},
                            {"Rotation about Y", num(record.rotation_y)}}});
    }
    add_notes(view, result);
    view.summary = doc.records.empty() ? std::string{"No items placed in this stage."}
                                       : std::to_string(doc.records.size()) + " item(s) placed: an id, a position "
                                             "and a turn about the vertical.";
    return view;
}

std::optional<StructureView> read_ste(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_ste(bytes);
    if (declined(result, "STE", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "ste";
    view.reader = "formats::stage_cfg::read_ste (StageCfg scene/effect transforms; schema read from the files)";
    add_section(view, {"Header",
                       {{"Version", std::to_string(doc.header.version)},
                        {"Transforms", std::to_string(doc.header.count)}}});
    for (std::size_t index = 0U; index < doc.records.size(); ++index) {
        const auto& record = doc.records[index];
        add_section(view, {"Transform " + std::to_string(index) + "  @" + hex(record.offset),
                           {{"Field +0x00", std::to_string(record.kind)},
                            {"Number (+0x02)", std::to_string(record.number)},
                            {"Position", point(record.position)},
                            {"Rotation", point(record.rotation_degrees) + " °"},
                            {"Scale", point(record.scale)}}});
    }
    add_notes(view, result);
    view.summary = std::to_string(doc.records.size()) + " transform(s): position, rotation in degrees and scale, "
        "each under a number.";
    return view;
}

constexpr std::array<std::string_view, formats::stage_cfg::est_modes> k_est_modes{
    "Easy", "Normal", "Hard", "Very Hard", "Dante Must Die"};

std::optional<StructureView> read_est(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_est(bytes);
    if (declined(result, "EST", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "est";
    view.reader = "formats::stage_cfg::read_est (StageCfg slot 9; commands read from the files)";
    add_section(view, {"Header",
                       {{"Version", std::to_string(doc.header.version)},
                        {"Rows", std::to_string(doc.header.count)},
                        {"Table", hex(doc.table_offset) + "  (" + std::to_string(doc.header.count) +
                                      " × 5 offsets)"},
                        {"Programs", std::to_string(doc.programs.size())},
                        {"Bytes not read", std::to_string(doc.unexplained_bytes)}}});
    StructureSection table{"Programs by difficulty", {}};
    std::size_t varying = 0U;
    for (std::size_t row = 0U; row < doc.rows.size(); ++row) {
        std::string cells;
        bool differs = false;
        for (std::size_t mode = 0U; mode < k_est_modes.size(); ++mode) {
            const auto index = doc.rows[row][mode];
            if (!cells.empty()) cells += "  ";
            cells += std::string{k_est_modes[mode].substr(0, mode == 4U ? 3U : k_est_modes[mode].size())} + " " +
                (index < 0 ? std::string{"—"} : "#" + std::to_string(index));
            differs = differs || doc.rows[row][mode] != doc.rows[row][0];
        }
        if (differs) ++varying;
        table.rows.push_back({"Row " + std::to_string(row), cells});
    }
    add_section(view, std::move(table));
    for (std::size_t index = 0U; index < doc.programs.size(); ++index) {
        const auto& program = doc.programs[index];
        StructureSection section{"Program #" + std::to_string(index) + "  @" + hex(program.offset) + "  (" +
                                     std::to_string(program.commands.size()) + " commands)",
                                 {}};
        for (const auto& command : program.commands) {
            std::string arguments;
            for (const auto argument : command.arguments) {
                if (!arguments.empty()) arguments += ", ";
                arguments += std::to_string(argument);
            }
            section.rows.push_back({hex(command.offset) + "  command " + std::to_string(command.code),
                                    arguments.empty() ? std::string{"—"} : arguments});
        }
        if (!program.terminated) section.rows.push_back({"Note", "no zero word ends this program"});
        add_section(view, std::move(section));
    }
    add_notes(view, result);
    view.summary = std::to_string(doc.rows.size()) + " row(s) of a program per difficulty mode, " +
        std::to_string(varying) + " of them different between modes; " + std::to_string(doc.programs.size()) +
        " distinct program(s) of commands. What each command does is open.";
    return view;
}

std::optional<StructureView> read_sef(std::span<const std::byte> bytes, std::string& detail) {
    const auto result = formats::stage_cfg::read_sef(bytes);
    if (declined(result, "SEF", detail)) return std::nullopt;
    const auto& doc = result.document;
    StructureView view;
    view.format = "sef";
    view.reader = "formats::stage_cfg::read_sef (stage effect companion; section table read, sections open)";
    add_section(view, {"Header",
                       {{"Sections", std::to_string(doc.sections_declared)},
                        {"Field +0x06", std::to_string(doc.field6)}}});
    StructureSection sections{"Sections", {}};
    for (std::size_t index = 0U; index < doc.sections.size(); ++index) {
        const auto& section = doc.sections[index];
        sections.rows.push_back({"Section " + std::to_string(index) + "  @" + hex(section.offset),
                                 std::to_string(section.size) + " bytes; table value " +
                                     std::to_string(section.value)});
    }
    add_section(view, std::move(sections));
    add_notes(view, result);
    view.summary = std::to_string(doc.sections.size()) + " section(s) named by the table at +0x08; "
        "what each holds is open.";
    return view;
}

// ---- Player parameter blocks (pl0NN slots 9, 10, 11) ------------------------

[[nodiscard]] float f32_at(std::span<const std::byte> bytes, std::size_t at) noexcept {
    float value = 0.0F;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

void add_float_grid(StructureView& view, std::span<const std::byte> bytes, std::size_t from) {
    StructureSection grid{"Values by offset", {}};
    for (std::size_t row = from; row + 4U <= bytes.size(); row += 16U) {
        std::string values;
        for (std::size_t at = row; at < row + 16U && at + 4U <= bytes.size(); at += 4U) {
            if (!values.empty()) values += "   ";
            values += num(f32_at(bytes, at));
        }
        grid.rows.push_back({hex(row), values});
    }
    add_section(view, std::move(grid));
}

std::optional<StructureView> read_player_params(std::span<const std::byte> bytes, std::string_view format,
                                                std::string& detail) {
    namespace pp = dmc3::player_params;
    const bool pairs_block = format == "player-pairs";
    const auto pairs = pairs_block ? pp::leading_pairs(bytes) : 0U;
    if (bytes.empty() || bytes.size() % 4U != 0U || (pairs_block && pairs == 0U)) {
        detail = "Not a player parameter block: whole 32-bit values" +
            std::string{pairs_block ? ", opening with u16 pairs." : "."};
        return std::nullopt;
    }
    StructureView view;
    view.format = std::string{format};
    view.reader = "profiles::dmc3::player_params (CPlDante init 0x140212C5C keeps slots 9/10/11)";
    const auto reads = pp::known_reads(bytes.size());
    StructureSection overview{"Block", {{"Bytes", std::to_string(bytes.size())},
                                        {"Values", std::to_string((bytes.size() - pairs * 4U) / 4U) + " floats"}}};
    if (pairs_block) overview.rows.push_back({"Leading pairs", std::to_string(pairs) + " × (u16, u16)"});
    add_section(view, std::move(overview));
    if (!reads.empty()) {
        StructureSection known{"Read by the executable", {}};
        for (const auto& read : reads) {
            known.rows.push_back({"+" + hex(read.offset) + " = " + num(f32_at(bytes, read.offset)),
                                  hex(read.address) + ": " + std::string{read.note}});
        }
        add_section(view, std::move(known));
    }
    if (pairs_block) {
        StructureSection list{"Pairs", {}};
        for (std::size_t index = 0U; index < pairs; ++index) {
            std::uint16_t first = 0U, second = 0U;
            std::memcpy(&first, bytes.data() + index * 4U, 2U);
            std::memcpy(&second, bytes.data() + index * 4U + 2U, 2U);
            list.rows.push_back({std::to_string(index), std::to_string(first) + ", " + std::to_string(second)});
        }
        add_section(view, std::move(list));
    }
    add_float_grid(view, bytes, pairs * 4U);
    view.summary = pairs_block
        ? std::to_string(pairs) + " u16 pairs, then " + std::to_string((bytes.size() - pairs * 4U) / 4U) +
              " floats. No reader of this block has been found yet."
        : std::to_string(bytes.size() / 4U) + " float parameters, " + std::to_string(reads.size()) +
              " of them with a known read site; the rest are listed by offset.";
    return view;
}

// ---- FON: the game's bitmap fonts ---------------------------------------------

[[nodiscard]] std::string_view unicode_block(std::uint8_t high) noexcept {
    if (high == 0x00U) return "Basic Latin and Latin-1";
    if (high == 0x01U) return "Latin Extended";
    if (high == 0x03U) return "Greek";
    if (high == 0x04U) return "Cyrillic";
    if (high == 0x20U) return "General Punctuation";
    if (high == 0x21U) return "Letterlike Symbols and Arrows";
    if (high == 0x25U) return "Box Drawing and Geometric Shapes";
    if (high == 0x26U) return "Miscellaneous Symbols";
    if (high == 0x30U) return "CJK Symbols, Hiragana and Katakana";
    if (high >= 0x4EU && high <= 0x9FU) return "CJK Unified Ideographs";
    if (high >= 0xACU && high <= 0xD7U) return "Hangul Syllables";
    if (high == 0xFFU) return "Halfwidth and Fullwidth Forms";
    return "Other";
}

std::optional<StructureView> read_font(std::span<const std::byte> bytes, std::string& detail) {
    namespace fon = formats::fon;
    const auto result = fon::read(bytes);
    if (!result.ok()) {
        detail = result.diagnostics.empty() ? "Not a FON font." : result.diagnostics.front().message;
        return std::nullopt;
    }
    const auto& doc = *result.document;
    StructureView view;
    view.format = "fon";
    view.reader = "formats::fon (layout read from the four retail fonts)";
    add_section(view, {"Font",
                       {{"Glyphs", std::to_string(doc.glyph_count)},
                        {"Cell", std::to_string(doc.width) + " × " + std::to_string(doc.height) + " px, 1 bit"},
                        {"Glyph bytes", std::to_string(doc.glyph_bytes) + " (" + std::to_string(doc.row_bytes) +
                                            " per row)"},
                        {"Pages", std::to_string(doc.pages) + " of 256 characters"},
                        {"Glyphs start at", hex(doc.glyph_offset)}}});
    // Characters per Unicode block, in the order the pages cover them.
    std::map<std::string_view, std::pair<std::uint32_t, std::string>> blocks;
    std::vector<std::string_view> order;
    for (const auto& character : doc.characters) {
        const auto block = unicode_block(static_cast<std::uint8_t>(character.code >> 8U));
        auto [entry, inserted] = blocks.try_emplace(block, 0U, std::string{});
        if (inserted) order.push_back(block);
        ++entry->second.first;
    }
    StructureSection coverage{"Characters by block", {}};
    for (const auto block : order) {
        coverage.rows.push_back({std::string{block}, std::to_string(blocks[block].first)});
    }
    add_section(view, std::move(coverage));
    StructureSection ranges{"Pages", {}};
    for (const auto high : doc.blocks) {
        std::uint32_t mapped = 0U;
        for (const auto& character : doc.characters) mapped += (character.code >> 8U) == high ? 1U : 0U;
        char label[16];
        std::snprintf(label, sizeof label, "U+%02X00..%02XFF", high, high);
        ranges.rows.push_back({label, std::to_string(mapped) + " glyph(s)"});
        if (ranges.rows.size() >= 24U && doc.blocks.size() > 25U) {
            ranges.rows.push_back({"…", std::to_string(doc.blocks.size() - ranges.rows.size()) + " more page(s)"});
            break;
        }
    }
    add_section(view, std::move(ranges));
    // The first glyphs, their ink measured (the advance is not stored).
    StructureSection sample{"First characters", {}};
    for (std::size_t index = 0U; index < doc.characters.size() && sample.rows.size() < 16U; ++index) {
        const auto& character = doc.characters[index];
        char code[12];
        std::snprintf(code, sizeof code, "U+%04X", character.code);
        const auto ink = fon::ink_span(doc, bytes, character.number);
        sample.rows.push_back({code, "glyph " + std::to_string(character.number) + ", ink " +
                                         (ink ? std::to_string(ink->first) + ".." + std::to_string(ink->last)
                                              : std::string{"none"})});
    }
    add_section(view, std::move(sample));
    view.summary = std::to_string(doc.glyph_count) + " glyphs of " + std::to_string(doc.width) + " × " +
        std::to_string(doc.height) + " px over " + std::to_string(doc.pages) +
        " page(s) of characters; the image shows them in code order.";
    return view;
}

// "so-volume" is the classifier's older name for the same 80-byte shape table
// when every record is a sphere or a segment (em000 slot 40); a table with a
// box record is typed "collision-shapes".
constexpr std::array<std::string_view, 28> k_formats{
    "clt", "tsc", "evt", "hits", "pnst", "collision-shapes", "so-volume", "motion-script", "pac", "txt",
    "pos", "eve", "cam", "itm", "ste", "est", "sef",
    "fx-a", "fx-c", "fx-e", "fx-g", "fx-m", "fx-p", "fx-t", "fx-v", "player-params", "player-pairs", "fon"};

} // namespace

std::span<const std::string_view> structure_formats() noexcept { return k_formats; }

bool has_structure(std::string_view format) noexcept {
    return std::find(k_formats.begin(), k_formats.end(), format) != k_formats.end();
}

std::optional<StructureView> read_structure(std::string_view format, std::span<const std::byte> bytes,
                                            std::string_view name, std::string& detail) {
    try {
        if (format == "clt") return read_clt(bytes, detail);
        if (format == "tsc") return read_tsc(bytes, detail);
        if (format == "evt") return read_evt(bytes, detail);
        if (format == "hits") return read_hits(bytes, name, detail);
        if (format == "pnst") return read_effect_bank(bytes, detail);
        if (format == "collision-shapes" || format == "so-volume") return read_collision_shapes(bytes, format, detail);
        if (format == "motion-script") return read_motion_script(bytes, detail);
        if (format == "pac") return read_pac_roles(bytes, name, detail);
        if (format == "txt") return read_stage_layout(bytes, detail);
        if (format == "pos") return read_pos(bytes, detail);
        if (format == "eve") return read_eve(bytes, detail);
        if (format == "cam") return read_cam(bytes, detail);
        if (format == "itm") return read_itm(bytes, detail);
        if (format == "ste") return read_ste(bytes, detail);
        if (format == "est") return read_est(bytes, detail);
        if (format == "sef") return read_sef(bytes, detail);
        if (format.starts_with("fx-")) return read_effect_record(bytes, format, name, detail);
        if (format == "player-params" || format == "player-pairs") return read_player_params(bytes, format, detail);
        if (format == "fon") return read_font(bytes, detail);
    } catch (const std::exception& exception) {
        detail = exception.what();
        return std::nullopt;
    }
    detail = "No structure reader is registered for '" + std::string{format} + "'.";
    return std::nullopt;
}

} // namespace dmc::rengine::integration
