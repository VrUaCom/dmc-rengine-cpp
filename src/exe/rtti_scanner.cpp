#include "dmc_rengine/exe/rtti_scanner.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace dmc::rengine::exe {
namespace {

using detail::has_range;
using detail::read_cstring;
using detail::read_u32;
using detail::read_u64;

constexpr std::size_t kMaxBaseClasses = 1024U;
constexpr std::size_t kMaxVtableSlots = 4096U;
constexpr std::size_t kMaxTypeNameLength = 1024U;

[[nodiscard]] bool is_initialized_data(const PeSection& section) noexcept {
    // IMAGE_SCN_CNT_INITIALIZED_DATA or IMAGE_SCN_CNT_CODE. RTTI lives in
    // read-only data, but nothing forbids a linker from placing it elsewhere.
    constexpr std::uint32_t initialized_data = 0x00000040U;
    constexpr std::uint32_t code = 0x00000020U;
    return section.raw_size > 0U && (section.characteristics & (initialized_data | code)) != 0U;
}

[[nodiscard]] bool is_executable(const PeSection& section) noexcept {
    constexpr std::uint32_t executable = 0x20000000U;
    return (section.characteristics & executable) != 0U;
}

/// Largest readable extent of a section within the file.
[[nodiscard]] std::size_t readable_size(std::span<const std::byte> bytes,
                                        const PeSection& section) noexcept {
    const auto begin = static_cast<std::size_t>(section.raw_offset);
    if (begin >= bytes.size()) {
        return 0U;
    }
    const auto available = bytes.size() - begin;
    const auto declared = static_cast<std::size_t>(std::min(section.raw_size, section.virtual_size != 0U
                                                                ? std::max(section.virtual_size,
                                                                           section.raw_size)
                                                                : section.raw_size));
    return std::min(available, declared);
}

// ---------------------------------------------------------------------------
// Decorated-name reconstruction
// ---------------------------------------------------------------------------

class NameParser final {
public:
    explicit NameParser(std::string_view text) : text_(text) {}

    [[nodiscard]] bool complete() const noexcept { return complete_; }

    [[nodiscard]] std::string parse_qualified_name() {
        std::vector<std::string> fragments;
        while (position_ < text_.size()) {
            if (text_[position_] == '@') {
                ++position_;
                break;
            }

            fragments.push_back(parse_fragment());
            if (position_ < text_.size() && text_[position_] == '@') {
                ++position_;
            } else {
                break;
            }
        }

        // MSVC records qualifiers innermost first.
        std::string joined;
        for (auto entry = fragments.rbegin(); entry != fragments.rend(); ++entry) {
            if (!joined.empty()) {
                joined += "::";
            }
            joined += *entry;
        }
        return joined;
    }

private:
    [[nodiscard]] std::string parse_fragment() {
        if (text_.compare(position_, 2U, "?$") == 0) {
            return parse_template_fragment();
        }

        const auto begin = position_;
        while (position_ < text_.size() && text_[position_] != '@') {
            ++position_;
        }
        return std::string{text_.substr(begin, position_ - begin)};
    }

    [[nodiscard]] std::string parse_template_fragment() {
        position_ += 2U; // "?$"

        const auto name_begin = position_;
        while (position_ < text_.size() && text_[position_] != '@') {
            ++position_;
        }
        std::string name{text_.substr(name_begin, position_ - name_begin)};
        if (position_ < text_.size()) {
            ++position_; // '@' after the template name
        }

        std::string arguments;
        while (position_ < text_.size() && text_[position_] != '@') {
            if (!arguments.empty()) {
                arguments += ", ";
            }
            arguments += parse_argument();
        }
        if (position_ < text_.size()) {
            ++position_; // '@' closing the argument list
        }

        return name + "<" + arguments + ">";
    }

