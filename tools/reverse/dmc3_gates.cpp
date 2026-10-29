// The DMC3 gates 0-48 that can be read off the canonical dmc3.exe, checked
// with the core's own PE reader, x86 length decoder, code graph and RTTI
// scanner: the first lifted function, the CRT and PTX manager entries, the
// scene factory, and the whole light and material path.
//
// Each check names its gate and reads the image: an instruction's operands,
// a direct call's target, a function's switch table, a vtable's RTTI class or
// a constant in .rdata. Evidence and statuses: docs/research/dmc3-gates-00-48-2026-10-29.md
// and docs/research/dmc3-light-pipeline-gates-2026-10-29.md.
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/code_graph.hpp"
#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/exe/rtti_scanner.hpp"
#include "dmc_rengine/exe/x86_decoder.hpp"
#include "dmc_rengine/profiles/dmc3/effect_spawn_sites.hpp"
#include "dmc_rengine/profiles/dmc3/em000_events.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace dmc::rengine;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

namespace {

constexpr auto kCanonical = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
constexpr U64 kBase = 0x140000000ULL;

// x86-64 register numbers as the decoder reports them.
enum Reg : std::uint8_t { rax = 0, rcx = 1, rdx = 2, rbx = 3, rsp = 4, rbp = 5, rsi = 6, rdi = 7 };

std::string hex(U64 v) {
    std::ostringstream s;
    s << "0x" << std::uppercase << std::hex << v;
    return s.str();
}

// ---- Instruction predicates (also exercised by --self-test) -----------------

bool is_call_to(const exe::X86Instruction& in, U64 at, U64 target) {
    return in.flow == exe::X86Flow::call_direct &&
           at + in.length + static_cast<U64>(static_cast<std::int64_t>(in.branch_displacement)) == target;
}

bool is_jump_to(const exe::X86Instruction& in, U64 at, U64 target) {
    return in.flow == exe::X86Flow::jump_direct &&
           at + in.length + static_cast<U64>(static_cast<std::int64_t>(in.branch_displacement)) == target;
}

// lea dest, [base + disp]
bool is_lea(const exe::X86Instruction& in, std::uint8_t dest, std::uint8_t base, std::int32_t disp) {
    return !in.two_byte_opcode && in.opcode == 0x8DU && in.reg_operand == dest && in.memory_base == base &&
           in.displacement == disp;
}

// mov dword/byte [reg + disp], imm
bool is_store_imm(const exe::X86Instruction& in, std::uint8_t opcode, std::int32_t disp, std::int64_t imm) {
    return !in.two_byte_opcode && in.opcode == opcode && in.modrm_mod != 3U && in.displacement == disp &&
           in.immediate == imm;
}

// ---- The image ----------------------------------------------------------------

struct Image {
    std::vector<std::byte> bytes;
    exe::PeImage pe;
    exe::PeFunctionTable functions;
    exe::CodeGraph graph;
    exe::RttiScanResult rtti;
    std::map<U32, const exe::FunctionWalk*> walks;
    // Every direct call target and taken address in the graph: how a leaf
    // function (no unwind entry) is known to be an entry.
    std::set<U32> referenced_entries;

