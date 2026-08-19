#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] constexpr unsigned char u8(std::byte value) noexcept {
    return std::to_integer<unsigned char>(value);
}

[[nodiscard]] bool is_nested_list_token(std::string_view token) noexcept {
    return token.size() >= 4U && token.ends_with(".lst");
}

[[nodiscard]] std::string join_child_path(
    std::string_view base, std::string_view token) {
    std::string result;
    result.reserve(base.size() + token.size());
    result.append(base);
    result.append(token);
    return result;
}

void write_u32_le(std::vector<std::byte>& bytes,
                  std::size_t offset,
                  std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

struct ChildImage final {
    std::string selected_path;
    std::vector<std::byte> bytes;
    bool dummy{false};
};

struct SynthesisContext final {
    const LooseContainerRead& read;
    LooseContainerSafetyLimits safety;
    std::vector<std::string> ancestry;
    std::vector<std::string> dependencies;
};

[[nodiscard]] LooseContainerMaterialization failure(
    LooseContainerStatus status,
    std::string requested,
    std::string selected,
    std::string detail,
    std::vector<std::string> dependencies = {}) {
    return LooseContainerMaterialization{
        .status = status,
        .representation = std::nullopt,
        .requested_path = std::move(requested),
        .selected_path = std::move(selected),
        .bytes = {},
        .dependency_paths = std::move(dependencies),
        .detail = std::move(detail),
    };
}

[[nodiscard]] LooseContainerMaterialization synthesize_list(
    std::string_view requested_packed_path,
    std::string list_path,
    std::vector<std::byte> list_bytes,
    SynthesisContext& context,
    std::size_t depth);

[[nodiscard]] std::optional<ChildImage> resolve_child(
    const LooseListEntry& entry,
    std::string_view base,
    SynthesisContext& context,
    std::size_t depth,
    LooseContainerStatus& failure_status,
    std::string& failure_detail) {
    if (entry.dummy) {
        return ChildImage{.selected_path = {}, .bytes = {}, .dummy = true};
    }

    auto child_path = join_child_path(base, entry.token);
    if (!entry.nested_list) {
        auto bytes = context.read(child_path);
        if (!bytes.has_value() || bytes->empty()) {
            failure_status = LooseContainerStatus::child_unavailable;
            failure_detail = "An ordinary .lst child is missing or has zero size: " +
                child_path;
            return std::nullopt;
        }
        context.dependencies.push_back(child_path);
        return ChildImage{
            .selected_path = std::move(child_path),
            .bytes = std::move(*bytes),
            .dummy = false,
        };
    }

    const auto packed_sibling =
        LooseContainerListPolicy::packed_sibling_for_list(child_path);
    if (!packed_sibling.has_value()) {
        failure_status = LooseContainerStatus::invalid_path;
        failure_detail = "A nested .lst child has no replaceable extension: " +
            child_path;
        return std::nullopt;
    }

    if (auto packed = context.read(*packed_sibling);
        packed.has_value() && !packed->empty()) {
        context.dependencies.push_back(*packed_sibling);
        return ChildImage{
            .selected_path = *packed_sibling,
            .bytes = std::move(*packed),
            .dummy = false,
        };
    }

    auto nested_list = context.read(child_path);
    if (!nested_list.has_value() || nested_list->empty()) {
        failure_status = LooseContainerStatus::child_unavailable;
        failure_detail = "Neither a positive-size sibling .pac nor nested .lst is available: " +
            child_path;
        return std::nullopt;
    }

    auto nested = synthesize_list(
        child_path, child_path, std::move(*nested_list), context, depth + 1U);
    if (!nested.ok()) {
        failure_status = nested.status;
        failure_detail = nested.detail;
        return std::nullopt;
    }
    return ChildImage{
        .selected_path = child_path,
        .bytes = std::move(nested.bytes),
        .dummy = false,
    };
}

[[nodiscard]] LooseContainerMaterialization synthesize_list(
    std::string_view requested_packed_path,
    std::string list_path,
    std::vector<std::byte> list_bytes,
    SynthesisContext& context,
    std::size_t depth) {
    if (depth > context.safety.max_recursion_depth) {
        return failure(
            LooseContainerStatus::recursion_limit,
            std::string{requested_packed_path},
            list_path,
            "Product safety recursion limit reached while synthesizing nested .lst.",
            context.dependencies);
    }
    if (std::find(context.ancestry.begin(), context.ancestry.end(), list_path) !=
        context.ancestry.end()) {
        return failure(
            LooseContainerStatus::recursion_cycle,
            std::string{requested_packed_path},
            list_path,
            "Product safety ancestry guard rejected a recursive .lst cycle.",
            context.dependencies);
    }

    context.ancestry.push_back(list_path);
    context.dependencies.push_back(list_path);
    struct Pop final {
        std::vector<std::string>& values;
        ~Pop() { values.pop_back(); }
    } pop{context.ancestry};

    const auto document = LooseContainerListPolicy::parse(list_bytes);
    if (!document.ok()) {
        return failure(
            document.status,
            std::string{requested_packed_path},
            list_path,
            document.detail,
            context.dependencies);
    }

    const auto header = LooseContainerListPolicy::header_size(document.entries.size());
    if (!header.has_value() || *header > context.safety.max_output_bytes) {
        return failure(
            LooseContainerStatus::output_too_large,
            std::string{requested_packed_path},
            list_path,
            "Synthesized .lst header exceeds the product output budget.",
            context.dependencies);
    }

    std::vector<ChildImage> children;
    children.reserve(document.entries.size());
    const auto base = LooseContainerListPolicy::base_directory(list_path);
    std::size_t total = *header;

    for (const auto& entry : document.entries) {
        LooseContainerStatus child_failure = LooseContainerStatus::child_unavailable;
        std::string child_detail;
        auto child = resolve_child(
            entry, base, context, depth, child_failure, child_detail);
        if (!child.has_value()) {
            return failure(
                child_failure,
                std::string{requested_packed_path},
                list_path,
                std::move(child_detail),
                context.dependencies);
        }

        if (!child->dummy) {
            const auto aligned =
                LooseContainerListPolicy::aligned_size(child->bytes.size());
            if (aligned < child->bytes.size() ||
                total > context.safety.max_output_bytes -
                    std::min(aligned, context.safety.max_output_bytes)) {
                return failure(
                    LooseContainerStatus::output_too_large,
                    std::string{requested_packed_path},
                    list_path,
                    "Synthesized .lst child placement exceeds the product output budget.",
                    context.dependencies);
            }
            total += aligned;
            if (total > context.safety.max_output_bytes ||
                total > static_cast<std::size_t>(
                    std::numeric_limits<std::uint32_t>::max())) {
                return failure(
                    LooseContainerStatus::output_too_large,
                    std::string{requested_packed_path},
                    list_path,
                    "Synthesized .lst image cannot be represented by 32-bit offsets.",
                    context.dependencies);
            }
        }
        children.push_back(std::move(*child));
    }

    std::vector<std::byte> output(total, std::byte{0});
    std::copy(document.magic.begin(), document.magic.end(), output.begin());
    write_u32_le(
        output, 4U, static_cast<std::uint32_t>(document.entries.size()));

    std::size_t cursor = *header;
    for (std::size_t index = 0U; index < children.size(); ++index) {
        const auto table_offset = 8U + index * 4U;
        const auto& child = children[index];
        if (child.dummy) {
            write_u32_le(output, table_offset, 0U);
            continue;
        }

        write_u32_le(output, table_offset, static_cast<std::uint32_t>(cursor));
        std::copy(
            child.bytes.begin(), child.bytes.end(),
            output.begin() + static_cast<std::ptrdiff_t>(cursor));
        cursor += LooseContainerListPolicy::aligned_size(child.bytes.size());
    }

    return LooseContainerMaterialization{
        .status = LooseContainerStatus::ok,
        .representation = LooseContainerRepresentation::loose_list,
        .requested_path = std::string{requested_packed_path},
        .selected_path = std::move(list_path),
        .bytes = std::move(output),
        .dependency_paths = context.dependencies,
        .detail = {},
    };
}

} // namespace

