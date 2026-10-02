#include "dmc_rengine/spider/texture_reencode_workflow.hpp"

#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/core/no_replace_publication.hpp"

#include <array>
#include <cstring>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>

namespace dmc::rengine::spider::tarantula {
namespace {

namespace dmc3 = profiles::dmc3;
namespace bcn = codecs::dds_bcn;

enum : crusader::OperationId {
    k_acquire = 1U,
    k_inspect,
    k_transform,
    k_assemble,
    k_validate,
    k_publish,
};

struct State final {
    // inputs
    std::optional<std::filesystem::path> input_path;
    std::span<const std::byte> input_bytes;
    const TextureReencodeRequest* request{};
    // acquire
    std::vector<std::byte> owned;
    std::span<const std::byte> source;
    // inspect
    bool pac{};
    std::vector<dmc3::PacSlotExtent> slots;
    std::vector<std::uint32_t> texture_slots;  // PAC slots to transform
    // transform: one cell per PAC slot, written only by that slot's step,
    // so the transforms run concurrently; assemble gathers them in order.
    struct SlotOutcome final {
        bool done{};
        TextureReencodeStep step;
        std::optional<dmc3::PacSlotReplacement> replacement;
        std::vector<dmc3::ReencodedTexture> textures;
    };
    std::vector<SlotOutcome> outcomes;
    std::vector<dmc3::PacSlotReplacement> replacements;
    dmc3::PayloadReencodeResult single;
    // assemble / validate
    dmc3::TextureReencodeResult result;
    std::vector<TextureReencodeStep> steps;
};

void step(State& s, std::string name, bool ok, std::string detail = {}) {
    s.steps.push_back({std::move(name), ok, std::move(detail)});
}

bool acquire(State& s, std::uint32_t) noexcept {
    try {
        if (s.input_path) {
            std::ifstream in(*s.input_path, std::ios::binary);
            if (!in) {
                step(s, "acquire", false, "cannot read " + s.input_path->string());
                return false;
            }
            const std::vector<char> raw_bytes((std::istreambuf_iterator<char>(in)), {});
            s.owned.resize(raw_bytes.size());
            std::memcpy(s.owned.data(), raw_bytes.data(), raw_bytes.size());
            s.source = {s.owned.data(), s.owned.size()};
        } else {
            s.source = s.input_bytes;
        }
        step(s, "acquire", !s.source.empty(), std::to_string(s.source.size()) + " bytes");
        return !s.source.empty();
    } catch (...) {
        step(s, "acquire", false, "read failed");
        return false;
    }
}

bool inspect(State& s, std::uint32_t) noexcept {
    try {
        const auto pac_slots = dmc3::read_pac_slots(s.source);
        s.pac = pac_slots.has_value();
        if (!s.pac) {
            const bool texture = dmc3::holds_textures(s.source);
            step(s, "inspect", texture, texture ? "single texture payload" : "no texture");
            return texture;
        }
        s.slots = *pac_slots;
        s.outcomes.resize(s.slots.size());
        const int only = s.request->options.pac_slot;
        for (std::uint32_t i = 0U; i < s.slots.size(); ++i) {
            if (s.slots[i].offset == 0U || (only >= 0 && static_cast<std::uint32_t>(only) != i)) continue;
            const auto extent = s.source.subspan(static_cast<std::size_t>(s.slots[i].offset),
                                                 static_cast<std::size_t>(s.slots[i].size));
            if (!dmc3::texture_payload(extent).empty()) s.texture_slots.push_back(i);
        }
        std::ostringstream d;
        d << "PAC, " << s.slots.size() << " slots, texture slots:";
        for (const auto i : s.texture_slots) d << ' ' << i;
        step(s, "inspect", !s.texture_slots.empty(), d.str());
        return !s.texture_slots.empty();
    } catch (...) {
        step(s, "inspect", false, "inspection failed");
        return false;
    }
}

bool transform(State& s, std::uint32_t operand) noexcept {
    try {
        if (!s.pac) {
            s.single = dmc3::reencode_payload(s.source, s.request->options);
            step(s, "transform", s.single.ok, s.single.ok ? std::to_string(s.single.textures.size()) + " texture(s)"
                                                          : s.single.detail);
            return s.single.ok;
        }
        const auto slot = operand;
        const auto& e = s.slots.at(slot);
        auto& outcome = s.outcomes.at(slot);
        const auto payload = dmc3::texture_payload(
            s.source.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size)));
        auto encoded = dmc3::reencode_payload(payload, s.request->options, static_cast<int>(slot));
        outcome.done = true;
        outcome.step = {"transform[" + std::to_string(slot) + "]", encoded.ok,
                        encoded.ok ? std::to_string(encoded.textures.size()) + " texture(s)" : encoded.detail};
        if (!encoded.ok) return false;
        outcome.textures = std::move(encoded.textures);
        outcome.replacement = dmc3::PacSlotReplacement{slot, std::move(encoded.bytes)};
        return true;
    } catch (...) {
        step(s, "transform", false, "transform failed");
        return false;
    }
}