    [[nodiscard]] std::optional<std::size_t> offset(U64 va) const {
        if (va < pe.image_base) return std::nullopt;
        const auto file = pe.rva_to_file_offset(static_cast<U32>(va - pe.image_base));
        if (!file) return std::nullopt;
        return static_cast<std::size_t>(*file);
    }
    [[nodiscard]] std::vector<std::uint8_t> read(U64 va, std::size_t n) const {
        const auto at = offset(va);
        if (!at || *at + n > bytes.size()) throw std::runtime_error("unbacked VA " + hex(va));
        std::vector<std::uint8_t> out(n);
        for (std::size_t i = 0; i < n; ++i) out[i] = std::to_integer<std::uint8_t>(bytes[*at + i]);
        return out;
    }
    [[nodiscard]] U64 uint(U64 va, std::size_t n) const {
        const auto b = read(va, n);
        U64 v = 0;
        for (std::size_t i = 0; i < n; ++i) v |= U64{b[i]} << (8U * i);
        return v;
    }
    [[nodiscard]] std::optional<exe::X86Instruction> decode(U64 va) const {
        const auto at = offset(va);
        if (!at) return std::nullopt;
        return exe::X86LengthDecoder::decode(bytes, *at);
    }
    // The primary function whose unwind ranges hold `va`.
    [[nodiscard]] std::optional<U64> function_of(U64 va) const {
        const auto rva = static_cast<U32>(va - pe.image_base);
        for (const auto& range : functions.functions) {
            if (rva >= range.begin_rva && rva < range.end_rva) return pe.image_base + range.primary_begin_rva;
        }
        return std::nullopt;
    }
    [[nodiscard]] const exe::FunctionWalk* walk(U64 va) const {
        const auto it = walks.find(static_cast<U32>(va - pe.image_base));
        return it == walks.end() ? nullptr : it->second;
    }
    [[nodiscard]] bool calls(U64 function, U64 target) const {
        const auto* w = walk(function);
        if (w == nullptr) return false;
        const auto rva = static_cast<U32>(target - pe.image_base);
        return std::binary_search(w->call_targets.begin(), w->call_targets.end(), rva) ||
               std::find(w->external_jump_targets.begin(), w->external_jump_targets.end(), rva) !=
                   w->external_jump_targets.end();
    }
    // Every decoded instruction of a function, with its VA. A leaf function
    // has no unwind entry and so no walk: it is decoded straight from its
    // entry to its first return or jump.
    void each_instruction(U64 function, const std::function<void(U64, const exe::X86Instruction&)>& visit) const {
        const auto* w = walk(function);
        if (w == nullptr) {
            each_in_range(function, function + 0x100U, visit, true);
            return;
        }
        for (const auto rva : w->instruction_starts) {
            const U64 va = pe.image_base + rva;
            if (const auto in = decode(va)) visit(va, *in);
        }
    }
    // Instructions decoded in sequence over [begin, end).
    void each_in_range(U64 begin, U64 end, const std::function<void(U64, const exe::X86Instruction&)>& visit,
                       bool stop_at_exit = false) const {
        for (U64 va = begin; va < end;) {
            const auto in = decode(va);
            if (!in) return;
            visit(va, *in);
            if (stop_at_exit && (in->flow == exe::X86Flow::return_ || in->flow == exe::X86Flow::jump_direct ||
                                 in->flow == exe::X86Flow::jump_indirect)) {
                return;
            }
            va += in->length;
        }
    }
    // The VA of an 8-byte pointer to `target` in a non-code section (a vtable,
    // a callback or an _initterm table), if any.
    [[nodiscard]] std::optional<U64> pointer_to(U64 target) const {
        for (const auto& section : pe.sections) {
            if ((section.characteristics & 0x20000000U) != 0U) continue;  // executable
            const std::size_t end = std::min<std::size_t>(bytes.size(), std::size_t{section.raw_offset} + section.raw_size);
            for (std::size_t at = section.raw_offset; at + 8U <= end; at += 8U) {
                U64 v = 0;
                for (std::size_t i = 0; i < 8U; ++i) v |= U64{std::to_integer<std::uint8_t>(bytes[at + i])} << (8U * i);
                if (v == target) return pe.image_base + section.virtual_address + (at - section.raw_offset);
            }
        }
        return std::nullopt;
    }
    [[nodiscard]] std::string class_of_vtable(U64 vtable) const {
        const auto rva = static_cast<U32>(vtable - pe.image_base);
        for (const auto& c : rtti.classes) {
            for (const auto& v : c.vtables) {
                if (v.vtable_rva == rva) return c.display_name;
            }
        }
        return {};
    }
    [[nodiscard]] const exe::RttiClass* rtti_class(std::string_view name) const {
        for (const auto& c : rtti.classes) {
            if (c.display_name == name) return &c;
        }
        return nullptr;
    }
};

// ---- Checks ---------------------------------------------------------------------

struct Check {
    std::string gate;
    std::string id;
    bool pass{};
    std::string detail;
};

class Gates {
public:
    explicit Gates(const Image& image) : im_(image) {}

    std::vector<Check> run() {
        foundation();
        material();
        selector();
        packing();
        lig2_object();
        owners_and_rtti();
        switch_function();
        accessor();
        light_lists();
        path();
        model_constants();
        stage_path();
        vertex_formats();
        effect_catalogs();
        return std::move(out_);
    }

private:
    const Image& im_;
    std::vector<Check> out_;

    void add(std::string gate, std::string id, bool pass, std::string detail) {
        out_.push_back({std::move(gate), std::move(id), pass, std::move(detail)});
    }

    // An entry: a primary unwind range starts there, or, for a leaf function
    // that has none, the code calls it or takes its address.
    void function_start(const std::string& gate, const std::string& id, U64 va) {
        const auto f = im_.function_of(va);
        if (f) {
            add(gate, id, *f == va, hex(va) + " in function " + hex(*f));
            return;
        }
        if (im_.referenced_entries.count(static_cast<U32>(va - kBase)) != 0U) {
            add(gate, id, true, hex(va) + " leaf (no unwind entry), called or taken");
            return;
        }
        const auto pointer = im_.pointer_to(va);
        add(gate, id, pointer.has_value(),
            hex(va) + (pointer ? " leaf, pointed to from data at " + hex(*pointer) : " outside .pdata, unreferenced"));
    }