    [[nodiscard]] std::string parse_argument() {
        if (position_ >= text_.size()) {
            complete_ = false;
            return "?";
        }

        const char code = text_[position_];
        switch (code) {
        case 'V':
        case 'U':
        case 'T':
            ++position_;
            return parse_qualified_name();
        case 'W':
            ++position_;
            if (position_ < text_.size() && text_[position_] == '4') {
                ++position_;
            }
            return parse_qualified_name();
        case 'C': ++position_; return "signed char";
        case 'D': ++position_; return "char";
        case 'E': ++position_; return "unsigned char";
        case 'F': ++position_; return "short";
        case 'G': ++position_; return "unsigned short";
        case 'H': ++position_; return "int";
        case 'I': ++position_; return "unsigned int";
        case 'J': ++position_; return "long";
        case 'K': ++position_; return "unsigned long";
        case 'M': ++position_; return "float";
        case 'N': ++position_; return "double";
        case 'X': ++position_; return "void";
        case '_': return parse_extended_basic_type();
        case 'P':
        case 'Q':
        case 'A':
        case 'R':
        case 'S':
            return parse_indirection(code);
        default:
            break;
        }

        // Unmodelled construct: keep the remaining text rather than guess at it.
        complete_ = false;
        const auto begin = position_;
        while (position_ < text_.size() && text_[position_] != '@') {
            ++position_;
        }
        if (position_ == begin) {
            ++position_;
        }
        return std::string{text_.substr(begin, position_ - begin)};
    }

    [[nodiscard]] std::string parse_extended_basic_type() {
        ++position_; // '_'
        if (position_ >= text_.size()) {
            complete_ = false;
            return "?";
        }

        const char code = text_[position_++];
        switch (code) {
        case 'N': return "bool";
        case 'J': return "__int64";
        case 'K': return "unsigned __int64";
        case 'W': return "wchar_t";
        default: break;
        }

        complete_ = false;
        return std::string{"_"} + code;
    }

    [[nodiscard]] std::string parse_indirection(char code) {
        ++position_; // pointer/reference class
        // A 64-bit target inserts 'E' before the cv-qualifier.
        if (position_ < text_.size() && text_[position_] == 'E') {
            ++position_;
        }
        if (position_ < text_.size() &&
            (text_[position_] == 'A' || text_[position_] == 'B' || text_[position_] == 'C' ||
             text_[position_] == 'D')) {
            ++position_;
        }

        const std::string pointee = parse_argument();
        return pointee + (code == 'A' || code == 'R' ? "&" : "*");
    }

    std::string_view text_;
    std::size_t position_{};
    bool complete_{true};
};

struct TypeDescriptor final {
    std::uint32_t rva{};
    std::string decorated;
};

struct ScanContext final {
    std::span<const std::byte> bytes;
    const PeImage* image{};

    [[nodiscard]] std::optional<std::size_t> offset_of(std::uint32_t rva) const {
        const auto mapped = image->rva_to_file_offset(rva);
        if (!mapped.has_value()) {
            return std::nullopt;
        }
        const auto offset = static_cast<std::size_t>(*mapped);
        return offset < bytes.size() ? std::optional<std::size_t>{offset} : std::nullopt;
    }

