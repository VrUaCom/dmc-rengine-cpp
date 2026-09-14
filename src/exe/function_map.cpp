#include "dmc_rengine/exe/function_map.hpp"

#include "dmc_rengine/exe/x86_decoder.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>

namespace dmc::rengine::exe {
namespace {

using detail::read_cstring;
using detail::read_u64;

constexpr std::size_t kMinStringLength = 4U;
constexpr std::size_t kMaxStringLength = 200U;
constexpr std::size_t kMaxStringsPerFunction = 8U;
constexpr std::size_t kMaxVtableSlotsRead = 4096U;

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

} // namespace

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

    const FunctionIndex index{graph.functions};
    map.functions.resize(graph.functions.size());
    for (std::size_t position = 0; position < graph.functions.size(); ++position) {
        const auto& walk = graph.functions[position];
        auto& facts = map.functions[position];
        facts.begin_rva = walk.begin_rva;
        facts.end_rva = walk.end_rva;
        facts.instruction_count = walk.instruction_count;
        facts.walk_complete = walk.complete;
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

    // ----- virtual method bindings -----------------------------------------
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

        for (const auto target : walk.data_references) {
            if (const auto slot = iat_slots.find(target); slot != iat_slots.end()) {
                facts.imports_called.push_back(slot->second);
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
            }
        }

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

            // Calling a thunk is calling the import behind it.
            if (const auto thunk = thunk_imports.find(*callee); thunk != thunk_imports.end()) {
                facts.imports_called.push_back(thunk->second);
            }
        }
    }

    for (auto& facts : map.functions) {
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

    // ----- aggregates ------------------------------------------------------
    std::map<std::pair<std::string, std::string>, std::uint32_t> usage;
    map.summary.functions = map.functions.size();
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
            facts.string_reference_count != 0U || facts.exported) {
            ++map.summary.attributed;
        }

        for (const auto& call : facts.imports_called) {
            ++usage[{call.module, call.function}];
        }
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