    void call_at(const std::string& gate, const std::string& id, U64 site, U64 target) {
        const auto in = im_.decode(site);
        add(gate, id, in && is_call_to(*in, site, target), hex(site) + " call " + hex(target));
    }

    void calls(const std::string& gate, const std::string& id, U64 function, U64 target) {
        add(gate, id, im_.calls(function, target), hex(function) + " calls " + hex(target));
    }

    void lea_at(const std::string& gate, const std::string& id, U64 site, std::uint8_t dest, std::uint8_t base,
                std::int32_t disp) {
        const auto in = im_.decode(site);
        add(gate, id, in && is_lea(*in, dest, base, disp), hex(site) + " lea [base+" + hex(static_cast<U32>(disp)) + "]");
    }

    void store_in(const std::string& gate, const std::string& id, U64 function, std::uint8_t opcode,
                  std::int32_t disp, std::int64_t imm) {
        std::optional<U64> found;
        im_.each_instruction(function, [&](U64 va, const exe::X86Instruction& in) {
            if (!found && is_store_imm(in, opcode, disp, imm)) found = va;
        });
        add(gate, id, found.has_value(),
            hex(function) + " stores " + hex(static_cast<U64>(imm)) + " at +" + hex(static_cast<U32>(disp)) +
                (found ? " (" + hex(*found) + ")" : ""));
    }

    void rtti(const std::string& gate, U64 vtable, const std::string& name) {
        const auto found = im_.class_of_vtable(vtable);
        add(gate, "rtti." + name, found == name, hex(vtable) + " -> " + (found.empty() ? "?" : found));
    }

    void bytes_at(const std::string& gate, const std::string& id, U64 va, const std::vector<std::uint8_t>& want) {
        const auto got = im_.read(va, want.size());
        std::ostringstream s;
        for (const auto b : got) s << std::hex << std::uppercase << (b < 16 ? "0" : "") << int(b) << ' ';
        add(gate, id, got == want, hex(va) + ": " + s.str());
    }

    // Gates 0-12: the first lifted function, the CRT initializers, the PTX
    // manager entries and the scene factory.
    void foundation() {
        bytes_at("0", "lift.0x14002AB40", 0x14002AB40ULL,
                 {0x84, 0xD2, 0x74, 0x08, 0x83, 0x89, 0x28, 0x02, 0x00, 0x00, 0x04, 0xC3, 0x83, 0xA1, 0x28, 0x02, 0x00,
                  0x00, 0xFB, 0xC3});
        function_start("1", "crt.initialize-cb9ed0", 0x140021A20ULL);
        function_start("1", "crt.initialize-cf2d90", 0x1400236A0ULL);
        bytes_at("1", "crt.cleanup-cb9ed0-ret0", 0x14034EA50ULL, {0xC2, 0x00, 0x00});
        bytes_at("1", "crt.cleanup-cf2d90-ret0", 0x14034EB50ULL, {0xC2, 0x00, 0x00});
        for (const U64 entry : {0x140314CB0ULL, 0x140314C50ULL, 0x140315150ULL, 0x140314E00ULL, 0x140314FA0ULL,
                                0x140315180ULL, 0x140314D00ULL, 0x140314D60ULL}) {
            function_start("2", "ptx-manager." + hex(entry), entry);
        }
        const auto* factory = im_.walk(0x140240090ULL);
        add("12", "scene-factory.ten-selectors", factory != nullptr && factory->switch_targets.size() == 10U,
            "0x140240090 table 0x1402402B8: " + std::to_string(factory ? factory->switch_targets.size() : 0U) +
                " targets");
        calls("12", "main.arena-initializer", 0x1402C5DF0ULL, 0x140030190ULL);
    }

    // Gates 17-19: the material state's functions.
    void material() {
        function_start("17", "material.clamp-register", 0x1402F9890ULL);
        function_start("17", "material.mod-postload", 0x1402FE3B0ULL);
        function_start("18", "material.hd-sampler-bridge", 0x14002BC30ULL);
        function_start("19", "material.alpha-control-copy", 0x1403029E0ULL);
    }

    // Gate 22: selection and score.
    void selector() {
        function_start("22", "selector.entry", 0x1402EE560ULL);
        function_start("22", "score.entry", 0x1402EE2A0ULL);
        calls("22", "score.dot3", 0x1402EE2A0ULL, 0x140030D30ULL);
    }

