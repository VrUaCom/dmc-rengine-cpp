#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/contact.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::hits::runtime {

// Rengine-native result contract for editor preview and deterministic tests.
// This is not claimed to be the original game's CollisionResult ABI.
struct ContactResult final {
    StaticSource source{StaticSource::source_0_member_3};
    std::uint32_t triangle_index{};
    std::uint32_t triangle_byte_offset{};
    std::uint32_t flags{};
    formats::hits::Vec3 point;
    formats::hits::Vec3 normal;
    float metric{};

    [[nodiscard]] bool finite() const noexcept {
        return std::isfinite(point.x) && std::isfinite(point.y) &&
               std::isfinite(point.z) && std::isfinite(normal.x) &&
               std::isfinite(normal.y) && std::isfinite(normal.z) &&
               std::isfinite(metric);
    }
};

[[nodiscard]] inline std::optional<ContactResult> make_contact_result(
    const TriangleCandidate& candidate,
    const formats::hits::Vec3& point,
    float metric) noexcept {
    if (candidate.triangle == nullptr || !std::isfinite(metric) || metric < 0.0F) {
        return std::nullopt;
    }

    ContactResult result{
        .source = candidate.source,
        .triangle_index = candidate.triangle_index,
        .triangle_byte_offset = candidate.triangle_byte_offset,
        .flags = candidate.flags,
        .point = point,
        .normal = candidate.triangle->normal,
        .metric = metric,
    };
    if (!result.finite()) {
        return std::nullopt;
    }
    return result;
}

[[nodiscard]] inline std::optional<ContactResult> select_nearest_contact(
    std::span<const ContactResult> contacts) noexcept {
    std::optional<ContactResult> nearest;
    for (const auto& contact : contacts) {
        if (!contact.finite() || contact.metric < 0.0F) {
            continue;
        }
        if (!nearest || contact.metric < nearest->metric ||
            (contact.metric == nearest->metric &&
             (static_cast<std::uint32_t>(contact.source) <
                  static_cast<std::uint32_t>(nearest->source) ||
              (contact.source == nearest->source &&
               contact.triangle_byte_offset < nearest->triangle_byte_offset)))) {
            nearest = contact;
        }
    }
    return nearest;
}

[[nodiscard]] inline std::vector<ContactResult> sort_contacts_deterministically(
    std::span<const ContactResult> contacts) {
    std::vector<ContactResult> sorted;
    sorted.reserve(contacts.size());
    for (const auto& contact : contacts) {
        if (contact.finite() && contact.metric >= 0.0F) {
            sorted.push_back(contact);
        }
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto& left, const auto& right) {
        if (left.metric != right.metric) {
            return left.metric < right.metric;
        }
        if (left.source != right.source) {
            return static_cast<std::uint32_t>(left.source) <
                   static_cast<std::uint32_t>(right.source);
        }
        return left.triangle_byte_offset < right.triangle_byte_offset;
    });
    return sorted;
}

} // namespace dmc::rengine::hits::runtime