bool assemble(State& s, std::uint32_t) noexcept {
    try {
        if (!s.pac) {
            s.result.container = s.single.container;
            s.result.bytes = std::move(s.single.bytes);
            s.result.textures = std::move(s.single.textures);
        } else {
            s.result.container = dmc3::ReencodeContainer::pac;
            for (auto& outcome : s.outcomes) {
                if (!outcome.done) continue;
                s.result.textures.insert(s.result.textures.end(), outcome.textures.begin(), outcome.textures.end());
                if (outcome.replacement) s.replacements.push_back(std::move(*outcome.replacement));
            }
            auto rebuilt = dmc3::replace_pac_slots(s.source, s.replacements);
            if (!rebuilt) {
                step(s, "assemble", false, "PAC rebuild failed");
                return false;
            }
            s.result.bytes = std::move(*rebuilt);
        }
        step(s, "assemble", true, std::to_string(s.result.bytes.size()) + " bytes");
        return true;
    } catch (...) {
        step(s, "assemble", false, "assemble failed");
        return false;
    }
}

bool validate(State& s, std::uint32_t) noexcept {
    try {
        const std::span<const std::byte> out{s.result.bytes.data(), s.result.bytes.size()};
        const auto formats = dmc3::list_textures(out);
        std::size_t matching = 0U;
        for (const auto& d : formats) matching += d.format == s.request->options.format ? 1U : 0U;
        if (matching < s.result.textures.size()) {
            step(s, "validate", false, "a texture did not take the requested format");
            return false;
        }
        if (s.pac) {
            const auto new_slots = dmc3::read_pac_slots(out);
            if (!new_slots || new_slots->size() != s.slots.size()) {
                step(s, "validate", false, "PAC slot table changed shape");
                return false;
            }
            for (std::uint32_t i = 0U; i < s.slots.size(); ++i) {
                bool touched = false;
                for (const auto& r : s.replacements) touched = touched || r.slot == i;
                if (touched || s.slots[i].offset == 0U) continue;
                const auto& a = s.slots[i];
                const auto& b = (*new_slots)[i];
                // A moved slot keeps its bytes; its extent may gain up to 15
                // alignment zeros when the slot after it moved.
                const auto n = static_cast<std::size_t>(std::min(a.size, b.size));
                if (std::memcmp(s.source.data() + a.offset, out.data() + b.offset, n) != 0) {
                    step(s, "validate", false, "untouched PAC slot " + std::to_string(i) + " changed");
                    return false;
                }
            }
        }
        step(s, "validate", true, std::to_string(matching) + " texture(s) in " +
                                       std::string(bcn::format_name(s.request->options.format)));
        return true;
    } catch (...) {
        step(s, "validate", false, "validation failed");
        return false;
    }
}

bool publish(State& s, std::uint32_t) noexcept {
    try {
        const auto& target = s.request->output;
        if (target.empty()) {
            step(s, "publish", true, "in memory");
            return true;
        }
        const std::span<const std::byte> bytes{s.result.bytes.data(), s.result.bytes.size()};
        if (s.request->replace_existing) {
            std::ofstream out(target, std::ios::binary | std::ios::trunc);
            out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            const bool ok = static_cast<bool>(out);
            step(s, "publish", ok, ok ? target.string() + " (replaced)" : "cannot write " + target.string());
            return ok;
        }
        const auto published = core::publish_bytes_no_replace(
            target, bytes, [](const std::filesystem::path& staged) {
                std::ifstream in(staged, std::ios::binary);
                const std::vector<char> raw_bytes((std::istreambuf_iterator<char>(in)), {});
                return dmc3::holds_textures(std::span<const std::byte>{
                    reinterpret_cast<const std::byte*>(raw_bytes.data()), raw_bytes.size()});
            });
        step(s, "publish", published.ok(),
             published.ok() ? target.string() : std::string(core::to_string(published.status)) + ": " + published.detail);
        return published.ok();
    } catch (...) {
        step(s, "publish", false, "publication failed");
        return false;
    }
}