    // Gate 23: CDrawSCM packing.
    void packing() {
        calls("23", "cdrawscm.selects", 0x1402FD040ULL, 0x1402EE560ULL);
        calls("23", "cdrawscm.packs", 0x1402FD040ULL, 0x1402FC850ULL);
        calls("47", "pack.copy4x4", 0x1402FC850ULL, 0x140030D00ULL);
        calls("47", "pack.dot3", 0x1402FC850ULL, 0x140030D30ULL);
        calls("47", "pack.normalize", 0x1402FC850ULL, 0x140330390ULL);
        calls("47", "pack.transpose", 0x1402FC850ULL, 0x140031240ULL);
    }

    // Gates 31-32: the LIG2 object (CLightStatic) and its two owners.
    void lig2_object() {
        call_at("31", "lig2-ctor.call-owner-a", 0x14023EB43ULL, 0x14023ECB0ULL);
        call_at("31", "lig2-ctor.call-owner-b", 0x14023EFEAULL, 0x14023ECB0ULL);
        lea_at("32", "owner-a.member-0x4078", 0x14023EB3CULL, rcx, rdi, 0x4078);
        lea_at("32", "owner-b.member-0x1D80", 0x14023EFE3ULL, rcx, rdi, 0x1D80);
        // add rcx, 0x28: the records (cleared by the call that follows).
        const auto record_base = im_.decode(0x14023ECE3ULL);
        add("32", "lig2-ctor.records-at-0x28",
            record_base && record_base->opcode == 0x83U && record_base->modrm_reg == 0U &&
                record_base->rm_operand == rcx && record_base->immediate == 0x28,
            "0x14023ECE3 add rcx, 0x28");
        // lea rcx, [rdi+0x928]: the 48 entries, built by the vector constructor.
        lea_at("32", "lig2-ctor.entries-at-0x928", 0x14023ECF8ULL, rcx, rdi, 0x928);
        const auto* ctor = im_.walk(0x14023ECB0ULL);
        const auto entry_ctor = static_cast<U32>(0x1400F7FF0ULL - kBase);
        add("46", "lig2-ctor.entry-constructor",
            ctor != nullptr && std::binary_search(ctor->taken_addresses.begin(), ctor->taken_addresses.end(), entry_ctor),
            "0x14023ECB0 passes 0x1400F7FF0 (entry constructor, a leaf)");
        // lea rcx, [rdi+0x940], then each entry's +0x18 gets its record, step 0x20.
        lea_at("32", "lig2-ctor.record-pointers-at-0x940", 0x14023ED16ULL, rcx, rdi, 0x940);
        bool step = false;
        im_.each_in_range(0x14023ED16ULL, 0x14023ED30ULL, [&](U64, const exe::X86Instruction& in) {
            step = step || is_lea(in, rcx, rcx, 0x20);
        });
        add("32", "lig2-ctor.entry-step-0x20", step, "lea rcx, [rcx+0x20] after 0x14023ED16");
    }

    // Gates 33-35, 46 v0.1/v0.6: owners' constructors and the classes.
    void owners_and_rtti() {
        const auto owner_a = im_.function_of(0x14023EB43ULL);
        add("33", "owner-a.constructor", owner_a && *owner_a == 0x14023EA50ULL,
            "call 0x14023EB43 in " + (owner_a ? hex(*owner_a) : std::string{"?"}));
        const auto owner_b = im_.function_of(0x14023EFEAULL);
        add("46", "owner-b.constructor", owner_b && *owner_b == 0x14023EEF0ULL,
            "call 0x14023EFEA in " + (owner_b ? hex(*owner_b) : std::string{"?"}));
        rtti("34", 0x1404E3238ULL, "CSceneGame");
        rtti("34", 0x1404E31C8ULL, "CGameW");
        rtti("46", 0x1404E3128ULL, "CLightStatic");
        rtti("46", 0x1404E32C8ULL, "CSceneGameMain");
        rtti("46", 0x1404E3118ULL, "CLightMgr");
        rtti("46", 0x1404E3108ULL, "CChain");
        const auto* scene = im_.rtti_class("CSceneGame");
        bool has_cscene = false;
        if (scene != nullptr) {
            for (const auto& base : scene->hierarchy) has_cscene = has_cscene || base.display_name == "CScene";
        }
        add("35", "rtti.CSceneGame-base-CScene", has_cscene, "CSceneGame hierarchy lists CScene");
        add("46", "clightstatic.slot0-destructor", im_.uint(0x1404E3128ULL, 8) == 0x14023FCA0ULL,
            "[0x1404E3128] = " + hex(im_.uint(0x1404E3128ULL, 8)));
        add("38", "pdata.entries", im_.functions.functions.size() == 12235U,
            std::to_string(im_.functions.functions.size()) + " RUNTIME_FUNCTION entries");
    }

