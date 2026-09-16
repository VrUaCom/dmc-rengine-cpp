#include "dmc_rengine/exe/function_map.hpp"

#include "dmc_rengine/exe/x86_decoder.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <cctype>
#include <utility>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <unordered_map>
#include <utility>
#include <unordered_set>
#include <optional>

namespace dmc::rengine::exe {
namespace {

using detail::read_cstring;
using detail::read_u64;

constexpr std::size_t kMinStringLength = 4U;
constexpr std::size_t kMaxStringLength = 200U;
constexpr std::size_t kMaxStringsPerFunction = 8U;
constexpr std::size_t kMaxVtableSlotsRead = 4096U;
/// Shorter than this and a run of addresses in data is as likely to be two
/// unrelated pointers side by side as a table.
constexpr std::size_t kMinFunctionPointerRun = 3U;

[[nodiscard]] bool is_read_only_data(const PeSection& section) noexcept {
    constexpr std::uint32_t initialized_data = 0x00000040U;
    constexpr std::uint32_t executable = 0x20000000U;
    constexpr std::uint32_t writable = 0x80000000U;
    return section.raw_size > 0U && (section.characteristics & initialized_data) != 0U &&
           (section.characteristics & executable) == 0U &&
           (section.characteristics & writable) == 0U;
}

[[nodiscard]] std::size_t readable_size(std::span<const std::byte> bytes,
                                        const PeSection& section) noexcept {
    const auto begin = static_cast<std::size_t>(section.raw_offset);
    if (begin >= bytes.size()) {
        return 0U;
    }
    return std::min(bytes.size() - begin, static_cast<std::size_t>(section.raw_size));
}

/// Recovers NUL-terminated printable ASCII literals from read-only data.
///
/// Wide literals are not collected: they would need their own evidence about
/// encoding, and the narrow set is already enough to attribute functions.
[[nodiscard]] std::vector<std::pair<std::uint32_t, std::string>> collect_strings(
    std::span<const std::byte> bytes, const PeImage& image) {
    std::vector<std::pair<std::uint32_t, std::string>> strings;

    for (const auto& section : image.sections) {
        if (!is_read_only_data(section)) {
            continue;
        }

        const auto begin = static_cast<std::size_t>(section.raw_offset);
        const auto size = readable_size(bytes, section);
        std::size_t index = 0U;
        while (index < size) {
            const auto character = std::to_integer<unsigned char>(bytes[begin + index]);
            if (character < 0x20U || character > 0x7EU) {
                ++index;
                continue;
            }

            std::size_t run = 0U;
            while (index + run < size) {
                const auto scanned = std::to_integer<unsigned char>(bytes[begin + index + run]);
                if (scanned < 0x20U || scanned > 0x7EU) {
                    break;
                }
                ++run;
            }

            const bool terminated =
                index + run < size &&
                std::to_integer<unsigned char>(bytes[begin + index + run]) == 0U;
            if (terminated && run >= kMinStringLength) {
                auto text = read_cstring(bytes, begin + index, kMaxStringLength + 1U);
                if (text.has_value() && text->size() <= kMaxStringLength) {
                    strings.emplace_back(static_cast<std::uint32_t>(section.virtual_address + index),
                                         std::move(*text));
                }
            }
            index += run + 1U;
        }
    }

    std::sort(strings.begin(), strings.end());
    return strings;
}

/// Import-address-table slot -> imported symbol.
[[nodiscard]] std::unordered_map<std::uint32_t, ImportCall> collect_iat_slots(
    const PeDirectories& directories) {
    std::unordered_map<std::uint32_t, ImportCall> slots;
    for (const auto& module : directories.imports) {
        for (const auto& function : module.functions) {
            if (function.iat_rva == 0U) {
                continue;
            }
            ImportCall call;
            call.module = module.name;
            call.function = function.by_ordinal
                                ? "#" + std::to_string(function.ordinal)
                                : function.name;
            slots.emplace(function.iat_rva, std::move(call));
        }
    }
    return slots;
}

/// Locates the function containing an RVA.
class FunctionIndex final {
public:
    explicit FunctionIndex(const std::vector<FunctionWalk>& walks) {
        order_.reserve(walks.size());
        for (std::size_t index = 0; index < walks.size(); ++index) {
            order_.push_back({walks[index].begin_rva, walks[index].end_rva, index});
        }
        std::sort(order_.begin(), order_.end(), [](const Entry& left, const Entry& right) {
            return left.begin < right.begin;
        });
    }

    [[nodiscard]] std::optional<std::size_t> containing(std::uint32_t rva) const {
        if (order_.empty()) {
            return std::nullopt;
        }

        auto entry = std::upper_bound(order_.begin(), order_.end(), rva,
                                      [](std::uint32_t value, const Entry& candidate) {
                                          return value < candidate.begin;
                                      });
        if (entry == order_.begin()) {
            return std::nullopt;
        }
        --entry;
        if (rva >= entry->begin && rva < entry->end) {
            return entry->index;
        }
        return std::nullopt;
    }

private:
    struct Entry final {
        std::uint32_t begin{};
        std::uint32_t end{};
        std::size_t index{};
    };

    std::vector<Entry> order_;
};

[[nodiscard]] std::string lowered(std::string_view value) {
    std::string result{value};
    for (auto& character : result) {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character)));
    }
    return result;
}

} // namespace

std::vector<std::string> resource_family_hints(std::string_view literal) {
    // Extensions and namespace markers for the families documented under
    // docs/formats. Matching is on the literal's own text; it carries no claim
    // about what the referencing code does with it.
    static constexpr std::pair<std::string_view, std::string_view> kMarkers[]{
        {".pac", "PAC"},      {".pnst", "PNST"},  {".mod", "MOD"},
        {".scm", "SCM"},      {".shw", "SHW"},    {".ptx", "PTX"},
        {".dds", "DDS"},      {".nbz", "NBZ"},    {".lig", "LIG"},
        {".dca", "DCA"},      {".efm", "EFM"},    {".mot", "MOT"},
        {".hit", "HITS"},     {".hlsl", "SHADER"},{".afs", "ARCHIVE"},
        {".sac", "SAC"},      {".tex", "TEX"},
        // Families measured in this image's name tables rather than inherited
        // from the format docs. Counts are recorded in the evidence packet.
        {".adx", "ADX"},      {".ogg", "OGG"},    {".sfd", "SFD"},
        {".fxh", "SHADER"},   {".tm2", "TM2"},    {".txt", "TEXT"},
    };

    const auto haystack = lowered(literal);
    std::vector<std::string> families;
    for (const auto& [marker, family] : kMarkers) {
        if (haystack.find(marker) != std::string::npos) {
            families.emplace_back(family);
        }
    }

    std::sort(families.begin(), families.end());
    families.erase(std::unique(families.begin(), families.end()), families.end());
    return families;
}

