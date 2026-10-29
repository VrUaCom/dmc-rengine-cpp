#include "dmc_rengine/profiles/dmc3/em000_events.hpp"

namespace dmc::rengine::profiles::dmc3::em000_events {
namespace {

constexpr HandlerSpawn kHandlerSpawns[] = {
#define EM000_HANDLER_ROW(code, type_1a, kind, id, place, joint, mode, scale) \
    {code, type_1a != 0, kind, id, static_cast<Place>(place), joint, mode, scale},
#include "em000_events.inc"
#undef EM000_HANDLER_ROW
};

constexpr CommandEvent kCommandEvents[] = {
#define EM000_COMMAND_EVENT(table, command, frame, code) {table, command, frame, code},
#include "em000_events.inc"
#undef EM000_COMMAND_EVENT
};

constexpr ClassTables kClassTables[] = {
#define EM000_CLASS_TABLES(name, ...) {name, {{__VA_ARGS__}}},
#include "em000_events.inc"
#undef EM000_CLASS_TABLES
};

} // namespace

std::span<const HandlerSpawn> handler_spawns() noexcept { return kHandlerSpawns; }
std::span<const CommandEvent> command_events() noexcept { return kCommandEvents; }
std::span<const ClassTables> class_tables() noexcept { return kClassTables; }

} // namespace dmc::rengine::profiles::dmc3::em000_events