    // Gates 39-43: the step machine 0x1402AA2B0 and its jump table.
    void switch_function() {
        constexpr U64 entry = 0x1402AA2B0ULL;
        const auto* w = im_.walk(entry);
        add("41", "step-machine.walked", w != nullptr, hex(entry));
        if (w == nullptr) return;
        const std::set<U64> expected{0x1402AA300ULL, 0x1402AA346ULL, 0x1402AA408ULL, 0x1402AA4C4ULL,
                                     0x1402AA5D0ULL, 0x1402AA8F8ULL, 0x1402AA953ULL, 0x1402AA996ULL,
                                     0x1402AA99DULL, 0x1402AAA8DULL, 0x1402AAB51ULL, 0x1402AAC65ULL};
        std::set<U64> targets;
        for (const auto rva : w->switch_targets) targets.insert(kBase + rva);
        add("43", "step-machine.table-targets", targets == expected,
            std::to_string(targets.size()) + " unique switch targets (12 expected, table 0x1402AAC6C)");
        add("43", "step-machine.no-unresolved-jumps", w->unresolved_indirect_jumps == 0U,
            std::to_string(w->unresolved_indirect_jumps) + " unresolved indirect jumps");
        add("43", "step-machine.instructions", w->instruction_count == 599U,
            std::to_string(w->instruction_count) + " reachable instructions (Gate 43: 599)");
        std::set<U32> starts(w->instruction_starts.begin(), w->instruction_starts.end());
        int reached = 0;
        for (const U64 site : {0x1402AA300ULL, 0x1402AA319ULL, 0x1402AA522ULL, 0x1402AAAEFULL, 0x1402AAC0EULL,
                               0x1402AAC25ULL}) {
            if (starts.count(static_cast<U32>(site - kBase)) != 0U) ++reached;
        }
        add("43", "step-machine.six-sites-reached", reached == 6, std::to_string(reached) + " of 6 sites");
        calls("44", "step-machine.hides-objects", entry, 0x1402F7350ULL);
    }

    // Gates 44-45: the folded accessor.
    void accessor() {
        bytes_at("44", "accessor.lea-rax-rcx-0x80-ret", 0x140089DE0ULL, {0x48, 0x8D, 0x81, 0x80, 0x00, 0x00, 0x00, 0xC3});
    }

    // Gate 46: the selector's five call sites.
    void light_lists() {
        const std::vector<std::pair<U64, U64>> sites{{0x1402FC7CDULL, 0x1402FC720ULL},
                                                     {0x1402FD140ULL, 0x1402FD040ULL},
                                                     {0x1402FD485ULL, 0x1402FD370ULL},
                                                     {0x1402FD4ADULL, 0x1402FD370ULL},
                                                     {0x14031FB02ULL, 0x14031FA80ULL}};
        for (const auto& [site, function] : sites) {
            call_at("46", "selector-call." + hex(site), site, 0x1402EE560ULL);
            const auto f = im_.function_of(site);
            add("46", "selector-call-owner." + hex(site), f && *f == function,
                hex(site) + " in " + (f ? hex(*f) : std::string{"?"}));
        }
    }

    // Gate 46 v0.6-v0.7: LIG2 -> CLightStatic -> CLightMgr main chain.
    void path() {
        // Interleaved with the end of the 0x920-byte copy (movups).
        std::optional<U64> mgr, statics, link;
        im_.each_in_range(0x14023CA77ULL, 0x14023CA94ULL, [&](U64 va, const exe::X86Instruction& in) {
            if (is_lea(in, rdx, rdi, 0x40)) mgr = va;
            if (is_lea(in, rcx, rdi, 0x1D80)) statics = va;
            if (is_call_to(in, va, 0x140089000ULL)) link = va;
        });
        add("46", "link.lea-rdx-mgr", mgr.has_value(), "lea rdx, [rdi+0x40]" + (mgr ? " at " + hex(*mgr) : ""));
        add("46", "link.lea-rcx-lightstatic", statics.has_value(),
            "lea rcx, [rdi+0x1D80]" + (statics ? " at " + hex(*statics) : ""));
        add("46", "link.call-0x140089000", link.has_value(), "call 0x140089000" + (link ? " at " + hex(*link) : ""));
        bool records = false, count = false, entries = false;
        im_.each_instruction(0x140089000ULL, [&](U64, const exe::X86Instruction& in) {
            records = records || is_lea(in, rbx, rdi, 0x940);
            count = count || (!in.two_byte_opcode && in.opcode == 0xBEU && in.immediate == 0x30);
            entries = entries || (!in.two_byte_opcode && in.opcode == 0x81U && in.modrm_reg == 0U &&
                                  in.rm_operand == rdi && in.immediate == 0x928);
        });
        add("46", "link.loop-records-0x940", records, "lea rbx, [rdi+0x940]");
        add("46", "link.loop-48-entries", count, "mov esi, 0x30");
        add("46", "link.loop-entries-0x928", entries, "add rdi, 0x928");
        calls("46", "link.chain-clear", 0x140089000ULL, 0x1402C83A0ULL);
        calls("46", "link.chain-append", 0x140089000ULL, 0x1402C8310ULL);
        add("46", "csgm.vtable-slot3", im_.uint(0x1404E32E0ULL, 8) == 0x14023B3E0ULL,
            "[0x1404E32E0] = " + hex(im_.uint(0x1404E32E0ULL, 8)));
        const auto* w = im_.walk(0x14023B3E0ULL);
        const bool case0 = w != nullptr && std::binary_search(w->switch_targets.begin(), w->switch_targets.end(),
                                                              static_cast<U32>(0x14023B424ULL - kBase));
        add("46", "csgm.step-switch-case0", case0,
            "0x14023B3E0 switch reaches 0x14023B424 (" + std::to_string(w ? w->switch_targets.size() : 0U) +
                " targets)");
        call_at("46", "csgm.calls-cfg-load", 0x14023B427ULL, 0x14023C3D0ULL);
    }