FunctionMap FunctionMapBuilder::build(std::span<const std::byte> bytes,
                                      const FunctionMapInputs& inputs) {
    FunctionMap map;
    if (inputs.image == nullptr || inputs.graph == nullptr) {
        map.warnings.emplace_back("Function map requires an image and a code graph.");
        return map;
    }

    const auto& image = *inputs.image;
    const auto& graph = *inputs.graph;
    const auto strings = collect_strings(bytes, image);
    const auto iat_slots = inputs.directories != nullptr
                               ? collect_iat_slots(*inputs.directories)
                               : std::unordered_map<std::uint32_t, ImportCall>{};
    map.summary.strings_recovered = strings.size();

    // Prologue facts live on the primary unwind range, keyed by function entry.
    std::unordered_map<std::uint32_t, PeUnwindFrame> frames;
    if (inputs.directories != nullptr && inputs.directories->functions.has_value()) {
        for (const auto& range : inputs.directories->functions->functions) {
            if (!range.chained) {
                frames.emplace(range.begin_rva, range.frame);
            }
        }
    }

    // Recovered name tables, ordered by base so a data reference can be placed
    // inside a span by binary search. A reference need not land on the base:
    // the compiler folds a constant index into the displacement.
    struct TableSpan final {
        std::uint32_t base{};
        std::uint32_t end{};
        bool record_layout{};
        std::uint32_t element_bytes{};
        std::uint32_t entries{};
        /// Field offsets within a record, for record layouts only.
        std::vector<std::uint32_t> field_offsets;
    };
    std::vector<TableSpan> table_spans;
    if (inputs.name_tables != nullptr) {
        for (const auto& run : inputs.name_tables->runs) {
            const auto span = run.span_bytes();
            if (span == 0U) {
                continue;
            }
            // A run that is a record's interior describes bytes the record
            // describes in full. Matching against the record instead resolves a
            // reference to a record and a field rather than to a grid position,
            // and keeps the same bytes from being counted as two tables.
            if (run.is_record_interior()) {
                continue;
            }
            table_spans.push_back(TableSpan{run.base_rva,
                                            static_cast<std::uint32_t>(run.base_rva + span), false,
                                            run.stride, run.entries, {}});
        }
        for (const auto& run : inputs.name_tables->records) {
            const auto span = run.span_bytes();
            if (span == 0U) {
                continue;
            }
            table_spans.push_back(TableSpan{run.base_rva,
                                            static_cast<std::uint32_t>(run.base_rva + span), true,
                                            run.record_bytes, run.records, run.field_offsets});
        }
        std::sort(table_spans.begin(), table_spans.end(),
                  [](const TableSpan& left, const TableSpan& right) {
                      return left.base < right.base;
                  });
    }

    // Spans can overlap, so the nearest preceding base is not always the one
    // that contains an address: a run sitting inside a record array starts
    // later and can end sooner. Walking back by the widest span seen covers
    // every candidate; the widest is what bounds how far back one can begin.
    std::uint64_t widest_span = 0U;
    for (const auto& span : table_spans) {
        widest_span = std::max<std::uint64_t>(widest_span, span.end - span.base);
    }

    const auto table_containing = [&](std::uint32_t rva) -> const TableSpan* {
        if (table_spans.empty()) {
            return nullptr;
        }
        auto entry = std::upper_bound(table_spans.begin(), table_spans.end(), rva,
                                      [](std::uint32_t value, const TableSpan& candidate) {
                                          return value < candidate.base;
                                      });

        const TableSpan* chosen = nullptr;
        while (entry != table_spans.begin()) {
            --entry;
            if (rva - entry->base > widest_span) {
                break;
            }
            if (rva < entry->base || rva >= entry->end) {
                continue;
            }
            // A record reading names the field the reference picks, so it is
            // preferred; between two of the same kind the tighter span is the
            // more specific statement about the address.
            if (chosen == nullptr || (entry->record_layout && !chosen->record_layout) ||
                (entry->record_layout == chosen->record_layout &&
                 entry->end - entry->base < chosen->end - chosen->base)) {
                chosen = &*entry;
            }
        }
        return chosen;
    };

    const FunctionIndex index{graph.functions};
    map.functions.resize(graph.functions.size());
    for (std::size_t position = 0; position < graph.functions.size(); ++position) {
        const auto& walk = graph.functions[position];
        auto& facts = map.functions[position];
        facts.begin_rva = walk.begin_rva;
        facts.end_rva = walk.end_rva;
        facts.instruction_count = walk.instruction_count;
        facts.walk_complete = walk.complete;
        if (const auto frame = frames.find(walk.begin_rva); frame != frames.end()) {
            facts.frame = frame->second;
        }
    }

    // ----- exports ---------------------------------------------------------
    if (inputs.directories != nullptr && inputs.directories->exports.has_value()) {
        for (const auto& symbol : inputs.directories->exports->symbols) {
            if (symbol.forwarded()) {
                continue;
            }
            if (const auto owner = index.containing(symbol.rva); owner.has_value()) {
                map.functions[*owner].exported = true;
                map.functions[*owner].export_name = symbol.name;
            }
        }
    }

    // Vtable address -> owning class. A function referencing one of these is
    // installing that vtable, which is what construction code does.
    struct VtableOwner final {
        std::string class_display_name;
        std::uint32_t vtable_index{};
    };
    std::unordered_map<std::uint32_t, VtableOwner> vtable_owners;
    if (inputs.rtti != nullptr) {
        for (const auto& entry : inputs.rtti->classes) {
            for (std::uint32_t index = 0U; index < entry.vtables.size(); ++index) {
                if (entry.vtables[index].vtable_rva != 0U) {
                    vtable_owners.emplace(entry.vtables[index].vtable_rva,
                                          VtableOwner{entry.display_name, index});
                }
            }
        }
    }

    // ----- virtual method bindings -----------------------------------------
    // Keyed by the vtable's own address, because that is what the code reads:
    // a call through a base subobject goes into that subobject's vtable, not
    // into the class's primary one.
    std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> slot_targets;
    std::map<std::pair<std::string, std::uint32_t>, std::uint32_t> class_vtable_rva;
    std::map<std::pair<std::string, std::uint32_t>, std::uint32_t> class_vtable_at_offset;
    if (inputs.rtti != nullptr) {
        std::map<std::string, ClassCodeCoverage> coverage;
        for (const auto& entry : inputs.rtti->classes) {
            auto& record = coverage[entry.display_name];
            record.class_display_name = entry.display_name;

            std::vector<std::size_t> bound;
            for (std::uint32_t vtable_index = 0U; vtable_index < entry.vtables.size();
                 ++vtable_index) {
                const auto& vtable = entry.vtables[vtable_index];
                const auto slots = std::min<std::size_t>(vtable.slot_count, kMaxVtableSlotsRead);
                record.vtable_slots += static_cast<std::uint32_t>(slots);

                const auto table_offset = image.rva_to_file_offset(vtable.vtable_rva);
                if (!table_offset.has_value()) {
                    continue;
                }

                for (std::size_t slot = 0; slot < slots; ++slot) {
                    const auto pointer =
                        read_u64(bytes, static_cast<std::size_t>(*table_offset) + slot * 8U);
                    if (!pointer.has_value() || *pointer <= image.image_base) {
                        continue;
                    }
                    const auto delta = *pointer - image.image_base;
                    if (delta > std::numeric_limits<std::uint32_t>::max()) {
                        continue;
                    }

                    const auto owner = index.containing(static_cast<std::uint32_t>(delta));
                    if (!owner.has_value()) {
                        continue;
                    }

                    ++record.slots_bound_to_functions;
                    bound.push_back(*owner);
                    map.functions[*owner].virtual_bindings.push_back(VirtualMethodBinding{
                        entry.display_name, vtable_index, static_cast<std::uint32_t>(slot),
                        vtable.subobject_offset});
                    slot_targets[{vtable.vtable_rva, static_cast<std::uint32_t>(slot)}] =
                        static_cast<std::uint32_t>(delta);
                    class_vtable_rva[{entry.display_name, vtable_index}] = vtable.vtable_rva;
                    // Where a base subobject sits inside the complete object is
                    // recorded by the RTTI itself, so a call through that offset
                    // needs no constructor store to find its vtable — and it
                    // finds the most derived class's override, which is what
                    // actually runs.
                    class_vtable_at_offset[{entry.display_name, vtable.subobject_offset}] =
                        vtable.vtable_rva;
                }
            }

            std::sort(bound.begin(), bound.end());
            bound.erase(std::unique(bound.begin(), bound.end()), bound.end());
            record.distinct_functions = static_cast<std::uint32_t>(bound.size());
        }

        map.class_coverage.reserve(coverage.size());
        for (auto& [name, record] : coverage) {
            static_cast<void>(name);
            map.class_coverage.push_back(std::move(record));
        }
        std::sort(map.class_coverage.begin(), map.class_coverage.end(),
                  [](const ClassCodeCoverage& left, const ClassCodeCoverage& right) {
                      if (left.vtable_slots != right.vtable_slots) {
                          return left.vtable_slots > right.vtable_slots;
                      }
                      return left.class_display_name < right.class_display_name;
                  });
    }

    // Candidate targets per slot index, over every vtable the type information
    // locates, and each vtable's extent. Filled by the census below and used by
    // the reachability bound further down.
    std::map<std::uint32_t, std::vector<std::uint32_t>> slot_candidates;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> vtable_extents;
    std::map<std::uint32_t, std::uint32_t> vtable_slot_count;

    // ----- vtable slot census ----------------------------------------------
    // What a slot's target *is* is readable from its first instruction alone,
    // and the three cases it separates are the ones that matter for reading an
    // interface: a declaration with no definition, a body that does nothing,
    // and code. The first is named outright by the import directory rather than
    // recognised by shape, which is why it is a fact and not a guess.
    if (inputs.rtti != nullptr) {
        struct VtableAtOffset final {
            std::uint32_t rva{};
            std::uint32_t slots{};
        };

        std::unordered_map<std::uint32_t, std::string> purecall_slots;
        if (inputs.directories != nullptr) {
            for (const auto& module : inputs.directories->imports) {
                for (const auto& function : module.functions) {
                    if (function.name == "_purecall") {
                        purecall_slots.emplace(function.iat_rva, module.name);
                    }
                }
            }
        }

        std::unordered_map<std::uint32_t, VtableSlotKind> slot_kind;
        const auto classify = [&](std::uint32_t target) -> VtableSlotKind {
            if (const auto known = slot_kind.find(target); known != slot_kind.end()) {
                return known->second;
            }
            auto kind = VtableSlotKind::implemented;
            if (const auto offset = image.rva_to_file_offset(target); offset.has_value()) {
                const auto decoded =
                    X86LengthDecoder::decode(bytes, static_cast<std::size_t>(*offset));
                if (decoded.has_value()) {
                    if (decoded->flow == X86Flow::return_) {
                        kind = VtableSlotKind::empty_body;
                    } else if (decoded->flow == X86Flow::jump_indirect && decoded->rip_relative) {
                        const auto reached = target + decoded->length +
                                             static_cast<std::uint32_t>(decoded->displacement);
                        if (purecall_slots.find(reached) != purecall_slots.end()) {
                            kind = VtableSlotKind::pure_virtual;
                        }
                    }
                }
            }
            slot_kind.emplace(target, kind);
            return kind;
        };

        const auto target_at = [&](std::uint32_t vtable_rva,
                                   std::uint32_t slot) -> std::optional<std::uint32_t> {
            const auto offset = image.rva_to_file_offset(vtable_rva);
            if (!offset.has_value()) {
                return std::nullopt;
            }
            const auto pointer =
                read_u64(bytes, static_cast<std::size_t>(*offset) + slot * 8U);
            if (!pointer.has_value() || *pointer <= image.image_base) {
                return std::nullopt;
            }
            const auto delta = *pointer - image.image_base;
            if (delta > std::numeric_limits<std::uint32_t>::max()) {
                return std::nullopt;
            }
            return static_cast<std::uint32_t>(delta);
        };

        // Every vtable of every class, keyed by the class and the subobject
        // offset its locator records. A class under multiple inheritance has
        // one per base subobject and they are different tables.
        std::map<std::pair<std::string, std::uint32_t>, VtableAtOffset> vtable_at;
        std::unordered_set<std::uint32_t> counted_vtables;
        for (const auto& entry : inputs.rtti->classes) {
            for (const auto& vtable : entry.vtables) {
                if (vtable.vtable_rva == 0U) {
                    continue;
                }
                const auto slots = std::min<std::uint32_t>(
                    vtable.slot_count, static_cast<std::uint32_t>(kMaxVtableSlotsRead));
                vtable_at.emplace(std::pair{entry.display_name, vtable.subobject_offset},
                                  VtableAtOffset{vtable.vtable_rva, slots});

                // One vtable can be reached through more than one class entry;
                // the census counts each table once.
                if (!counted_vtables.insert(vtable.vtable_rva).second) {
                    continue;
                }
                vtable_extents.emplace_back(vtable.vtable_rva,
                                            vtable.vtable_rva + slots * 8U);
                vtable_slot_count.emplace(vtable.vtable_rva, slots);
                for (std::uint32_t slot = 0U; slot < slots; ++slot) {
                    const auto target = target_at(vtable.vtable_rva, slot);
                    if (!target.has_value()) {
                        continue;
                    }
                    ++map.summary.vtable_slots_classified;
                    // A slot declared but not defined reaches the CRT's abort
                    // path, not the program, so it is not a candidate target.
                    if (classify(*target) != VtableSlotKind::pure_virtual) {
                        slot_candidates[slot].push_back(*target);
                    }
                    switch (classify(*target)) {
                    case VtableSlotKind::pure_virtual:
                        ++map.summary.vtable_slots_pure_virtual;
                        break;
                    case VtableSlotKind::empty_body:
                        ++map.summary.vtable_slots_empty_body;
                        break;
                    case VtableSlotKind::implemented:
                        ++map.summary.vtable_slots_implemented;
                        break;
                    }
                }
            }
        }

        std::unordered_set<std::uint32_t> distinct_implementations;
        for (const auto& [target, kind] : slot_kind) {
            if (kind == VtableSlotKind::implemented) {
                distinct_implementations.insert(target);
            }
        }
        map.summary.vtable_slot_implementations = distinct_implementations.size();

        // Pair each class with each of its bases at the offset the hierarchy
        // descriptor records, so the comparison runs table against table.
        std::map<std::string, std::vector<std::pair<std::string, std::uint32_t>>> inheritors;
        for (const auto& entry : inputs.rtti->classes) {
            for (std::size_t position = 1; position < entry.hierarchy.size(); ++position) {
                const auto& base = entry.hierarchy[position];
                if (base.member_displacement < 0) {
                    ++map.summary.base_pairings_without_a_vtable;
                    continue;
                }
                const auto offset = static_cast<std::uint32_t>(base.member_displacement);
                if (vtable_at.find({entry.display_name, offset}) == vtable_at.end()) {
                    ++map.summary.base_pairings_without_a_vtable;
                    continue;
                }
                inheritors[base.display_name].emplace_back(entry.display_name, offset);
            }
        }

        for (const auto& [base_name, classes] : inheritors) {
            const auto base_table = vtable_at.find({base_name, 0U});
            if (base_table == vtable_at.end()) {
                continue;
            }
            for (std::uint32_t slot = 0U; slot < base_table->second.slots; ++slot) {
                const auto base_target = target_at(base_table->second.rva, slot);
                if (!base_target.has_value()) {
                    continue;
                }

                BaseSlotOverride record;
                record.base_display_name = base_name;
                record.slot = slot;
                record.base_kind = classify(*base_target);
                record.base_target_rva = *base_target;

                std::unordered_set<std::uint32_t> implementations;
                for (const auto& [derived_name, offset] : classes) {
                    const auto table = vtable_at.find({derived_name, offset});
                    if (table == vtable_at.end() || slot >= table->second.slots) {
                        continue;
                    }
                    const auto target = target_at(table->second.rva, slot);
                    if (!target.has_value()) {
                        continue;
                    }
                    ++record.derived_classes;
                    if (*target == *base_target) {
                        ++record.keep_base_target;
                    }
                    switch (classify(*target)) {
                    case VtableSlotKind::pure_virtual:
                        ++record.pure_virtual;
                        break;
                    case VtableSlotKind::empty_body:
                        ++record.empty_bodies;
                        break;
                    case VtableSlotKind::implemented:
                        implementations.insert(*target);
                        break;
                    }
                }
                if (record.derived_classes == 0U) {
                    continue;
                }
                record.distinct_implementations =
                    static_cast<std::uint32_t>(implementations.size());
                map.base_slot_overrides.push_back(std::move(record));
            }
        }
        map.summary.base_slots_measured = map.base_slot_overrides.size();
        map.summary.vtables_located = vtable_extents.size();
        for (auto& [slot, targets] : slot_candidates) {
            static_cast<void>(slot);
            std::sort(targets.begin(), targets.end());
            targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
        }
        std::sort(vtable_extents.begin(), vtable_extents.end());
    }

    // ----- import thunks ---------------------------------------------------
    // A thunk is the whole function: one indirect jump through an IAT slot.
    std::unordered_map<std::size_t, ImportCall> thunk_imports;
    for (std::size_t position = 0; position < graph.functions.size(); ++position) {
        const auto& walk = graph.functions[position];
        if (walk.instruction_count != 1U || walk.indirect_jumps != 1U ||
            walk.data_references.size() != 1U) {
            continue;
        }

        const auto slot = iat_slots.find(walk.data_references.front());
        if (slot == iat_slots.end()) {
            continue;
        }

        map.functions[position].import_thunk = true;
        map.functions[position].imports_called.push_back(slot->second);
        thunk_imports.emplace(position, slot->second);
    }

    // A call can land on an import thunk that has no unwind data and so never
    // entered the inventory. Decoding its single jump recovers the import the
    // caller actually reached.
    std::unordered_map<std::uint32_t, ImportCall> external_thunks;
    std::unordered_map<std::uint32_t, bool> external_examined;
    const auto resolve_external_thunk = [&](std::uint32_t target) -> const ImportCall* {
        if (const auto known = external_thunks.find(target); known != external_thunks.end()) {
            return &known->second;
        }
        if (external_examined.find(target) != external_examined.end()) {
            return nullptr;
        }
        external_examined.emplace(target, true);

        const auto offset = image.rva_to_file_offset(target);
        if (!offset.has_value()) {
            return nullptr;
        }
        const auto decoded =
            X86LengthDecoder::decode(bytes, static_cast<std::size_t>(*offset));
        if (!decoded.has_value() || decoded->flow != X86Flow::jump_indirect ||
            !decoded->rip_relative) {
            return nullptr;
        }

        const auto slot_rva = static_cast<std::uint32_t>(
            static_cast<std::int64_t>(target) + decoded->length + decoded->displacement);
        const auto slot = iat_slots.find(slot_rva);
        if (slot == iat_slots.end()) {
            return nullptr;
        }

        const auto inserted = external_thunks.emplace(target, slot->second);
        ++map.summary.external_thunks_resolved;
        return &inserted.first->second;
    };

    // ----- references, callers and callees ---------------------------------
    for (std::size_t position = 0; position < graph.functions.size(); ++position) {
        const auto& walk = graph.functions[position];
        auto& facts = map.functions[position];

        facts.indirect_call_displacements = walk.indirect_call_displacements;

        for (const auto target : walk.data_references) {
            if (const auto slot = iat_slots.find(target); slot != iat_slots.end()) {
                facts.imports_called.push_back(slot->second);
                continue;
            }

            // An address inside a table that points at a NUL byte is not a
            // table access. The linker folds an empty string literal into any
            // NUL it can find, and a table's padding is full of them, so `""`
            // in a format call arrives here wearing a table's coordinates.
            const auto target_offset = image.rva_to_file_offset(target);
            const bool points_at_nul =
                target_offset.has_value() &&
                static_cast<std::size_t>(*target_offset) < bytes.size() &&
                std::to_integer<unsigned char>(bytes[static_cast<std::size_t>(*target_offset)]) ==
                    0U;

            if (const auto* table = points_at_nul ? nullptr : table_containing(target);
                table != nullptr) {
                NameTableReference reference;
                reference.table_base_rva = table->base;
                reference.offset_in_table = target - table->base;
                reference.record_layout = table->record_layout;
                reference.element_bytes = table->element_bytes;
                reference.entries = table->entries;

                if (table->element_bytes != 0U) {
                    reference.element_index = reference.offset_in_table / table->element_bytes;
                    reference.offset_in_element = reference.offset_in_table % table->element_bytes;
                    reference.constant_index = reference.offset_in_element == 0U;

                    // Inside a record the offset picks a field rather than the
                    // record's start, so an unaligned offset is still readable.
                    if (table->record_layout && !table->field_offsets.empty()) {
                        for (std::size_t field = 0; field < table->field_offsets.size(); ++field) {
                            if (table->field_offsets[field] <= reference.offset_in_element) {
                                reference.field_index = static_cast<std::uint32_t>(field);
                            }
                        }
                        reference.constant_index =
                            reference.offset_in_element == table->field_offsets[reference.field_index];
                    }
                }

                facts.name_tables.push_back(std::move(reference));
                continue;
            }

            if (const auto owner = vtable_owners.find(target); owner != vtable_owners.end()) {
                facts.installs_vtables.push_back(VtableInstall{
                    owner->second.class_display_name, owner->second.vtable_index, target});
                continue;
            }

            const auto literal = std::lower_bound(
                strings.begin(), strings.end(), target,
                [](const std::pair<std::uint32_t, std::string>& candidate, std::uint32_t value) {
                    return candidate.first < value;
                });
            if (literal != strings.end() && literal->first == target) {
                ++facts.string_reference_count;
                if (facts.referenced_strings.size() < kMaxStringsPerFunction) {
                    facts.referenced_strings.push_back(literal->second);
                }
                for (auto& family : resource_family_hints(literal->second)) {
                    facts.resource_families.push_back(std::move(family));
                }
            }
        }

        std::sort(facts.name_tables.begin(), facts.name_tables.end(),
                  [](const NameTableReference& left, const NameTableReference& right) {
                      if (left.table_base_rva != right.table_base_rva) {
                          return left.table_base_rva < right.table_base_rva;
                      }
                      return left.offset_in_table < right.offset_in_table;
                  });
        facts.name_tables.erase(
            std::unique(facts.name_tables.begin(), facts.name_tables.end()),
            facts.name_tables.end());

        std::sort(facts.resource_families.begin(), facts.resource_families.end());
        facts.resource_families.erase(
            std::unique(facts.resource_families.begin(), facts.resource_families.end()),
            facts.resource_families.end());

        // Tail calls leave the range, so they are call edges too.
        std::vector<std::uint32_t> targets = walk.call_targets;
        targets.insert(targets.end(), walk.external_jump_targets.begin(),
                       walk.external_jump_targets.end());
        std::sort(targets.begin(), targets.end());
        targets.erase(std::unique(targets.begin(), targets.end()), targets.end());

        for (const auto target : targets) {
            const auto callee = index.containing(target);
            if (!callee.has_value()) {
                if (const auto* import = resolve_external_thunk(target); import != nullptr) {
                    facts.imports_called.push_back(*import);
                }
                continue;
            }
            if (*callee == position) {
                continue;
            }

            ++facts.callee_count;
            ++map.functions[*callee].caller_count;
            facts.calls.push_back(map.functions[*callee].begin_rva);

            // Calling a thunk is calling the import behind it.
            if (const auto thunk = thunk_imports.find(*callee); thunk != thunk_imports.end()) {
                facts.imports_called.push_back(thunk->second);
            }
        }
    }

    for (auto& facts : map.functions) {
        std::sort(facts.calls.begin(), facts.calls.end());
        facts.calls.erase(std::unique(facts.calls.begin(), facts.calls.end()), facts.calls.end());
        std::sort(facts.imports_called.begin(), facts.imports_called.end(),
                  [](const ImportCall& left, const ImportCall& right) {
                      if (left.module != right.module) {
                          return left.module < right.module;
                      }
                      return left.function < right.function;
                  });
        facts.imports_called.erase(
            std::unique(facts.imports_called.begin(), facts.imports_called.end()),
            facts.imports_called.end());
    }

    // ----- reachability ----------------------------------------------------
    const auto propagate = [&](std::uint32_t root_rva, bool FunctionFacts::*flag) {
        const auto root = index.containing(root_rva);
        if (!root.has_value()) {
            return;
        }

        std::deque<std::size_t> queue{*root};
        map.functions[*root].*flag = true;
        while (!queue.empty()) {
            const auto position = queue.front();
            queue.pop_front();

            const auto& walk = graph.functions[position];
            const auto visit = [&](std::uint32_t target) {
                const auto callee = index.containing(target);
                if (!callee.has_value() || map.functions[*callee].*flag) {
                    return;
                }
                map.functions[*callee].*flag = true;
                queue.push_back(*callee);
            };

            for (const auto target : walk.call_targets) {
                visit(target);
            }
            for (const auto target : walk.external_jump_targets) {
                visit(target);
            }
        }
    };

    propagate(image.entry_point_rva, &FunctionFacts::reachable_from_entry_point);
    if (inputs.directories != nullptr && inputs.directories->exports.has_value()) {
        for (const auto& symbol : inputs.directories->exports->symbols) {
            if (!symbol.forwarded()) {
                propagate(symbol.rva, &FunctionFacts::reachable_from_export);
            }
        }
    }

    // ----- the upper bound on reachability ---------------------------------
    // Direct calls reach a fraction of this image, because almost everything
    // the engine does it does through a vtable. Assuming a virtual call can
    // reach whatever sits at its slot in *any* located vtable is unsound as an
    // answer and sound as a bound: whatever this does not reach, nothing in the
    // file says the entry point can.
    {
        // Dispatch slots each function uses, decoded from its own instructions.
        // A call or tail jump through a memory operand off a register is the
        // dispatch; a RIP-relative one is an import, and a register operand is
        // a computed jump, neither of which reads a vtable.
        std::vector<std::vector<std::uint32_t>> slots_used(map.functions.size());
        for (std::size_t position = 0; position < map.functions.size(); ++position) {
            auto& slots = slots_used[position];
            for (const auto rva : graph.functions[position].instruction_starts) {
                const auto offset = image.rva_to_file_offset(rva);
                if (!offset.has_value()) {
                    continue;
                }
                const auto decoded =
                    X86LengthDecoder::decode(bytes, static_cast<std::size_t>(*offset));
                if (!decoded.has_value()) {
                    continue;
                }
                if (decoded->flow != X86Flow::call_indirect &&
                    decoded->flow != X86Flow::jump_indirect) {
                    continue;
                }
                if (decoded->rip_relative || decoded->register_indirect() ||
                    decoded->memory_base == X86Instruction::kNoRegister) {
                    continue;
                }
                if (decoded->displacement < 0 || decoded->displacement % 8 != 0) {
                    continue;
                }
                slots.push_back(static_cast<std::uint32_t>(decoded->displacement) / 8U);
            }
            std::sort(slots.begin(), slots.end());
            slots.erase(std::unique(slots.begin(), slots.end()), slots.end());
        }

        std::deque<std::size_t> queue;
        std::set<std::uint32_t> slots_reached;
        const auto push = [&](std::size_t position) {
            if (!map.functions[position].reachable_through_dispatch) {
                map.functions[position].reachable_through_dispatch = true;
                queue.push_back(position);
            }
        };
        if (const auto root = index.containing(image.entry_point_rva); root.has_value()) {
            push(*root);
        }

        // A slot reached later can pull in targets already passed over, so the
        // two halves alternate until neither adds anything.
        bool changed = true;
        while (changed) {
            changed = false;
            while (!queue.empty()) {
                const auto position = queue.front();
                queue.pop_front();
                const auto& walk = graph.functions[position];
                for (const auto target : walk.call_targets) {
                    if (const auto callee = index.containing(target); callee.has_value()) {
                        push(*callee);
                    }
                }
                for (const auto target : walk.external_jump_targets) {
                    if (const auto callee = index.containing(target); callee.has_value()) {
                        push(*callee);
                    }
                }
                for (const auto slot : slots_used[position]) {
                    if (slots_reached.insert(slot).second) {
                        changed = true;
                    }
                }
            }
            for (const auto slot : slots_reached) {
                const auto candidates = slot_candidates.find(slot);
                if (candidates == slot_candidates.end()) {
                    continue;
                }
                for (const auto target : candidates->second) {
                    const auto callee = index.containing(target);
                    if (callee.has_value() &&
                        !map.functions[*callee].reachable_through_dispatch) {
                        push(*callee);
                        changed = true;
                    }
                }
            }
        }
        map.summary.dispatch_slots_reached = slots_reached.size();

        // Tightening the bound the usual way: only classes some reachable
        // function installs can be receivers. Measured, not applied, because on
        // this image it barely moves — construction is itself behind dispatch,
        // so the analysis starves before it starts.
        std::set<std::uint32_t> instantiated;
        for (std::size_t position = 0; position < map.functions.size(); ++position) {
            if (!map.functions[position].reachable_from_entry_point) {
                continue;
            }
            for (const auto& install : map.functions[position].installs_vtables) {
                if (vtable_slot_count.find(install.vtable_rva) != vtable_slot_count.end()) {
                    instantiated.insert(install.vtable_rva);
                }
            }
        }
        map.summary.vtables_instantiated_by_reachable_code = instantiated.size();
    }

    // ----- function-address runs in data ------------------------------------
    // Every located vtable's whole extent is excluded, not merely its base:
    // otherwise a slot the inventory does not cover splits a vtable into
    // fragments and each fragment counts as a table of its own.
    {
        const auto overlaps_a_vtable = [&](std::uint32_t begin, std::uint32_t end) {
            auto position = std::upper_bound(vtable_extents.begin(), vtable_extents.end(),
                                             std::pair{begin, std::numeric_limits<std::uint32_t>::max()});
            if (position != vtable_extents.begin()) {
                auto previous = std::prev(position);
                if (previous->second > begin) {
                    return true;
                }
            }
            return position != vtable_extents.end() && position->first < end;
        };

        for (const auto& section : image.sections) {
            if ((section.characteristics & 0x20000000U) != 0U) {
                continue;
            }
            const auto begin = static_cast<std::size_t>(section.raw_offset);
            const auto size = static_cast<std::size_t>(section.raw_size);
            std::size_t cursor = 0;
            while (cursor + 8U <= size) {
                const auto start = cursor;
                std::vector<std::uint32_t> run;
                while (cursor + 8U <= size) {
                    const auto value = read_u64(bytes, begin + cursor);
                    if (!value.has_value() || *value <= image.image_base) {
                        break;
                    }
                    const auto delta = *value - image.image_base;
                    if (delta > std::numeric_limits<std::uint32_t>::max() ||
                        !index.containing(static_cast<std::uint32_t>(delta)).has_value()) {
                        break;
                    }
                    run.push_back(static_cast<std::uint32_t>(delta));
                    cursor += 8U;
                }
                if (run.size() >= kMinFunctionPointerRun) {
                    const auto base = static_cast<std::uint32_t>(section.virtual_address + start);
                    const auto end = base + static_cast<std::uint32_t>(run.size() * 8U);
                    if (!overlaps_a_vtable(base, end)) {
                        FunctionPointerRun record;
                        record.base_rva = base;
                        record.entries = static_cast<std::uint32_t>(run.size());
                        for (const auto target : run) {
                            const auto owner = index.containing(target);
                            if (owner.has_value() &&
                                !map.functions[*owner].reachable_through_dispatch) {
                                ++record.entries_reaching_nothing_else;
                            }
                        }
                        for (const auto& walk : graph.functions) {
                            for (const auto reference : walk.data_references) {
                                if (reference >= base && reference < end) {
                                    ++record.referencing_functions;
                                    break;
                                }
                            }
                        }
                        map.function_pointer_runs.push_back(std::move(record));
                    }
                }
                if (cursor == start) {
                    cursor += 8U;
                }
            }
        }
        map.summary.function_pointer_runs = map.function_pointer_runs.size();
        for (const auto& run : map.function_pointer_runs) {
            map.summary.function_pointer_run_entries += run.entries;
        }
    }

    // ----- aggregates ------------------------------------------------------
    std::map<std::pair<std::string, std::string>, std::uint32_t> usage;
    std::map<std::string, std::uint32_t> family_functions;
    std::map<std::string, std::uint32_t> install_sites;
    std::map<std::uint32_t, std::uint32_t> dispatch_sites;
    std::map<std::uint32_t, std::uint32_t> table_referrers;
    std::map<std::uint32_t, std::uint32_t> table_base_references;
    std::map<std::uint32_t, std::set<std::uint32_t>> table_elements;
    map.summary.functions = map.functions.size();
    map.summary.indirect_call_sites = graph.indirect_call_sites;
    for (const auto& facts : map.functions) {
        if (facts.walk_complete) {
            ++map.summary.walks_complete;
        }
        if (!facts.virtual_bindings.empty()) {
            ++map.summary.with_virtual_binding;
        }
        if (!facts.imports_called.empty()) {
            ++map.summary.with_import_call;
        }
        if (facts.string_reference_count != 0U) {
            ++map.summary.with_string_reference;
        }
        if (facts.import_thunk) {
            ++map.summary.import_thunks;
        }
        if (facts.reachable_from_entry_point) {
            ++map.summary.reachable_from_entry_point;
        }
        if (facts.reachable_from_export) {
            ++map.summary.reachable_from_export;
        }
        if (facts.reachable_through_dispatch) {
            ++map.summary.reachable_through_dispatch;
        } else {
            ++map.summary.outside_every_closure;
        }
        if (facts.structurally_unreferenced()) {
            ++map.summary.structurally_unreferenced;
        }
        if (!facts.virtual_bindings.empty()) {
            ++map.summary.virtual_dispatch_candidates;
        }
        if (facts.reachable_from_entry_point || facts.reachable_from_export ||
            !facts.virtual_bindings.empty() || facts.exported) {
            ++map.summary.structurally_reachable;
        }
        if (!facts.virtual_bindings.empty() || !facts.imports_called.empty() ||
            facts.string_reference_count != 0U || facts.exported ||
            !facts.installs_vtables.empty() || !facts.name_tables.empty()) {
            ++map.summary.attributed;
        }

        if (facts.frame.uses_frame_pointer()) {
            ++map.summary.with_frame_pointer;
        }
        if (facts.frame.has_exception_handler) {
            ++map.summary.with_exception_handler;
        }
        if (is_leaf_frame(facts.frame)) {
            ++map.summary.leaf_functions;
        }
        map.summary.total_stack_allocation += facts.frame.stack_allocation;
        map.summary.largest_stack_allocation =
            std::max(map.summary.largest_stack_allocation, facts.frame.stack_allocation);

        if (!facts.name_tables.empty()) {
            ++map.summary.with_name_table;
        }
        if (!facts.installs_vtables.empty()) {
            ++map.summary.with_vtable_install;
        }
        if (!facts.resource_families.empty()) {
            ++map.summary.with_resource_family;
        }
        if (!facts.indirect_call_displacements.empty()) {
            ++map.summary.with_indirect_dispatch;
        }

        for (const auto& call : facts.imports_called) {
            ++usage[{call.module, call.function}];
        }
        for (const auto& family : facts.resource_families) {
            ++family_functions[family];
        }
        for (const auto displacement : facts.indirect_call_displacements) {
            ++dispatch_sites[displacement];
        }
        for (const auto& install : facts.installs_vtables) {
            ++install_sites[install.class_display_name];
        }

        std::set<std::uint32_t> tables_this_function_reaches;
        for (const auto& reference : facts.name_tables) {
            tables_this_function_reaches.insert(reference.table_base_rva);
            if (reference.offset_in_table == 0U) {
                ++table_base_references[reference.table_base_rva];
            }
            if (reference.constant_index) {
                ++map.summary.constant_index_references;
                table_elements[reference.table_base_rva].insert(reference.element_index);
            } else {
                ++map.summary.computed_index_references;
            }
        }
        for (const auto base : tables_this_function_reaches) {
            ++table_referrers[base];
        }
    }

    // Literals per family, counted independently of how many functions use them.
    std::map<std::string, std::uint32_t> family_literals;
    for (const auto& [rva, literal] : strings) {
        static_cast<void>(rva);
        for (const auto& family : resource_family_hints(literal)) {
            ++family_literals[family];
        }
    }
    for (const auto& [family, literals] : family_literals) {
        ResourceFamilyUsage entry;
        entry.family = family;
        entry.literals = literals;
        const auto referencing = family_functions.find(family);
        entry.referencing_functions =
            referencing == family_functions.end() ? 0U : referencing->second;
        map.resource_family_usage.push_back(std::move(entry));
    }
    std::sort(map.resource_family_usage.begin(), map.resource_family_usage.end(),
              [](const ResourceFamilyUsage& left, const ResourceFamilyUsage& right) {
                  if (left.referencing_functions != right.referencing_functions) {
                      return left.referencing_functions > right.referencing_functions;
                  }
                  return left.family < right.family;
              });

    // Which runs the code actually indexes as arrays.
    //
    // The address an indexed instruction forms is `base + displacement +
    // index * scale`, so the table it walks begins at base plus displacement,
    // and the scale is the element size the code assumes. A base register
    // holding the image base makes the displacement an absolute RVA, which is
    // how MSVC reaches a table it does not need a separate pointer for.
    //
    // Requiring the scale to equal the run's own element size is what makes
    // this worth recording: a coincidence would have to land on a recovered
    // base and agree about its stride.
    std::map<std::uint32_t, std::uint32_t> table_indexed_sites;
    struct ArrayFacts final {
        std::set<std::uint32_t> fields;
        std::set<std::uint32_t> functions;
        std::uint32_t sites{};
    };
    std::map<std::pair<std::uint32_t, std::uint32_t>, ArrayFacts> arrays;

    for (const auto& walk : graph.functions) {
        for (const auto& access : walk.indexed_accesses) {
            ++map.summary.indexed_accesses;
            if (access.base_rva == 0U) {
                ++map.summary.image_base_indexed_accesses;
            }

            const auto addressed = static_cast<std::uint32_t>(
                static_cast<std::int64_t>(access.base_rva) + access.displacement);
            if (const auto* table = table_containing(addressed);
                table != nullptr && addressed == table->base &&
                table->element_bytes == access.element_bytes) {
                ++table_indexed_sites[table->base];
                ++map.summary.indexed_table_accesses;
            }

            if (access.base_rva == 0U || access.element_bytes == 0U) {
                continue;
            }

            // On a held base the displacement is a field offset within the
            // element, so it has to land inside it.
            ++map.summary.held_base_accesses;
            if (access.displacement < 0) {
                ++map.summary.string_scan_accesses;
                continue;
            }
            if (static_cast<std::uint32_t>(access.displacement) >= access.element_bytes) {
                ++map.summary.inconsistent_array_accesses;
                continue;
            }

            ++map.summary.consistent_array_accesses;
            auto& facts = arrays[{access.base_rva, access.element_bytes}];
            facts.fields.insert(static_cast<std::uint32_t>(access.displacement));
            facts.functions.insert(walk.begin_rva);
            ++facts.sites;
        }
    }

    // Image-base reads: the array's start is folded into the displacement, so a
    // single read cannot separate base from field. Two reads that share a
    // function, an index register and an element size are walking one array, and
    // that is measured rather than guessed from how close their addresses are —
    // which matters, because most such groups turn out to span more than one
    // element, meaning the register was reused for a different array.
    std::set<std::pair<std::uint32_t, std::uint32_t>> image_base_derived;
    {
        std::map<std::tuple<std::uint32_t, std::uint8_t, std::uint32_t>,
                 std::set<std::int32_t>>
            groups;
        for (const auto& walk : graph.functions) {
            for (const auto& access : walk.indexed_accesses) {
                if (access.base_rva != 0U || access.element_bytes == 0U ||
                    access.displacement <= 0) {
                    continue;
                }
                groups[{walk.begin_rva, access.index_register, access.element_bytes}].insert(
                    access.displacement);
            }
        }

        map.summary.image_base_groups = groups.size();
        for (const auto& [key2, displacements] : groups) {
            if (displacements.size() < 2U) {
                continue;
            }
            ++map.summary.image_base_groups_with_several_reads;

            const auto element = std::get<2>(key2);
            const auto lowest = *displacements.begin();
            const auto highest = *displacements.rbegin();
            if (static_cast<std::uint32_t>(highest - lowest) >= element) {
                ++map.summary.image_base_groups_spanning_elements;
                continue;
            }

            // Where a register was also seen holding this base at this element
            // size, the two routes agree and the measured base stands; the
            // offsets merge into it rather than becoming a second array.
            const auto key = std::pair{static_cast<std::uint32_t>(lowest), element};
            auto& facts = arrays[key];
            if (facts.sites == 0U) {
                image_base_derived.insert(key);
            } else {
                ++map.summary.image_base_groups_corroborating;
            }
            facts.functions.insert(std::get<0>(key2));
            for (const auto displacement : displacements) {
                facts.fields.insert(static_cast<std::uint32_t>(displacement - lowest));
                ++facts.sites;
            }
        }
    }

    map.indexed_arrays.reserve(arrays.size());
    for (const auto& [key, facts] : arrays) {
        IndexedArray entry;
        entry.base_rva = key.first;
        entry.element_bytes = key.second;
        entry.sites = facts.sites;
        entry.referencing_functions = static_cast<std::uint32_t>(facts.functions.size());
        entry.field_offsets.assign(facts.fields.begin(), facts.fields.end());
        entry.base_measured = image_base_derived.find(key) == image_base_derived.end();
        map.indexed_arrays.push_back(std::move(entry));
    }
    map.summary.indexed_arrays = map.indexed_arrays.size();
    {
        std::map<std::uint32_t, std::set<std::uint32_t>> sizes_per_base;
        for (const auto& array : map.indexed_arrays) {
            sizes_per_base[array.base_rva].insert(array.element_bytes);
        }
        for (const auto& [base, sizes] : sizes_per_base) {
            static_cast<void>(base);
            if (sizes.size() > 1U) {
                ++map.summary.arrays_with_conflicting_element_size;
            }
        }
    }
    std::sort(map.indexed_arrays.begin(), map.indexed_arrays.end(),
              [](const IndexedArray& left, const IndexedArray& right) {
                  if (left.field_offsets.size() != right.field_offsets.size()) {
                      return left.field_offsets.size() > right.field_offsets.size();
                  }
                  if (left.sites != right.sites) {
                      return left.sites > right.sites;
                  }
                  return left.base_rva < right.base_rva;
              });

    // ----- class layout from constructor stores ----------------------------
    //
    // A constructor writes its class's vtable at offset zero; everything else
    // it writes into the object at a non-zero offset is what the object
    // contains. Where that vtable belongs to another class the offset names an
    // embedded member and its type; where it belongs to the same class it is a
    // base subobject, and the RTTI's own subobject offset says independently
    // where that sits.
    {
        std::map<std::uint32_t, std::pair<std::string, std::uint32_t>> vtable_class;
        if (inputs.rtti != nullptr) {
            for (const auto& entry : inputs.rtti->classes) {
                for (const auto& vtable : entry.vtables) {
                    vtable_class[vtable.vtable_rva] = {entry.display_name,
                                                       vtable.subobject_offset};
                }
            }
        }

        // Counted over every function, since a function can store a pointer
        // into its object without storing any vtable at all.
        for (const auto& walk : graph.functions) {
            for (const auto& store : walk.pointer_stores_into_this) {
                ++map.summary.pointer_stores_into_this;
                const auto callee = index.containing(store.callee_rva);
                if (callee.has_value() && !map.functions[*callee].constructs_class.empty()) {
                    ++map.summary.pointer_stores_from_a_constructor;
                }
            }
        }

        for (std::size_t position = 0; position < graph.functions.size(); ++position) {
            const auto& walk = graph.functions[position];
            if (walk.stores_into_this.empty()) {
                continue;
            }

            std::string owner;
            for (const auto& store : walk.stores_into_this) {
                if (store.offset != 0U) {
                    continue;
                }
                const auto found = vtable_class.find(store.stored_rva);
                if (found != vtable_class.end()) {
                    owner = found->second.first;
                }
            }
            if (!owner.empty()) {
                map.functions[position].constructs_class = owner;
                ++map.summary.constructors_identified;
            }

            for (const auto& store : walk.stores_into_this) {
                ++map.summary.stores_into_this;
                const auto found = vtable_class.find(store.stored_rva);
                if (found == vtable_class.end()) {
                    continue;
                }
                ++map.summary.stores_of_a_vtable;
                if (store.offset == 0U || owner.empty()) {
                    continue;
                }

                ClassFieldLayout field;
                field.class_display_name = owner;
                field.offset = store.offset;
                field.member_class_display_name = found->second.first;
                field.member_vtable_rva = store.stored_rva;
                field.site_rva = store.site_rva;
                field.embedded_member = found->second.first != owner;
                field.offset_confirmed_by_rtti = found->second.second == store.offset;
                map.class_field_layout.push_back(std::move(field));
            }
        }

        std::sort(map.class_field_layout.begin(), map.class_field_layout.end(),
                  [](const ClassFieldLayout& left, const ClassFieldLayout& right) {
                      if (left.class_display_name != right.class_display_name) {
                          return left.class_display_name < right.class_display_name;
                      }
                      if (left.offset != right.offset) {
                          return left.offset < right.offset;
                      }
                      return left.member_class_display_name < right.member_class_display_name;
                  });
        map.class_field_layout.erase(
            std::unique(map.class_field_layout.begin(), map.class_field_layout.end(),
                        [](const ClassFieldLayout& left, const ClassFieldLayout& right) {
                            return left.class_display_name == right.class_display_name &&
                                   left.offset == right.offset &&
                                   left.member_vtable_rva == right.member_vtable_rva;
                        }),
            map.class_field_layout.end());
        map.summary.field_layout_entries = map.class_field_layout.size();
        for (const auto& field : map.class_field_layout) {
            if (field.offset_confirmed_by_rtti) {
                ++map.summary.field_offsets_confirmed_by_rtti;
            }
        }
    }

    // ----- functions parameterised by a constant ---------------------------
    {
        std::map<std::uint32_t, std::pair<std::uint32_t, std::set<std::uint32_t>>> by_callee;
        for (const auto& walk : graph.functions) {
            for (const auto& call : walk.constant_argument_calls) {
                ++map.summary.constant_argument_calls;
                auto& entry = by_callee[call.callee_rva];
                ++entry.first;
                entry.second.insert(call.argument);
            }
        }

        constexpr std::size_t kMaxArgumentsListed = 24U;
        map.constant_argument_callees.reserve(by_callee.size());
        for (const auto& [callee, facts] : by_callee) {
            ConstantArgumentCallee entry;
            entry.callee_rva = callee;
            entry.call_sites = facts.first;
            entry.distinct_arguments = static_cast<std::uint32_t>(facts.second.size());
            for (const auto argument : facts.second) {
                if (entry.arguments.size() >= kMaxArgumentsListed) {
                    break;
                }
                entry.arguments.push_back(argument);
            }
            map.constant_argument_callees.push_back(std::move(entry));
        }
        map.summary.constant_argument_callees = map.constant_argument_callees.size();
        std::sort(map.constant_argument_callees.begin(), map.constant_argument_callees.end(),
                  [](const ConstantArgumentCallee& left, const ConstantArgumentCallee& right) {
                      if (left.call_sites != right.call_sites) {
                          return left.call_sites > right.call_sites;
                      }
                      return left.callee_rva < right.callee_rva;
                  });
    }

    // ----- virtual dispatch resolved through `this` ------------------------
    // What each class holds at each offset, so a call on a member can be given
    // the member's class.
    std::map<std::pair<std::string, std::uint32_t>, std::pair<std::string, std::uint32_t>>
        field_at;
    for (const auto& field : map.class_field_layout) {
        const auto key = std::pair{field.class_display_name, field.offset};
        const auto existing = field_at.find(key);
        if (existing == field_at.end()) {
            field_at.emplace(key, std::pair{field.member_class_display_name,
                                            field.member_vtable_rva});
        } else if (existing->second.second != field.member_vtable_rva) {
            // Two vtables at one offset: a member with a base of its own, or a
            // reading that is wrong. Either way the offset does not name one
            // thing, so it names none.
            existing->second = {};
        }
    }

    for (std::size_t position = 0; position < graph.functions.size(); ++position) {
        const auto& walk = graph.functions[position];
        const auto& facts = map.functions[position];

        // The class the enclosing function works on: the one whose vtable it
        // writes if it is a constructor, otherwise the one whose vtable it is
        // bound into. A method bound into more than one vtable is inherited,
        // and `this` then does not say which, so it says nothing.
        std::string enclosing;
        std::uint32_t enclosing_vtable_rva = 0U;
        if (!facts.constructs_class.empty()) {
            enclosing = facts.constructs_class;
            const auto primary = class_vtable_rva.find({enclosing, 0U});
            if (primary != class_vtable_rva.end()) {
                enclosing_vtable_rva = primary->second;
            }
        } else if (!facts.virtual_bindings.empty()) {
            std::set<std::pair<std::string, std::uint32_t>> vtables;
            for (const auto& binding : facts.virtual_bindings) {
                vtables.insert({binding.class_display_name, binding.vtable_index});
            }
            if (vtables.size() == 1U) {
                enclosing = vtables.begin()->first;
                const auto found = class_vtable_rva.find(*vtables.begin());
                if (found != class_vtable_rva.end()) {
                    enclosing_vtable_rva = found->second;
                }
            }
        }

        for (const auto& site : walk.resolved_dispatch_sites) {
            ++map.summary.dispatch_sites;
            if (!site.through_an_argument) {
                continue;
            }
            if (site.receiver_argument != 0U) {
                // The receiver is something the function was handed, not the
                // object it belongs to. Which argument it is says where to look
                // next; the enclosing class says nothing about it.
                ++map.summary.dispatch_sites_on_an_argument;
                continue;
            }
            ++map.summary.dispatch_sites_on_this;
            if (site.receiver_depth > 1U) {
                // The vtable came out of a pointer the object holds, so it is
                // the pointee's. What the enclosing class has at that offset is
                // the pointer, not the object, and nothing here says what it
                // points at.
                ++map.summary.dispatch_sites_through_a_pointer_member;
                continue;
            }
            if (facts.virtual_bindings.empty() && facts.constructs_class.empty()) {
                continue;
            }
            ++map.summary.dispatch_sites_in_a_bound_function;
            if (enclosing.empty()) {
                continue;
            }

            // At offset zero the receiver is the object itself, through the
            // vtable the enclosing function belongs to. At any other offset it
            // is the member sitting there, through that member's own primary
            // vtable — the member's vtable pointer is at its own offset zero,
            // which is what the load read.
            std::string receiver = enclosing;
            std::uint32_t receiver_vtable_rva = enclosing_vtable_rva;
            if (site.receiver_field_offset != 0U) {
                // A base subobject first: the RTTI says where each of a class's
                // own vtables sits, and a class that inherits a layout inherits
                // the offset with it, so this covers classes whose constructors
                // store nothing because a base constructor did it for them.
                const auto base = class_vtable_at_offset.find(
                    {enclosing, site.receiver_field_offset});
                if (base != class_vtable_at_offset.end()) {
                    receiver_vtable_rva = base->second;
                } else {
                    const auto member = field_at.find({enclosing, site.receiver_field_offset});
                    if (member == field_at.end() || member->second.first.empty()) {
                        continue;
                    }
                    receiver = member->second.first;
                    receiver_vtable_rva = member->second.second;
                }
            }
            if (receiver_vtable_rva == 0U) {
                continue;
            }

            const auto slot = site.displacement / 8U;
            const auto target = slot_targets.find({receiver_vtable_rva, slot});
            if (target == slot_targets.end()) {
                continue;
            }

            ++map.summary.dispatch_sites_resolved;
            if (site.receiver_field_offset != 0U) {
                ++map.summary.dispatch_sites_on_a_member;
            }
            map.resolved_dispatches.push_back(
                ResolvedDispatch{site.site_rva, walk.begin_rva, site.displacement, slot, receiver,
                                 target->second, site.receiver_field_offset});
        }
    }
    std::sort(map.resolved_dispatches.begin(), map.resolved_dispatches.end(),
              [](const ResolvedDispatch& left, const ResolvedDispatch& right) {
                  return left.site_rva < right.site_rva;
              });

    map.name_table_usage.reserve(table_spans.size());
    for (const auto& span : table_spans) {
        NameTableUsage usage;
        usage.table_base_rva = span.base;
        usage.record_layout = span.record_layout;
        usage.element_bytes = span.element_bytes;
        usage.entries = span.entries;
        const auto referrers = table_referrers.find(span.base);
        usage.referencing_functions = referrers == table_referrers.end() ? 0U : referrers->second;
        const auto bases = table_base_references.find(span.base);
        usage.base_references = bases == table_base_references.end() ? 0U : bases->second;
        const auto named = table_elements.find(span.base);
        usage.elements_named_by_constant =
            named == table_elements.end() ? 0U : static_cast<std::uint32_t>(named->second.size());
        const auto indexed = table_indexed_sites.find(span.base);
        usage.indexed_sites = indexed == table_indexed_sites.end() ? 0U : indexed->second;

        if (usage.referencing_functions == 0U) {
            ++map.summary.name_tables_unreferenced;
        } else {
            ++map.summary.name_tables_referenced;
        }
        map.name_table_usage.push_back(std::move(usage));
    }
    std::sort(map.name_table_usage.begin(), map.name_table_usage.end(),
              [](const NameTableUsage& left, const NameTableUsage& right) {
                  if (left.referencing_functions != right.referencing_functions) {
                      return left.referencing_functions > right.referencing_functions;
                  }
                  return left.table_base_rva < right.table_base_rva;
              });

    map.dispatch_slots.reserve(dispatch_sites.size());
    for (const auto& [displacement, sites] : dispatch_sites) {
        map.dispatch_slots.push_back(
            DispatchSlotUsage{displacement, displacement / 8U, sites});
    }
    std::sort(map.dispatch_slots.begin(), map.dispatch_slots.end(),
              [](const DispatchSlotUsage& left, const DispatchSlotUsage& right) {
                  if (left.call_sites != right.call_sites) {
                      return left.call_sites > right.call_sites;
                  }
                  return left.displacement < right.displacement;
              });

    for (auto& entry : map.class_coverage) {
        const auto sites = install_sites.find(entry.class_display_name);
        entry.install_sites = sites == install_sites.end() ? 0U : sites->second;
    }

    // Both of the following read `constructs_class`, which the constructor
    // identification above is what fills in. They ran before it once, and the
    // constructor half of each measurement silently contributed nothing.
    // ----- fixed addresses the code operates on -----------------------------
    // An address handed to a call in the register the convention reserves for
    // the first argument is something the callee works on. What makes a block
    // interesting is not one such call but many, from many places, into many
    // entry points.
    {
        std::map<std::uint32_t, std::set<std::uint32_t>> arguments_of_callee;
        std::map<std::uint32_t, std::uint32_t> image_calls_to_callee;
        for (const auto& walk : graph.functions) {
            for (const auto& call : walk.object_argument_calls) {
                arguments_of_callee[call.callee_rva].insert(call.object_rva);
                ++image_calls_to_callee[call.callee_rva];
            }
        }

        struct Block final {
            std::uint32_t call_sites{};
            std::set<std::uint32_t> callees;
            std::set<std::uint32_t> callers;
            std::uint32_t reach{};
            std::string constructed_class;
        };
        std::map<std::uint32_t, Block> blocks;
        for (std::size_t position = 0; position < graph.functions.size(); ++position) {
            for (const auto& call : graph.functions[position].object_argument_calls) {
                auto& block = blocks[call.object_rva];
                ++block.call_sites;
                block.callees.insert(call.callee_rva);
                block.callers.insert(map.functions[position].begin_rva);
                ++map.summary.global_block_call_sites;

                const auto callee = index.containing(call.callee_rva);
                if (!callee.has_value() ||
                    map.functions[*callee].begin_rva != call.callee_rva) {
                    continue;
                }
                if (block.constructed_class.empty()) {
                    block.constructed_class = map.functions[*callee].constructs_class;
                }
                // Only a callee this block has to itself can lend its reach.
                if (arguments_of_callee[call.callee_rva].size() != 1U) {
                    ++map.summary.global_block_sites_pooled;
                    continue;
                }
                if (map.functions[*callee].caller_count !=
                    image_calls_to_callee[call.callee_rva]) {
                    ++map.summary.global_block_sites_with_other_callers;
                    continue;
                }
                const auto& offsets = graph.functions[*callee].entry_field_offsets;
                if (!offsets.empty()) {
                    block.reach = std::max(block.reach, offsets.back() + 1U);
                }
            }
        }

        std::vector<std::uint32_t> addresses;
        addresses.reserve(blocks.size());
        for (const auto& [address, block] : blocks) {
            static_cast<void>(block);
            addresses.push_back(address);
        }

        for (std::size_t position = 0; position < addresses.size(); ++position) {
            const auto address = addresses[position];
            const auto& block = blocks[address];
            GlobalStateBlock record;
            record.base_rva = address;
            for (const auto& section : image.sections) {
                if (section.contains_rva(address)) {
                    record.section = section.name;
                    break;
                }
            }
            record.call_sites = block.call_sites;
            record.distinct_callees = static_cast<std::uint32_t>(block.callees.size());
            record.distinct_callers = static_cast<std::uint32_t>(block.callers.size());
            record.field_reach = block.reach;
            record.constructed_class = block.constructed_class;
            if (position + 1U < addresses.size()) {
                record.bytes_to_next_block = addresses[position + 1U] - address;
                if (block.reach != 0U) {
                    ++map.summary.global_blocks_with_a_field_reach;
                    if (block.reach > record.bytes_to_next_block) {
                        record.reach_runs_past_the_next_block = true;
                        ++map.summary.global_blocks_reach_past_the_gap;
                    } else {
                        ++map.summary.global_blocks_reach_within_the_gap;
                    }
                }
            } else if (block.reach != 0U) {
                ++map.summary.global_blocks_with_a_field_reach;
            }
            if (!record.constructed_class.empty()) {
                ++map.summary.global_blocks_with_a_class;
            }
            map.global_state_blocks.push_back(std::move(record));
        }
        std::sort(map.global_state_blocks.begin(), map.global_state_blocks.end(),
                  [](const GlobalStateBlock& left, const GlobalStateBlock& right) {
                      if (left.distinct_callers != right.distinct_callers) {
                          return left.distinct_callers > right.distinct_callers;
                      }
                      return left.base_rva < right.base_rva;
                  });
        map.summary.global_state_blocks = map.global_state_blocks.size();
    }

    // ----- class size floors -----------------------------------------------
    // Nothing here reads a field. One source is where the hierarchy descriptor
    // places each base subobject; the other is how far into the object the
    // class's own methods reach, the first argument to such a method being the
    // object by the Microsoft x64 convention.
    if (inputs.rtti != nullptr) {
        struct Floor final {
            std::uint32_t from_bases{};
            std::string deepest_base;
            std::uint32_t from_access{};
            std::vector<std::uint32_t> per_function;
        };
        std::map<std::string, Floor> floors;

        for (const auto& entry : inputs.rtti->classes) {
            std::set<std::uint32_t> offsets_with_a_vtable;
            for (const auto& vtable : entry.vtables) {
                if (vtable.vtable_rva != 0U) {
                    offsets_with_a_vtable.insert(vtable.subobject_offset);
                }
            }
            auto& floor = floors[entry.display_name];
            // The class's own vtable pointer sits at offset zero, so eight
            // bytes is the floor every polymorphic class starts from.
            floor.from_bases = 8U;
            floor.deepest_base = entry.display_name;
            for (const auto& base : entry.hierarchy) {
                if (base.member_displacement < 0) {
                    continue;
                }
                const auto displacement = static_cast<std::uint32_t>(base.member_displacement);
                if (offsets_with_a_vtable.count(displacement) == 0U) {
                    continue;
                }
                if (displacement + 8U > floor.from_bases) {
                    floor.from_bases = displacement + 8U;
                    floor.deepest_base = base.display_name;
                }
            }
        }

        for (std::size_t position = 0; position < map.functions.size(); ++position) {
            const auto& walk = graph.functions[position];
            if (walk.entry_field_offsets.empty()) {
                continue;
            }
            const auto deepest = walk.entry_field_offsets.back();
            const auto& facts = map.functions[position];

            // A method bound at a non-zero subobject offset is handed a pointer
            // to that subobject, so an offset inside it sits that much further
            // into the complete object.
            std::map<std::string, std::uint32_t> reach;
            for (const auto& binding : facts.virtual_bindings) {
                auto& value = reach[binding.class_display_name];
                value = std::max(value, binding.subobject_offset + deepest + 1U);
            }
            if (!facts.constructs_class.empty()) {
                auto& value = reach[facts.constructs_class];
                value = std::max(value, deepest + 1U);
            }
            for (const auto& [name, value] : reach) {
                auto& floor = floors[name];
                floor.from_access = std::max(floor.from_access, value);
                floor.per_function.push_back(value);
            }
        }

        for (auto& [name, floor] : floors) {
            ClassSizeFloor record;
            record.class_display_name = name;
            record.floor_from_bases = floor.from_bases;
            record.deepest_base_display_name = floor.deepest_base;
            record.floor_from_field_access = floor.from_access;
            record.floor_bytes = std::max(floor.from_bases, floor.from_access);
            record.functions_speaking = static_cast<std::uint32_t>(floor.per_function.size());
            for (const auto value : floor.per_function) {
                if (value * 2U >= record.floor_bytes) {
                    ++record.functions_reaching_half;
                }
            }
            if (floor.from_access != 0U && floor.from_bases > floor.from_access) {
                ++map.summary.size_floors_where_bases_say_more;
            }
            if (record.floor_bytes > 8U) {
                ++map.summary.size_floors_above_a_vtable_pointer;
            }
            if (record.functions_reaching_half >= 2U) {
                ++map.summary.size_floors_corroborated;
            } else if (record.functions_speaking > 1U) {
                ++map.summary.size_floors_on_a_lone_outlier;
            }
            map.class_size_floors.push_back(std::move(record));
        }
        std::sort(map.class_size_floors.begin(), map.class_size_floors.end(),
                  [](const ClassSizeFloor& left, const ClassSizeFloor& right) {
                      if (left.floor_bytes != right.floor_bytes) {
                          return left.floor_bytes > right.floor_bytes;
                      }
                      return left.class_display_name < right.class_display_name;
                  });
        map.summary.class_size_floors = map.class_size_floors.size();
    }

    map.import_usage.reserve(usage.size());
    for (const auto& [key, count] : usage) {
        map.import_usage.push_back(ImportUsage{key.first, key.second, count});
    }
    std::sort(map.import_usage.begin(), map.import_usage.end(),
              [](const ImportUsage& left, const ImportUsage& right) {
                  if (left.calling_functions != right.calling_functions) {
                      return left.calling_functions > right.calling_functions;
                  }
                  if (left.module != right.module) {
                      return left.module < right.module;
                  }
                  return left.function < right.function;
              });

    return map;
}

} // namespace dmc::rengine::exe