LooseListDocument LooseContainerListPolicy::parse(
    std::span<const std::byte> bytes) {
    LooseListDocument result;
    const auto limit = std::min(bytes.size(), scan_limit);
    std::size_t cursor = 0U;
    bool terminated = false;

    while (cursor < limit) {
        const auto current = u8(bytes[cursor]);
        if (current == 0U) {
            terminated = true;
            break;
        }
        if (current == 0x0AU) {
            ++cursor;
            continue;
        }
        if (current == 0x0DU) {
            while (cursor < limit && u8(bytes[cursor]) != 0x0AU &&
                   u8(bytes[cursor]) != 0U) {
                ++cursor;
            }
            if (cursor < limit && u8(bytes[cursor]) == 0x0AU) {
                ++cursor;
            }
            continue;
        }

        const auto line_offset = cursor;
        if (current == static_cast<unsigned char>('/')) {
            while (cursor < limit && u8(bytes[cursor]) != 0x0AU &&
                   u8(bytes[cursor]) != 0U) {
                ++cursor;
            }
            if (cursor < limit && u8(bytes[cursor]) == 0x0AU) {
                ++cursor;
            }
            continue;
        }

        if (current == static_cast<unsigned char>('#')) {
            ++cursor;
            std::array<std::byte, 4> magic{};
            std::size_t captured = 0U;
            while (cursor < limit && u8(bytes[cursor]) != 0x0AU &&
                   u8(bytes[cursor]) != 0U) {
                if (captured < magic.size()) {
                    magic[captured++] = bytes[cursor];
                }
                ++cursor;
            }
            result.magic = magic;
            result.magic_from_directive = true;
            if (cursor < limit && u8(bytes[cursor]) == 0x0AU) {
                ++cursor;
            }
            continue;
        }

        std::string token;
        token.reserve(32U);
        while (cursor < limit) {
            const auto value = u8(bytes[cursor]);
            if (value == 0U || value == 0x0DU) {
                break;
            }
            if (value == 0x0AU) {
                result.status = LooseContainerStatus::lf_only_normal_line;
                result.scanned_bytes = cursor;
                result.detail =
                    "A normal .lst child line reached LF without the recovered CR terminator.";
                return result;
            }
            if (token.size() >= token_limit) {
                result.status = LooseContainerStatus::token_limit_exceeded;
                result.scanned_bytes = cursor;
                result.detail = "A .lst child token exceeds the recovered 0x100-byte bound.";
                return result;
            }
            token.push_back(static_cast<char>(value));
            ++cursor;
        }

        if (!token.empty()) {
            result.entries.push_back(LooseListEntry{
                .token = token,
                .source_offset = line_offset,
                .dummy = token == "dummy",
                .nested_list = is_nested_list_token(token),
            });
        }

        if (cursor < limit && u8(bytes[cursor]) == 0U) {
            terminated = true;
            break;
        }
        while (cursor < limit && u8(bytes[cursor]) != 0x0AU &&
               u8(bytes[cursor]) != 0U) {
            ++cursor;
        }
        if (cursor < limit && u8(bytes[cursor]) == 0x0AU) {
            ++cursor;
        }
    }

    result.scanned_bytes = cursor;
    if (!terminated && bytes.size() > scan_limit) {
        result.status = LooseContainerStatus::scan_limit_exceeded;
        result.detail = "The .lst stream exceeds the recovered 0x1FC0 scan ceiling.";
    }
    return result;
}