    // Gate 47: the model constants and ambient.
    void model_constants() {
        calls("47", "cdrawscm.ambient", 0x1402FD040ULL, 0x1402FA580ULL);
        calls("47", "static-lights.selects", 0x1402FC720ULL, 0x1402EE560ULL);
    }

    // Gate 48: the stage path, K and the masks.
    void stage_path() {
        const auto tail = im_.decode(0x14008991AULL);
        add("48", "holder.tail-call-static", tail && is_jump_to(*tail, 0x14008991AULL, 0x1402FC670ULL),
            "0x14008991A jmp 0x1402FC670");
        calls("48", "holder.frame-update", 0x14008B5D0ULL, 0x1402FD2B0ULL);
        calls("48", "model-holder.cdrawscm", 0x14008B500ULL, 0x1402FD040ULL);
        calls("48", "objects.static", 0x1402FC670ULL, 0x1402FC720ULL);
        calls("48", "objects.frame", 0x1402FD2B0ULL, 0x1402FD370ULL);
        calls("48", "stage.transpose", 0x1402FD370ULL, 0x140031240ULL);
        calls("48", "stage.transform", 0x1402FD370ULL, 0x140030E40ULL);
        calls("48", "stage.pack", 0x1402FD370ULL, 0x1402FCED0ULL);
        calls("48", "stage.ambient", 0x1402FD370ULL, 0x1402FA6D0ULL);
        calls("48", "stage.scale", 0x1402FCED0ULL, 0x1400311E0ULL);
        add("48", "stage.K-1/255", im_.uint(0x14035D558ULL, 4) == 0x3B808081ULL,
            "[0x14035D558] = " + hex(im_.uint(0x14035D558ULL, 4)));
        bytes_at("48", "stage.w-mask-source", 0x14035D340ULL,
                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF});
        store_in("48", "stage-holder.mask-1-lights-on", 0x140089320ULL, 0xC7U, 0x570, 0x10000);
        store_in("48", "model-holder.mask-2", 0x140089270ULL, 0xC7U, 0x744, 2);
        store_in("48", "stage-holder.lights-off", 0x140089F20ULL, 0xC6U, 0x570, 1);
    }

    // The spawn-site catalog (profiles/dmc3/effect_spawn_sites.hpp): every row
    // is a direct call (or tail jump) to its spawn API or kind factory, made
    // from the function the row names; and the em000 event handler.
    void effect_catalogs() {
        namespace sites = profiles::dmc3::effect_spawn_sites;
        std::size_t calls_ok = 0U, owners_ok = 0U, total = 0U;
        std::set<U64> addresses;
        for (const auto& site : sites::spawn_sites()) {
            ++total;
            const auto in = im_.decode(site.address);
            const U64 target = sites::call_target(site);
            if (in && (is_call_to(*in, site.address, target) || is_jump_to(*in, site.address, target))) ++calls_ok;
            // A leaf function has no unwind entry: its row names its entry.
            const auto f = im_.function_of(site.address);
            if ((f && *f == site.function) || (!f && site.function <= site.address)) {
                ++owners_ok;
            } else if (std::getenv("DMC3_GATES_DEBUG") != nullptr) {
                std::cerr << "row " << hex(site.address) << " names " << hex(site.function) << ", .pdata "
                          << (f ? hex(*f) : std::string{"none"}) << '\n';
            }
            addresses.insert(site.address);
        }
        add("fx", "spawn-sites.calls", calls_ok == total,
            std::to_string(calls_ok) + " of " + std::to_string(total) + " rows call their API or factory (" +
                std::to_string(addresses.size()) + " call sites)");
        add("fx", "spawn-sites.functions", owners_ok == total,
            std::to_string(owners_ok) + " of " + std::to_string(total) + " rows lie in the function they name");
        function_start("em000", "event-handler", profiles::dmc3::em000_events::handler_va);
        const auto death = im_.function_of(profiles::dmc3::em000_events::death_state_va);
        add("em000", "death-state-in-code", death.has_value(),
            "0x140095E85 in " + (death ? hex(*death) : std::string{"?"}));
    }