constexpr std::array k_bindings{
    crusader::bind<State, &acquire>(k_acquire),
    crusader::bind<State, &inspect>(k_inspect),
    crusader::bind_concurrent<State, &transform>(k_transform),
    crusader::bind<State, &assemble>(k_assemble),
    crusader::bind<State, &validate>(k_validate),
    crusader::bind<State, &publish>(k_publish),
};

TextureReencodeWorkflowResult run(State& state) {
    TextureReencodeWorkflowResult out;
    // Plan 1: acquire -> inspect. The transform fan-out depends on what
    // inspect finds, so the rest is compiled after it (Tarantula: choose
    // the operation, then run it).
    crusader::Builder front;
    const auto acquired = front.add(k_acquire, 0U, crusader::Domain::io, {}, "acquire");
    front.add(k_inspect, 0U, crusader::Domain::cpu, {acquired}, "inspect");
    out.report = crusader::execute(front.build(), k_bindings, state);
    std::string failed{front.failed_label(out.report)};
    if (out.report.ok()) {
        // Plan 2: transform[slot] x N -> assemble -> validate -> publish.
        crusader::Builder back;
        std::vector<crusader::Node> transforms;
        const std::vector<std::uint32_t> transform_slots =
            state.pac ? state.texture_slots : std::vector<std::uint32_t>{0U};
        for (const auto slot : transform_slots) {
            transforms.push_back(back.add(k_transform, slot, crusader::Domain::cpu, {}, "transform"));
        }
        const auto assembled = back.add_span(k_assemble, 0U, crusader::Domain::cpu, transforms, "assemble");
        const auto validated = back.add(k_validate, 0U, crusader::Domain::cpu, {assembled}, "validate");
        back.add(k_publish, 0U, crusader::Domain::io, {validated}, "publish");
        // The transforms are independent (one PAC slot each) and run on all
        // cores; the other steps run alone, in order.
        out.report = crusader::execute_parallel(back.build(), k_bindings, state);
        failed = back.failed_label(out.report);
    }
    // Transform receipts in slot order, between inspect and assemble.
    {
        std::vector<TextureReencodeStep> ordered;
        for (auto& st : state.steps) {
            if (st.name == "assemble" || st.name == "validate" || st.name == "publish") break;
            ordered.push_back(std::move(st));
        }
        const auto head = ordered.size();
        for (auto& outcome : state.outcomes) {
            if (outcome.done) ordered.push_back(std::move(outcome.step));
        }
        for (std::size_t i = head; i < state.steps.size(); ++i) ordered.push_back(std::move(state.steps[i]));
        state.steps = std::move(ordered);
    }
    out.steps = std::move(state.steps);
    out.result = std::move(state.result);
    out.ok = out.report.ok();
    out.result.ok = out.ok;
    if (out.ok) {
        std::ostringstream d;
        d << out.result.textures.size() << " texture(s) -> " << bcn::format_name(state.request->options.format)
          << (state.request->options.force_dx10 || !bcn::legacy_fourcc(state.request->options.format) ? " DX10" : "")
          << ", " << state.source.size() << " -> " << out.result.bytes.size() << " bytes";
        out.detail = d.str();
        out.result.detail = out.detail;
    } else {
        // The step the executor stopped at, with that step's own receipt.
        out.detail = failed.empty() ? std::string(crusader::to_string(out.report.status)) : failed;
        if (!out.steps.empty() && !out.steps.back().ok) out.detail = out.steps.back().name + ": " + out.steps.back().detail;
        out.result.bytes.clear();
    }
    return out;
}

}  // namespace

TextureReencodeWorkflowResult run_texture_reencode(const std::filesystem::path& input,
                                                   const TextureReencodeRequest& request) {
    State state;
    state.input_path = input;
    state.request = &request;
    return run(state);
}

TextureReencodeWorkflowResult run_texture_reencode(std::span<const std::byte> input,
                                                   const TextureReencodeRequest& request) {
    State state;
    state.input_bytes = input;
    state.request = &request;
    return run(state);
}

}  // namespace dmc::rengine::spider::tarantula