    [[nodiscard]] bool rva_is_executable(std::uint32_t rva) const {
        for (const auto& section : image->sections) {
            if (is_executable(section) && section.contains_rva(rva)) {
                return true;
            }
        }
        return false;
    }
};

/// Locates every `type_info` descriptor by its decorated-name payload.
[[nodiscard]] std::unordered_map<std::uint32_t, std::string> collect_type_descriptors(
    const ScanContext& context, RttiScanResult& result) {
    std::unordered_map<std::uint32_t, std::string> descriptors;

    // On a 64-bit image the name follows a vftable pointer and a spare
    // pointer; a 32-bit image halves both.
    const std::uint32_t name_displacement = context.image->kind == PeKind::pe32_plus ? 16U : 8U;

    for (const auto& section : context.image->sections) {
        if (!is_initialized_data(section)) {
            continue;
        }

        const auto begin = static_cast<std::size_t>(section.raw_offset);
        const auto size = readable_size(context.bytes, section);
        if (size < 4U) {
            continue;
        }

        for (std::size_t index = 0; index + 4U <= size; ++index) {
            const auto offset = begin + index;
            if (std::to_integer<unsigned char>(context.bytes[offset]) != '.' ||
                std::to_integer<unsigned char>(context.bytes[offset + 1U]) != '?' ||
                std::to_integer<unsigned char>(context.bytes[offset + 2U]) != 'A') {
                continue;
            }

            auto decorated = read_cstring(context.bytes, offset, kMaxTypeNameLength);
            if (!decorated.has_value() || decorated->size() < 6U ||
                !decorated->ends_with("@@")) {
                continue;
            }

            const auto name_rva = static_cast<std::uint32_t>(section.virtual_address + index);
            if (name_rva < name_displacement) {
                continue;
            }

            const auto descriptor_rva = name_rva - name_displacement;
            if (!section.contains_rva(descriptor_rva)) {
                // The descriptor header must live in the same section as its
                // name; a match that straddles the boundary is a false hit.
                continue;
            }

            descriptors.emplace(descriptor_rva, std::move(*decorated));
        }
    }

    result.type_descriptors = descriptors.size();
    return descriptors;
}

struct LocatorRecord final {
    std::uint32_t locator_rva{};
    std::uint32_t type_descriptor_rva{};
    std::uint32_t hierarchy_rva{};
    std::uint32_t subobject_offset{};
    std::uint32_t constructor_displacement_offset{};
};

/// Finds complete-object locators by their self-reference.
///
/// A 64-bit locator stores its own RVA in the last field. Requiring that field
/// to match the candidate position rejects essentially every coincidental
/// byte pattern, which is what makes an unanchored scan trustworthy here.
[[nodiscard]] std::vector<LocatorRecord> collect_locators(
    const ScanContext& context, const std::unordered_map<std::uint32_t, std::string>& descriptors,
    RttiScanResult& result) {
    std::vector<LocatorRecord> locators;
    if (context.image->kind != PeKind::pe32_plus) {
        result.warnings.emplace_back(
            "Complete-object-locator scanning requires the 64-bit self-referencing layout.");
        return locators;
    }

    for (const auto& section : context.image->sections) {
        if (!is_initialized_data(section) || is_executable(section)) {
            continue;
        }

        const auto begin = static_cast<std::size_t>(section.raw_offset);
        const auto size = readable_size(context.bytes, section);
        if (size < 24U) {
            continue;
        }

        for (std::size_t index = 0; index + 24U <= size; index += 4U) {
            const auto offset = begin + index;
            const auto signature = read_u32(context.bytes, offset);
            if (!signature.has_value() || *signature != 1U) {
                continue;
            }

            const auto self_rva = read_u32(context.bytes, offset + 20U);
            const auto candidate_rva = static_cast<std::uint32_t>(section.virtual_address + index);
            if (!self_rva.has_value() || *self_rva != candidate_rva) {
                continue;
            }

            const auto descriptor_rva = read_u32(context.bytes, offset + 12U);
            const auto hierarchy_rva = read_u32(context.bytes, offset + 16U);
            const auto subobject_offset = read_u32(context.bytes, offset + 4U);
            const auto constructor_displacement = read_u32(context.bytes, offset + 8U);
            if (!descriptor_rva.has_value() || !hierarchy_rva.has_value() ||
                !subobject_offset.has_value() || !constructor_displacement.has_value()) {
                continue;
            }
            if (descriptors.find(*descriptor_rva) == descriptors.end()) {
                continue;
            }

            locators.push_back(LocatorRecord{candidate_rva, *descriptor_rva, *hierarchy_rva,
                                             *subobject_offset, *constructor_displacement});
        }
    }

    result.locators = locators.size();
    return locators;
}

void read_hierarchy(const ScanContext& context,
                    const std::unordered_map<std::uint32_t, std::string>& descriptors,
                    const LocatorRecord& locator, RttiClass& entry, RttiScanResult& result) {
    const auto hierarchy_offset = context.offset_of(locator.hierarchy_rva);
    if (!hierarchy_offset.has_value()) {
        result.warnings.emplace_back("Class hierarchy descriptor for '" + entry.decorated_name +
                                     "' does not map to file data.");
        return;
    }

    const auto attributes = read_u32(context.bytes, *hierarchy_offset + 4U);
    const auto base_count = read_u32(context.bytes, *hierarchy_offset + 8U);
    const auto array_rva = read_u32(context.bytes, *hierarchy_offset + 12U);
    if (!attributes.has_value() || !base_count.has_value() || !array_rva.has_value()) {
        result.warnings.emplace_back("Class hierarchy descriptor for '" + entry.decorated_name +
                                     "' is truncated.");
        return;
    }

    entry.multiple_inheritance = (*attributes & 1U) != 0U;
    entry.virtual_inheritance = (*attributes & 2U) != 0U;
    entry.ambiguous = (*attributes & 4U) != 0U;

    if (*base_count > kMaxBaseClasses) {
        result.warnings.emplace_back("Class '" + entry.decorated_name +
                                     "' declares more bases than the safety limit allows.");
        return;
    }

    const auto array_offset = context.offset_of(*array_rva);
    if (!array_offset.has_value()) {
        result.warnings.emplace_back("Base-class array for '" + entry.decorated_name +
                                     "' does not map to file data.");
        return;
    }

    entry.hierarchy.reserve(*base_count);
    for (std::size_t index = 0; index < static_cast<std::size_t>(*base_count); ++index) {
        const auto base_rva = read_u32(context.bytes, *array_offset + index * 4U);
        if (!base_rva.has_value()) {
            result.warnings.emplace_back("Base-class array for '" + entry.decorated_name +
                                         "' is truncated.");
            return;
        }

        const auto base_offset = context.offset_of(*base_rva);
        if (!base_offset.has_value()) {
            continue;
        }

        const auto base_descriptor_rva = read_u32(context.bytes, *base_offset);
        const auto contained = read_u32(context.bytes, *base_offset + 4U);
        const auto member_displacement = read_u32(context.bytes, *base_offset + 8U);
        const auto vbtable_displacement = read_u32(context.bytes, *base_offset + 12U);
        const auto vbtable_index = read_u32(context.bytes, *base_offset + 16U);
        if (!base_descriptor_rva.has_value() || !contained.has_value() ||
            !member_displacement.has_value() || !vbtable_displacement.has_value() ||
            !vbtable_index.has_value()) {
            continue;
        }

        const auto named = descriptors.find(*base_descriptor_rva);
        if (named == descriptors.end()) {
            continue;
        }

        RttiBaseClass base;
        base.decorated_name = named->second;
        base.display_name = reconstruct_type_name(base.decorated_name).display;
        base.contained_bases = *contained;
        base.member_displacement = static_cast<std::int32_t>(*member_displacement);
        base.vbtable_displacement = static_cast<std::int32_t>(*vbtable_displacement);
        base.vbtable_index = static_cast<std::int32_t>(*vbtable_index);
        entry.hierarchy.push_back(std::move(base));
    }
}

/// Attaches vtables by finding the pointer that precedes each one.
///
/// MSVC stores the locator pointer immediately before the first virtual slot,
/// so a data word equal to `image_base + locator_rva` marks a vtable at the
/// next pointer.
void attach_vtables(const ScanContext& context, std::vector<RttiClass>& classes,
                    RttiScanResult& result) {
    struct VtableSlotRef final {
        std::size_t class_index{};
        std::size_t vtable_index{};
    };

    std::unordered_map<std::uint32_t, VtableSlotRef> by_locator;
    for (std::size_t class_index = 0; class_index < classes.size(); ++class_index) {
        const auto& entry = classes[class_index];
        for (std::size_t vtable_index = 0; vtable_index < entry.vtables.size(); ++vtable_index) {
            by_locator.emplace(entry.vtables[vtable_index].complete_object_locator_rva,
                               VtableSlotRef{class_index, vtable_index});
        }
    }

    const std::uint64_t image_base = context.image->image_base;
    for (const auto& section : context.image->sections) {
        if (!is_initialized_data(section) || is_executable(section)) {
            continue;
        }

        const auto begin = static_cast<std::size_t>(section.raw_offset);
        const auto size = readable_size(context.bytes, section);
        if (size < 16U) {
            continue;
        }

        for (std::size_t index = 0; index + 8U <= size; index += 8U) {
            const auto value = read_u64(context.bytes, begin + index);
            if (!value.has_value() || *value <= image_base) {
                continue;
            }

            const auto delta = *value - image_base;
            if (delta > std::numeric_limits<std::uint32_t>::max()) {
                continue;
            }

            const auto match = by_locator.find(static_cast<std::uint32_t>(delta));
            if (match == by_locator.end()) {
                continue;
            }

            auto& entry = classes[match->second.class_index];
            auto& vtable = entry.vtables[match->second.vtable_index];
            const auto vtable_rva =
                static_cast<std::uint32_t>(section.virtual_address + index + 8U);
            if (vtable.vtable_rva != 0U && vtable.vtable_rva != vtable_rva) {
                // One locator referenced from two places is not something a
                // normal MSVC image produces; record it instead of picking.
                entry.ambiguous = true;
                continue;
            }
            vtable.vtable_rva = vtable_rva;

            std::uint32_t slots = 0U;
            for (std::size_t slot = 0; slot < kMaxVtableSlots; ++slot) {
                const auto slot_offset = begin + index + 8U + slot * 8U;
                if (slot_offset + 8U > begin + size) {
                    break;
                }
                const auto pointer = read_u64(context.bytes, slot_offset);
                if (!pointer.has_value() || *pointer <= image_base) {
                    break;
                }
                const auto target = *pointer - image_base;
                if (target > std::numeric_limits<std::uint32_t>::max() ||
                    !context.rva_is_executable(static_cast<std::uint32_t>(target))) {
                    break;
                }
                ++slots;
            }
            vtable.slot_count = slots;
            ++result.vtables_located;
        }
    }
}

} // namespace