    // Gate 48 v0.3d: the vertex formats.
    void vertex_formats() {
        constexpr U64 builder = 0x1400473F0ULL;
        std::set<std::int64_t> immediates;
        im_.each_instruction(builder, [&](U64, const exe::X86Instruction& in) {
            if (in.immediate_size != 0U) immediates.insert(in.immediate & 0xFFFFFFFFLL);
        });
        for (const std::int64_t fvf : {0x80030112LL, 0x80030152LL, 0x80000152LL, 0x40000002LL, 0x102LL, 0x142LL, 0x42LL}) {
            add("48", "vertex.fvf-" + hex(static_cast<U64>(fvf)), immediates.count(fvf) != 0U,
                hex(builder) + " writes FVF " + hex(static_cast<U64>(fvf)));
        }
        const auto* w = im_.walk(builder);
        for (const U64 layout : {0x140562620ULL, 0x140562920ULL, 0x1405626C0ULL, 0x1405629C0ULL, 0x140562780ULL,
                                 0x140562A80ULL, 0x140562800ULL, 0x140562820ULL, 0x140562880ULL, 0x140562B40ULL,
                                 0x1405628C0ULL, 0x140562B80ULL, 0x140562840ULL, 0x140562B00ULL}) {
            const auto rva = static_cast<U32>(layout - kBase);
            const bool referenced =
                w != nullptr && (std::binary_search(w->data_references.begin(), w->data_references.end(), rva) ||
                                 std::binary_search(w->taken_addresses.begin(), w->taken_addresses.end(), rva));
            add("48", "vertex.layout-" + hex(layout), referenced, hex(builder) + " references " + hex(layout));
        }
    }
};

// ---- Output -----------------------------------------------------------------------

std::string json_escape(const std::string& text) {
    std::string out;
    for (const char c : text) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

void write_json(const std::string& path, const std::vector<Check>& checks) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write " + path);
    std::size_t passed = 0;
    for (const auto& c : checks) passed += c.pass ? 1U : 0U;
    out << "{\n  \"schema\": \"dmc-rengine.dmc3-gates.v1\",\n"
        << "  \"target\": {\"name\": \"dmc3.exe\", \"sha256\": \"" << kCanonical
        << "\", \"image_base\": \"0x140000000\"},\n"
        << "  \"tool\": \"tools/reverse/dmc3_gates.cpp\",\n"
        << "  \"evidence\": [\"docs/research/dmc3-gates-00-48-2026-10-29.md\", "
           "\"docs/research/dmc3-light-pipeline-gates-2026-10-29.md\"],\n"
        << "  \"checks_passed\": " << passed << ",\n  \"checks_total\": " << checks.size() << ",\n  \"checks\": [\n";
    for (std::size_t i = 0; i < checks.size(); ++i) {
        const auto& c = checks[i];
        out << "    {\"gate\": \"" << c.gate << "\", \"id\": \"" << json_escape(c.id) << "\", \"status\": \""
            << (c.pass ? "PASS" : "FAIL") << "\", \"detail\": \"" << json_escape(c.detail) << "\"}"
            << (i + 1 < checks.size() ? "," : "") << '\n';
    }
    out << "  ]\n}\n";
}

// ---- Self test: the predicates on hand-assembled code -------------------------------

