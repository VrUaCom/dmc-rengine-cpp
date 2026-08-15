#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_stage_cfg_collision_tables {

inline constexpr std::size_t k_entry_stride = 0x04U;
inline constexpr std::size_t k_primitive_descriptor_stride = 0x50U;
inline constexpr std::size_t k_primitive_type_offset = 0x00U;

enum class SlotGeneration : std::uint8_t {
    modern_cem_stage_cfg,
    legacy_cem008_stage_cfg,
};

struct SlotPair final {
    std::size_t entry_table_slot{};
    std::size_t primitive_descriptor_table_slot{};
};

struct Entry final {
    std::uint8_t flags{};
    std::uint8_t transform_selector{};
    std::uint16_t descriptor_index{};
};

struct DescriptorReferenceValidation final {
    bool descriptor_index_in_range{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return descriptor_index_in_range;
    }
};

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

[[nodiscard]] constexpr SlotPair slots_for_generation(SlotGeneration generation) noexcept {
    switch (generation) {
    case SlotGeneration::modern_cem_stage_cfg:
        return {39U, 40U};
    case SlotGeneration::legacy_cem008_stage_cfg:
        return {22U, 23U};
    }
    return {};
}

class View final {
public:
    [[nodiscard]] static std::optional<View> open(
        std::string_view executable_sha256,
        std::span<const std::byte> container_bytes,
        const formats::ContainerDocument& container,
        SlotGeneration generation) noexcept {
        if (!matches_canonical_target(executable_sha256) || !container.valid() ||
            container.container_size > container_bytes.size()) {
            return std::nullopt;
        }

        const auto slots = slots_for_generation(generation);
        const auto* entry_slot = find_slot(container, slots.entry_table_slot);
        const auto* descriptor_slot =
            find_slot(container, slots.primitive_descriptor_table_slot);
        if (entry_slot == nullptr || descriptor_slot == nullptr ||
            !entry_slot->valid || !descriptor_slot->valid ||
            (entry_slot->size % k_entry_stride) != 0U ||
            (descriptor_slot->size % k_primitive_descriptor_stride) != 0U ||
            !range_fits(container_bytes, *entry_slot) ||
            !range_fits(container_bytes, *descriptor_slot)) {
            return std::nullopt;
        }

        return View(
            generation,
            slots,
            container_bytes.subspan(entry_slot->offset, entry_slot->size),
            container_bytes.subspan(descriptor_slot->offset, descriptor_slot->size));
    }

    [[nodiscard]] constexpr SlotGeneration generation() const noexcept {
        return generation_;
    }

    [[nodiscard]] constexpr SlotPair slots() const noexcept {
        return slots_;
    }

    [[nodiscard]] constexpr std::size_t entry_count() const noexcept {
        return entry_table_.size() / k_entry_stride;
    }

    [[nodiscard]] constexpr std::size_t primitive_descriptor_count() const noexcept {
        return primitive_descriptor_table_.size() / k_primitive_descriptor_stride;
    }

    [[nodiscard]] std::optional<Entry> entry(std::size_t index) const noexcept {
        if (index >= entry_count()) {
            return std::nullopt;
        }

        const auto offset = index * k_entry_stride;
        const auto low = std::to_integer<std::uint16_t>(entry_table_[offset + 2U]);
        const auto high = std::to_integer<std::uint16_t>(entry_table_[offset + 3U]);
        return Entry{
            .flags = std::to_integer<std::uint8_t>(entry_table_[offset]),
            .transform_selector = std::to_integer<std::uint8_t>(entry_table_[offset + 1U]),
            .descriptor_index = static_cast<std::uint16_t>(low | (high << 8U)),
        };
    }

    [[nodiscard]] std::optional<std::span<const std::byte>>
    primitive_descriptor(std::size_t index) const noexcept {
        if (index >= primitive_descriptor_count()) {
            return std::nullopt;
        }
        return primitive_descriptor_table_.subspan(
            index * k_primitive_descriptor_stride,
            k_primitive_descriptor_stride);
    }

    [[nodiscard]] std::optional<std::uint8_t>
    primitive_type(std::size_t descriptor_index) const noexcept {
        const auto descriptor = primitive_descriptor(descriptor_index);
        if (!descriptor) {
            return std::nullopt;
        }
        return std::to_integer<std::uint8_t>((*descriptor)[k_primitive_type_offset]);
    }

    [[nodiscard]] std::optional<DescriptorReferenceValidation>
    validate_descriptor_reference(std::size_t entry_index) const noexcept {
        const auto decoded = entry(entry_index);
        if (!decoded) {
            return std::nullopt;
        }
        return DescriptorReferenceValidation{
            .descriptor_index_in_range =
                static_cast<std::size_t>(decoded->descriptor_index) < primitive_descriptor_count(),
        };
    }

    [[nodiscard]] bool all_descriptor_references_valid() const noexcept {
        for (std::size_t index = 0; index < entry_count(); ++index) {
            const auto validation = validate_descriptor_reference(index);
            if (!validation || !validation->valid()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr bool transform_selector_bounds_available() const noexcept {
        return false;
    }

private:
    [[nodiscard]] static const formats::ContainerEntry*
    find_slot(const formats::ContainerDocument& container, std::size_t slot_index) noexcept {
        for (const auto& entry : container.entries) {
            if (entry.slot_index == slot_index) {
                return &entry;
            }
        }
        return nullptr;
    }

    [[nodiscard]] static bool range_fits(
        std::span<const std::byte> bytes,
        const formats::ContainerEntry& entry) noexcept {
        return entry.offset <= bytes.size() && entry.size <= bytes.size() - entry.offset;
    }

    constexpr View(
        SlotGeneration generation,
        SlotPair slots,
        std::span<const std::byte> entry_table,
        std::span<const std::byte> primitive_descriptor_table) noexcept
        : generation_(generation),
          slots_(slots),
          entry_table_(entry_table),
          primitive_descriptor_table_(primitive_descriptor_table) {}

    SlotGeneration generation_{};
    SlotPair slots_{};
    std::span<const std::byte> entry_table_;
    std::span<const std::byte> primitive_descriptor_table_;
};

} // namespace dmc::rengine::profiles::dmc3::hits_stage_cfg_collision_tables