DecoratedName reconstruct_type_name(std::string_view decorated) {
    DecoratedName reconstructed;

    // Every RTTI name begins with ".?A" followed by a one- or two-character
    // type-kind code.
    if (!decorated.starts_with(".?A") || decorated.size() < 5U) {
        reconstructed.display = std::string{decorated};
        reconstructed.complete = false;
        return reconstructed;
    }

    std::size_t cursor = 3U;
    const char kind = decorated[cursor++];
    if (kind == 'W' && cursor < decorated.size() && decorated[cursor] == '4') {
        ++cursor;
    } else if (kind != 'V' && kind != 'U' && kind != 'T' && kind != 'X') {
        reconstructed.display = std::string{decorated};
        reconstructed.complete = false;
        return reconstructed;
    }

    NameParser parser{decorated.substr(cursor)};
    reconstructed.display = parser.parse_qualified_name();
    reconstructed.complete = parser.complete();
    if (reconstructed.display.empty()) {
        reconstructed.display = std::string{decorated};
        reconstructed.complete = false;
    }
    return reconstructed;
}

RttiScanResult RttiScanner::scan(std::span<const std::byte> bytes, const PeImage& image) {
    RttiScanResult result;
    const ScanContext context{bytes, &image};

    const auto descriptors = collect_type_descriptors(context, result);
    if (descriptors.empty()) {
        result.warnings.emplace_back("No MSVC type descriptors found.");
        return result;
    }

    const auto locators = collect_locators(context, descriptors, result);

    // Several locators can describe the same type: multiple inheritance gives
    // a type one locator and one vtable per base subobject. Group by type
    // descriptor so the report counts types as types.
    std::unordered_map<std::uint32_t, std::size_t> class_by_descriptor;
    for (const auto& locator : locators) {
        const auto existing = class_by_descriptor.find(locator.type_descriptor_rva);
        if (existing == class_by_descriptor.end()) {
            RttiClass entry;
            entry.decorated_name = descriptors.at(locator.type_descriptor_rva);
            const auto reconstructed = reconstruct_type_name(entry.decorated_name);
            entry.display_name = reconstructed.display;
            entry.display_name_complete = reconstructed.complete;
            entry.type_descriptor_rva = locator.type_descriptor_rva;
            entry.class_hierarchy_rva = locator.hierarchy_rva;

            read_hierarchy(context, descriptors, locator, entry, result);
            class_by_descriptor.emplace(locator.type_descriptor_rva, result.classes.size());
            result.classes.push_back(std::move(entry));
        } else if (result.classes[existing->second].class_hierarchy_rva != locator.hierarchy_rva) {
            result.warnings.emplace_back(
                "Type '" + descriptors.at(locator.type_descriptor_rva) +
                "' has locators pointing at different class hierarchy descriptors.");
        }

        auto& entry = result.classes[class_by_descriptor.at(locator.type_descriptor_rva)];
        entry.vtables.push_back(RttiVtable{locator.locator_rva, 0U, 0U, locator.subobject_offset,
                                           locator.constructor_displacement_offset});
    }

    attach_vtables(context, result.classes, result);

    for (auto& entry : result.classes) {
        std::sort(entry.vtables.begin(), entry.vtables.end(),
                  [](const RttiVtable& left, const RttiVtable& right) {
                      if (left.subobject_offset != right.subobject_offset) {
                          return left.subobject_offset < right.subobject_offset;
                      }
                      return left.complete_object_locator_rva < right.complete_object_locator_rva;
                  });
    }

    std::sort(result.classes.begin(), result.classes.end(),
              [](const RttiClass& left, const RttiClass& right) {
                  if (left.display_name != right.display_name) {
                      return left.display_name < right.display_name;
                  }
                  return left.type_descriptor_rva < right.type_descriptor_rva;
              });
    return result;
}

} // namespace dmc::rengine::exe
