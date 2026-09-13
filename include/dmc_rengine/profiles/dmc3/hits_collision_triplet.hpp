#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::profiles::dmc3::hits_collision_triplet {

inline constexpr std::size_t k_transform_stride = 0x40U;
inline constexpr std::size_t k_entry_stride = 0x04U;
inline constexpr std::size_t k_primitive_descriptor_stride = 0x50U;
inline constexpr std::size_t k_primitive_type_offset = 0x00U;

struct Entry final {
    std::uint8_t flags{};
    std::uint8_t transform_selector{};
    std::uint16_t descriptor_index{};
};

struct EntryReferenceValidation final {
    bool transform_selector_in_range{};
    bool descriptor_index_in_range{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return transform_selector_in_range && descriptor_index_in_range;
    }
};

class View final {
public:
    [[nodiscard]] static std::optional<View> open(
        std::span<const std::byte> transform_table,
        std::span<const std::byte> entry_table,
        std::span<const std::byte> primitive_descriptor_table) noexcept {
        if ((transform_table.size() % k_transform_stride) != 0U ||
            (entry_table.size() % k_entry_stride) != 0U ||
            (primitive_descriptor_table.size() % k_primitive_descriptor_stride) != 0U) {
            return std::nullopt;
        }

        return View(transform_table, entry_table, primitive_descriptor_table);
    }

    [[nodiscard]] constexpr std::size_t transform_count() const noexcept {
        return transform_table_.size() / k_transform_stride;
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
    transform_record(std::size_t index) const noexcept {
        if (index >= transform_count()) {
            return std::nullopt;
        }
        return transform_table_.subspan(index * k_transform_stride, k_transform_stride);
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

    [[nodiscard]] std::optional<EntryReferenceValidation>
    validate_entry(std::size_t index) const noexcept {
        const auto decoded = entry(index);
        if (!decoded) {
            return std::nullopt;
        }

        return EntryReferenceValidation{
            .transform_selector_in_range =
                static_cast<std::size_t>(decoded->transform_selector) < transform_count(),
            .descriptor_index_in_range =
                static_cast<std::size_t>(decoded->descriptor_index) < primitive_descriptor_count(),
        };
    }

    [[nodiscard]] bool all_entry_references_valid() const noexcept {
        for (std::size_t index = 0; index < entry_count(); ++index) {
            const auto validation = validate_entry(index);
            if (!validation || !validation->valid()) {
                return false;
            }
        }
        return true;
    }

private:
    constexpr View(
        std::span<const std::byte> transform_table,
        std::span<const std::byte> entry_table,
        std::span<const std::byte> primitive_descriptor_table) noexcept
        : transform_table_(transform_table),
          entry_table_(entry_table),
          primitive_descriptor_table_(primitive_descriptor_table) {}

    std::span<const std::byte> transform_table_;
    std::span<const std::byte> entry_table_;
    std::span<const std::byte> primitive_descriptor_table_;
};

} // namespace dmc::rengine::profiles::dmc3::hits_collision_triplet