void self_test() {
    const auto require = [](bool ok, const char* what) {
        if (!ok) throw std::runtime_error(std::string{"self-test failed: "} + what);
    };
    // lea rdx,[rdi+0x40]; lea rcx,[rdi+0x1D80]; call +0; jmp +0x10;
    // mov dword [rbx+0x570],0x10000; mov byte [rbx+0x570],1; mov esi,0x30; add rdi,0x928
    const std::vector<std::uint8_t> code{
        0x48, 0x8D, 0x57, 0x40,                                      // lea rdx,[rdi+0x40]
        0x48, 0x8D, 0x8F, 0x80, 0x1D, 0x00, 0x00,                    // lea rcx,[rdi+0x1D80]
        0xE8, 0x00, 0x00, 0x00, 0x00,                                // call next
        0xE9, 0x10, 0x00, 0x00, 0x00,                                // jmp +0x10
        0xC7, 0x83, 0x70, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,  // mov dword [rbx+0x570],0x10000
        0xC6, 0x83, 0x70, 0x05, 0x00, 0x00, 0x01,                    // mov byte [rbx+0x570],1
        0xBE, 0x30, 0x00, 0x00, 0x00,                                // mov esi,0x30
        0x48, 0x81, 0xC7, 0x28, 0x09, 0x00, 0x00,                    // add rdi,0x928
    };
    std::vector<std::byte> bytes(code.size());
    std::transform(code.begin(), code.end(), bytes.begin(), [](std::uint8_t b) { return std::byte{b}; });
    std::size_t at = 0;
    const auto next = [&]() {
        const auto in = exe::X86LengthDecoder::decode(bytes, at);
        if (!in) throw std::runtime_error("self-test failed: decode");
        const auto here = at;
        at += in->length;
        return std::pair{static_cast<U64>(here), *in};
    };
    auto [a0, i0] = next();
    require(is_lea(i0, rdx, rdi, 0x40) && !is_lea(i0, rcx, rdi, 0x40), "lea disp8");
    auto [a1, i1] = next();
    require(is_lea(i1, rcx, rdi, 0x1D80), "lea disp32");
    auto [a2, i2] = next();
    require(is_call_to(i2, kBase + a2, kBase + a2 + 5U) && !is_call_to(i2, kBase + a2, kBase), "call target");
    auto [a3, i3] = next();
    require(is_jump_to(i3, kBase + a3, kBase + a3 + 5U + 0x10U), "jump target");
    auto [a4, i4] = next();
    require(is_store_imm(i4, 0xC7U, 0x570, 0x10000) && !is_store_imm(i4, 0xC7U, 0x570, 1), "dword store");
    auto [a5, i5] = next();
    require(is_store_imm(i5, 0xC6U, 0x570, 1), "byte store");
    auto [a6, i6] = next();
    require(!i6.two_byte_opcode && i6.opcode == 0xBEU && i6.immediate == 0x30, "mov esi");
    auto [a7, i7] = next();
    require(i7.opcode == 0x81U && i7.modrm_reg == 0U && i7.rm_operand == rdi && i7.immediate == 0x928, "add rdi");
    (void)a0; (void)a1; (void)a4; (void)a5; (void)a6; (void)a7;
    std::cout << "self-test PASS: lea disp8/disp32, call and jump targets, dword/byte stores, mov esi, add rdi\n";
}

}  // namespace

int main(int argc, char** argv) try {
    if (argc == 2 && std::string{argv[1]} == "--self-test") {
        self_test();
        return 0;
    }
    if (argc != 2 && !(argc == 4 && std::string{argv[2]} == "--json")) {
        std::cerr << "usage: dmc3-gates canonical-dmc3.exe [--json report.json] | --self-test\n";
        return 2;
    }
    Image im;
    std::ifstream in(argv[1], std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("cannot open the image");
    im.bytes.resize(static_cast<std::size_t>(in.tellg()));
    in.seekg(0);
    if (!in.read(reinterpret_cast<char*>(im.bytes.data()), static_cast<std::streamsize>(im.bytes.size()))) {
        throw std::runtime_error("image read failed");
    }
    if (core::Sha256::compute(im.bytes).hex() != kCanonical) throw std::runtime_error("canonical SHA-256 mismatch");
    const auto parsed = exe::PeReader::read(im.bytes);
    if (!parsed.ok()) throw std::runtime_error("invalid PE");
    im.pe = *parsed.image;
    const auto directories = exe::PeDirectoryReader::read(im.bytes, im.pe);
    if (!directories.directories.functions) throw std::runtime_error("no exception directory");
    im.functions = *directories.directories.functions;
    im.graph = exe::CodeGraphBuilder::build(im.bytes, im.pe, im.functions);
    for (const auto& walk : im.graph.functions) {
        im.walks[walk.begin_rva] = &walk;
        im.referenced_entries.insert(walk.call_targets.begin(), walk.call_targets.end());
        im.referenced_entries.insert(walk.taken_addresses.begin(), walk.taken_addresses.end());
        im.referenced_entries.insert(walk.external_jump_targets.begin(), walk.external_jump_targets.end());
    }
    im.rtti = exe::RttiScanner::scan(im.bytes, im.pe);

    const auto checks = Gates{im}.run();
    std::size_t passed = 0;
    for (const auto& c : checks) {
        passed += c.pass ? 1U : 0U;
        std::cout << (c.pass ? "PASS" : "FAIL") << "  gate " << c.gate << "  " << c.id << "  " << c.detail << '\n';
    }
    std::cout << passed << " of " << checks.size() << " checks pass\n";
    if (argc == 4) write_json(argv[3], checks);
    return passed == checks.size() ? 0 : 1;
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
