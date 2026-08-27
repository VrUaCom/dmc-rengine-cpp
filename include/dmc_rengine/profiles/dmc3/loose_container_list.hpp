#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class LooseContainerRepresentation {
    packed,
    loose_list,
};

enum class LooseContainerStatus {
    ok,
    invalid_path,
    not_found,
    scan_limit_exceeded,
    token_limit_exceeded,
    lf_only_normal_line,
    child_unavailable,
    output_too_large,
    recursion_cycle,
    recursion_limit,
};

[[nodiscard]] constexpr std::string_view to_string(
    LooseContainerStatus status) noexcept {
    switch (status) {
    case LooseContainerStatus::ok: return "ok";
    case LooseContainerStatus::invalid_path: return "invalid-path";
    case LooseContainerStatus::not_found: return "not-found";
    case LooseContainerStatus::scan_limit_exceeded: return "scan-limit-exceeded";
    case LooseContainerStatus::token_limit_exceeded: return "token-limit-exceeded";
    case LooseContainerStatus::lf_only_normal_line: return "lf-only-normal-line";
    case LooseContainerStatus::child_unavailable: return "child-unavailable";
    case LooseContainerStatus::output_too_large: return "output-too-large";
    case LooseContainerStatus::recursion_cycle: return "recursion-cycle";
    case LooseContainerStatus::recursion_limit: return "recursion-limit";
    }
    return "invalid-path";
}

struct LooseListEntry final {
    std::string token;
    std::size_t source_offset{};
    bool dummy{false};
    bool nested_list{false};
};

struct LooseListDocument final {
    LooseContainerStatus status{LooseContainerStatus::ok};
    std::array<std::byte, 4> magic{
        static_cast<std::byte>('P'),
        static_cast<std::byte>('A'),
        static_cast<std::byte>('C'),
        std::byte{0}};
    bool magic_from_directive{false};
    std::vector<LooseListEntry> entries;
    std::size_t scanned_bytes{};
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == LooseContainerStatus::ok;
    }
};

struct LooseContainerSafetyLimits final {
    // Product hardening only. The original recursion depth/cycle policy is not
    // recovered by the preserved Pass-26 disassembly summary.
    std::size_t max_recursion_depth{64U};
    std::size_t max_output_bytes{0x40000000U};
};

using LooseContainerRead = std::function<
    std::optional<std::vector<std::byte>>(std::string_view path)>;

struct LooseContainerMaterialization final {
    LooseContainerStatus status{LooseContainerStatus::not_found};
    std::optional<LooseContainerRepresentation> representation;
    std::string requested_path;
    std::string selected_path;
    std::vector<std::byte> bytes;
    std::vector<std::string> dependency_paths;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == LooseContainerStatus::ok && representation.has_value();
    }
};

class LooseContainerListPolicy final {
public:
    static constexpr std::size_t scan_limit = 0x1FC0U;
    static constexpr std::size_t token_limit = 0x100U;
    static constexpr std::size_t synthesis_alignment = 0x40U;
    static constexpr std::size_t transfer_granularity = 0x800U;

    [[nodiscard]] static LooseListDocument parse(std::span<const std::byte> bytes);

    // Recovered 0x1401B9390 behavior: replace an existing extension with the
    // lowercase .lst suffix. A path without a dot cannot enter list fallback.
    [[nodiscard]] static std::optional<std::string> list_path_for(
        std::string_view packed_path);

    [[nodiscard]] static std::string base_directory(std::string_view list_path);
    [[nodiscard]] static std::optional<std::string> packed_sibling_for_list(
        std::string_view list_path);

    // 0x40 structural alignment is used by the synthesized header and by a
    // recursively synthesized child complete image. Direct whole-file children
    // use the recovered 0x800 transfer extent instead.
    [[nodiscard]] static std::size_t aligned_size(std::size_t value) noexcept;
    [[nodiscard]] static std::size_t direct_transfer_extent(
        std::size_t value) noexcept;
    [[nodiscard]] static std::optional<std::size_t> header_size(
        std::size_t slot_count) noexcept;

    // Recovered representation order: positive-size packed container first;
    // .lst synthesis only when the packed representation is unavailable.
    // read() is deliberately injected by the GDSpaces caller so this layer does
    // not create a second resolver/source authority.
    [[nodiscard]] static LooseContainerMaterialization materialize(
        std::string_view packed_path,
        const LooseContainerRead& read,
        LooseContainerSafetyLimits safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