std::optional<std::string> LooseContainerListPolicy::list_path_for(
    std::string_view packed_path) {
    const auto slash = packed_path.find_last_of("\\/");
    const auto dot = packed_path.find_last_of('.');
    if (dot == std::string_view::npos ||
        (slash != std::string_view::npos && dot < slash)) {
        return std::nullopt;
    }

    std::string result{packed_path.substr(0U, dot)};
    result += ".lst";
    return result;
}

std::string LooseContainerListPolicy::base_directory(std::string_view list_path) {
    const auto slash = list_path.find_last_of('\\');
    if (slash == std::string_view::npos) {
        return {};
    }
    return std::string{list_path.substr(0U, slash + 1U)};
}

std::optional<std::string> LooseContainerListPolicy::packed_sibling_for_list(
    std::string_view list_path) {
    if (!list_path.ends_with(".lst")) {
        return std::nullopt;
    }
    std::string result{list_path.substr(0U, list_path.size() - 4U)};
    result += ".pac";
    return result;
}

std::size_t LooseContainerListPolicy::aligned_size(std::size_t value) noexcept {
    constexpr auto mask = synthesis_alignment - 1U;
    if (value > std::numeric_limits<std::size_t>::max() - mask) {
        return std::numeric_limits<std::size_t>::max();
    }
    return (value + mask) & ~mask;
}

std::optional<std::size_t> LooseContainerListPolicy::header_size(
    std::size_t slot_count) noexcept {
    if (slot_count >
        (std::numeric_limits<std::size_t>::max() / 4U) - 2U) {
        return std::nullopt;
    }
    const auto raw = (slot_count + 2U) * 4U;
    const auto aligned = aligned_size(raw);
    if (aligned == std::numeric_limits<std::size_t>::max()) {
        return std::nullopt;
    }
    return aligned;
}

LooseContainerMaterialization LooseContainerListPolicy::materialize(
    std::string_view packed_path,
    const LooseContainerRead& read,
    LooseContainerSafetyLimits safety) {
    if (packed_path.empty() || !read || safety.max_recursion_depth == 0U ||
        safety.max_output_bytes == 0U) {
        return failure(
            LooseContainerStatus::invalid_path,
            std::string{packed_path}, {},
            "Invalid loose-container request or product safety configuration.");
    }

    if (auto packed = read(packed_path); packed.has_value() && !packed->empty()) {
        if (packed->size() > safety.max_output_bytes) {
            return failure(
                LooseContainerStatus::output_too_large,
                std::string{packed_path}, std::string{packed_path},
                "Packed container exceeds the product output budget.");
        }
        return LooseContainerMaterialization{
            .status = LooseContainerStatus::ok,
            .representation = LooseContainerRepresentation::packed,
            .requested_path = std::string{packed_path},
            .selected_path = std::string{packed_path},
            .bytes = std::move(*packed),
            .dependency_paths = {std::string{packed_path}},
            .detail = {},
        };
    }

    const auto list_path = list_path_for(packed_path);
    if (!list_path.has_value()) {
        return failure(
            LooseContainerStatus::invalid_path,
            std::string{packed_path}, {},
            "The recovered .lst fallback requires an existing extension boundary.");
    }

    auto list_bytes = read(*list_path);
    if (!list_bytes.has_value() || list_bytes->empty()) {
        return failure(
            LooseContainerStatus::not_found,
            std::string{packed_path}, *list_path,
            "Neither a positive-size packed container nor its .lst fallback is available.");
    }

    SynthesisContext context{
        .read = read,
        .safety = safety,
        .ancestry = {},
        .dependencies = {},
    };
    return synthesize_list(
        packed_path, *list_path, std::move(*list_bytes), context, 1U);
}

} // namespace dmc::rengine::profiles::dmc3
