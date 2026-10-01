#include "dmc_rengine/profiles/dmc3/attachment_tables.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <memory>
#include <new>
#include <optional>
#include <vector>

#include "dmc_rengine/analysis/mod/animation_binding.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

namespace dmc::rengine::profiles::dmc3::motion {
namespace {

namespace world = dmc::rengine::formats::mod::world_transform;

}  // namespace
std::vector<CompositeNodeConstraint> parse_coat_constraints(std::span<const std::uint8_t> bytes) {
    std::vector<CompositeNodeConstraint> out;
    const auto u32 = [&bytes](std::size_t o) {
        return static_cast<std::uint32_t>(bytes[o]) |
               (static_cast<std::uint32_t>(bytes[o + 1U]) << 8U) |
               (static_cast<std::uint32_t>(bytes[o + 2U]) << 16U) |
               (static_cast<std::uint32_t>(bytes[o + 3U]) << 24U);
    };
    if (bytes.size() < 0x10U || bytes[0] != 'C' || bytes[1] != 'C' || bytes[2] != 'N' ||
        bytes[3] != 'S' || u32(4U) != 1U || u32(8U) > 16U) {
        return out;
    }
    const std::size_t count = u32(8U);
    for (std::size_t i = 0U; i < count; ++i) {
        const std::size_t o = 0x10U + i * 0x50U;
        if (o + 0x50U > bytes.size()) break;
        const auto node = u32(o);
        const auto joint = u32(o + 4U);
        if (node >= kPlayerCoatJointCapacity || joint >= 96U) continue;
        CompositeNodeConstraint c;
        c.child_node = node;
        c.host_node = joint;
        for (std::size_t k = 0U; k < 16U; ++k) {
            c.offset.values[k] = std::bit_cast<float>(u32(o + 0x10U + k * 4U));
        }
        out.push_back(c);
    }
    return out;
}

std::array<ClothCapsule, 6> player_coat_capsules(std::span<const std::uint8_t> bytes) {
    std::array<ClothCapsule, 6> out{};
    std::copy(kPlayerCoatCapsules.begin(), kPlayerCoatCapsules.end(), out.begin());
    const auto u32 = [&bytes](std::size_t o) {
        return static_cast<std::uint32_t>(bytes[o]) |
               (static_cast<std::uint32_t>(bytes[o + 1U]) << 8U) |
               (static_cast<std::uint32_t>(bytes[o + 2U]) << 16U) |
               (static_cast<std::uint32_t>(bytes[o + 3U]) << 24U);
    };
    const auto f32 = [&](std::size_t o) { return std::bit_cast<float>(u32(o)); };
    if (bytes.size() < 0x10U || bytes[0] != 'C' || bytes[1] != 'C' || bytes[2] != 'N' ||
        bytes[3] != 'S' || u32(4U) != 1U || u32(8U) > 16U || u32(12U) > 6U) {
        return out;
    }
    std::size_t o = 0x10U + static_cast<std::size_t>(u32(8U)) * 0x50U;
    for (std::uint32_t i = 0U; i < u32(12U) && o + 0x40U <= bytes.size(); ++i, o += 0x40U) {
        const auto index = u32(o);
        if (index >= out.size()) continue;
        out[index].a = {f32(o + 0x10U), f32(o + 0x14U), f32(o + 0x18U)};
        out[index].b = {f32(o + 0x20U), f32(o + 0x24U), f32(o + 0x28U)};
        out[index].radius = f32(o + 0x30U);
    }
    return out;
}

std::optional<EnemyPartConstraints> enemy_constraints_for(std::string_view archive_name,
                                                          std::uint32_t part_slot) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    for (const auto& record : kEnemyPartConstraints) {
        if (record.part_slot != part_slot) continue;
        const auto& stem = record.pac_stem;
        if (archive_name.size() != stem.size() + 4U) continue;
        bool match = true;
        for (std::size_t i = 0U; i < archive_name.size() && match; ++i) {
            const char expected = i < stem.size() ? stem[i] : ".pac"[i - stem.size()];
            match = std::tolower(static_cast<unsigned char>(archive_name[i])) == expected;
        }
        if (match) return record;
    }
    return std::nullopt;
}

std::optional<std::uint32_t> tsc_slot_for(std::string_view archive_name,
                                          std::uint32_t model_slot) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    for (const auto& record : kTscSources) {
        bool used = false;
        for (std::uint32_t k = 0U; k < record.model_count; ++k) {
            used = used || record.model_slots[k] == model_slot;
        }
        if (!used) continue;
        const auto& stem = record.pac_stem;
        if (archive_name.size() != stem.size() + 4U) continue;
        bool match = true;
        for (std::size_t i = 0U; i < archive_name.size() && match; ++i) {
            const char expected = i < stem.size() ? stem[i] : ".pac"[i - stem.size()];
            match = std::tolower(static_cast<unsigned char>(archive_name[i])) == expected;
        }
        if (match) return record.tsc_slot;
    }
    return std::nullopt;
}

std::optional<std::uint32_t> enemy_cloth_slot(std::string_view archive_name,
                                              std::uint32_t model_slot) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    for (const auto& record : kEnemyClothSources) {
        if (record.model_slot != model_slot) continue;
        const auto& stem = record.pac_stem;
        if (archive_name.size() != stem.size() + 4U) continue;
        bool match = true;
        for (std::size_t i = 0U; i < archive_name.size() && match; ++i) {
            const char expected = i < stem.size() ? stem[i] : ".pac"[i - stem.size()];
            match = std::tolower(static_cast<unsigned char>(archive_name[i])) == expected;
        }
        if (match) return record.clt_slot;
    }
    return std::nullopt;
}

std::optional<WeaponAttachRecord> weapon_record_for_archive(
    std::string_view archive_name) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    for (const auto& record : kWeaponState0Records) {
        const auto& stem = record.pac_stem;
        if (archive_name.size() != stem.size() + 4U) continue;
        bool match = true;
        for (std::size_t i = 0U; i < archive_name.size() && match; ++i) {
            const char expected = i < stem.size() ? stem[i] : ".pac"[i - stem.size()];
            match = std::tolower(static_cast<unsigned char>(archive_name[i])) == expected;
        }
        if (match) return record;
    }
    return std::nullopt;
}

Matrix4 attach_local_matrix(const std::array<float, 3>& translation,
                            const std::array<float, 3>& rotation_xyz_radians) noexcept {
    dmc::rengine::formats::mod::transform_domain::LocalTransformRecord local{};
    local.translation = {translation[0], translation[1], translation[2]};
    local.rotation_xyz_radians = {rotation_xyz_radians[0], rotation_xyz_radians[1],
                                  rotation_xyz_radians[2]};
    const auto built = world::build_local_matrix(local);
    Matrix4 out;
    out.values = built.values;
    out.values[12] = translation[0];
    out.values[13] = translation[1];
    out.values[14] = translation[2];
    out.values[15] = 1.0F;
    return out;
}

const WeaponStateRecord* weapon_state_record(std::string_view class_name,
                                             std::uint8_t state) noexcept {
    if (state >= 24U) return nullptr;
    for (const auto& table : kWeaponStateTables) {
        if (table.class_name != class_name) continue;
        const auto& record = table.states[state];
        if (record.branch > 1U) return nullptr;  // 255 empty, >= 2 special poses
        return &record;
    }
    return nullptr;
}

Matrix4 weapon_offset_matrix(const WeaponAttachRecord& record) noexcept {
    return attach_local_matrix(record.translation, record.rotation_xyz_radians);
}

std::span<const EnemyVariant> enemy_variants_for(std::string_view archive_name) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    constexpr std::string_view name = "em000.pac";
    if (archive_name.size() != name.size()) return {};
    for (std::size_t i = 0U; i < name.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(archive_name[i])) != name[i]) return {};
    }
    return kEm000Variants;
}

std::vector<ArchiveVariant> archive_variants(std::string_view archive_name) {
    std::vector<ArchiveVariant> out;
    for (const auto& enemy : enemy_variants_for(archive_name)) {
        ArchiveVariant first;
        first.enemy = &enemy;
        first.label = std::string{enemy.class_name};
        if (enemy.weapon_slot_alt == enemy.weapon_slot) {
            out.push_back(std::move(first));
            continue;
        }
        first.label += " A";
        out.push_back(first);
        ArchiveVariant second = first;
        second.alternate_weapon = true;
        second.label = std::string{enemy.class_name} + " B";
        out.push_back(std::move(second));
    }
    if (!out.empty()) return out;
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);

    constexpr std::string_view nevan = "em028.pac";
    bool is_nevan = archive_name.size() == nevan.size();
    for (std::size_t i = 0U; is_nevan && i < nevan.size(); ++i) {
        is_nevan = std::tolower(static_cast<unsigned char>(archive_name[i])) == nevan[i];
    }
    if (is_nevan) {
        ArchiveVariant calm;
        calm.label = "Bats in";
        calm.hide_slot = 5U;
        calm.hide_objects = {2U, 3U};
        calm.hide_count = 2U;
        out.push_back(calm);
        ArchiveVariant swarm;
        swarm.label = "Bats out";
        out.push_back(swarm);
        return out;
    }

    // em034.pac (Lady) contains two complete appearance sets plus a separate
    // group of equipment/weapon MODs. Corpus evidence from the retail PAC:
    //   slots 1 + 17  -> MOD header +0x14 = 0x00000384
    //   slots 32 + 34 -> MOD header +0x14 = 0x0000063D
    //   slots 20..26 + 30 -> +0x14 = 0x000AF258 (equipment group)
    // Both appearance pairs also share the same transform/texture-domain
    // shape (23+9 nodes, two texture slots). Keeping all twelve MODs in one
    // composite is therefore structurally wrong and visibly overlays both
    // costumes while dropping equipment at source origin.
    constexpr std::string_view lady = "em034.pac";
    bool is_lady = archive_name.size() == lady.size();
    for (std::size_t i = 0U; is_lady && i < lady.size(); ++i) {
        is_lady = std::tolower(static_cast<unsigned char>(archive_name[i])) == lady[i];
    }
    if (is_lady) {
        // CEm034 runtime ownership is now EXE-confirmed:
        //   persistent components: slots20..24, owned for the actor lifetime;
        //   dynamic CShell resources: slot25 (Shl02), slots26+30 (Shl03).
        // Dynamic resources remain PAC children and are not overlaid in the
        // base Lady appearance.
        const auto bind_core = [](ArchiveVariant& variant,
                                  std::uint32_t body_slot,
                                  std::uint32_t hair_slot) {
            variant.part_attachments[0] =
                {body_slot, hair_slot, 5U, false, true};
            for (std::size_t i = 0U; i < kCEm034LadyComponents.size(); ++i) {
                const auto& component = kCEm034LadyComponents[i];
                const auto& stowed =
                    component.presets[static_cast<std::size_t>(
                        LadyPlacementPreset::BodyStowed)];
                variant.part_attachments[i + 1U] = {
                    body_slot,
                    component.model_slot,
                    stowed.serialized_node,
                    false,
                    true,
                    true,
                    stowed.translation,
                    stowed.rotation_xyz_radians,
                    true,
                };
            }
            variant.part_attachment_count =
                1U + static_cast<std::uint32_t>(kCEm034LadyComponents.size());
        };

        ArchiveVariant first;
        first.label = "Lady · costume 1";
        first.include_top_level_mod_slots =
            {1U, 17U, 20U, 21U, 22U, 23U, 24U};
        first.include_top_level_mod_count = 7U;
        first.texture_overrides = {{
            {1U, 0U}, {17U, 0U},
            {20U, 19U}, {21U, 19U}, {22U, 19U}, {23U, 19U},
            {24U, 19U},
        }};
        first.texture_override_count = 7U;
        bind_core(first, 1U, 17U);
        out.push_back(first);

        ArchiveVariant second;
        second.label = "Lady · costume 2";
        second.include_top_level_mod_slots =
            {32U, 34U, 20U, 21U, 22U, 23U, 24U};
        second.include_top_level_mod_count = 7U;
        second.texture_overrides = {{
            {32U, 31U}, {34U, 31U},
            {20U, 33U}, {21U, 33U}, {22U, 33U}, {23U, 33U},
            {24U, 33U},
        }};
        second.texture_override_count = 7U;
        bind_core(second, 32U, 34U);
        out.push_back(second);
    }
    return out;
}

std::uint32_t player_coat_host_joint(std::span<const std::uint8_t> coat_mod,
                                    std::size_t body_joint_count) noexcept {
    if (coat_mod.size() < 0x40U || coat_mod[0] != 'M' || coat_mod[1] != 'O' ||
        coat_mod[2] != 'D') {
        return kPlayerCoatHostJoint;
    }
    const std::uint32_t joint = kPlayerCoatHostJoint + coat_mod[0x13];
    return joint < body_joint_count ? joint : kPlayerCoatHostJoint;
}

Matrix4 attach_local_matrix_zyx(const std::array<float, 3>& translation,
                                const std::array<float, 3>& rotation_xyz_radians) noexcept {
    // Row-vector rotations as 0x140030F10 / 0x140030FC0 / 0x140031080 build them.
    const float cx = std::cos(rotation_xyz_radians[0]), sx = std::sin(rotation_xyz_radians[0]);
    const float cy = std::cos(rotation_xyz_radians[1]), sy = std::sin(rotation_xyz_radians[1]);
    const float cz = std::cos(rotation_xyz_radians[2]), sz = std::sin(rotation_xyz_radians[2]);
    const std::array<float, 9> rx{1.0F, 0.0F, 0.0F, 0.0F, cx, sx, 0.0F, -sx, cx};
    const std::array<float, 9> ry{cy, 0.0F, -sy, 0.0F, 1.0F, 0.0F, sy, 0.0F, cy};
    const std::array<float, 9> rz{cz, sz, 0.0F, -sz, cz, 0.0F, 0.0F, 0.0F, 1.0F};
    const auto mul = [](const std::array<float, 9>& a, const std::array<float, 9>& b) {
        std::array<float, 9> out{};
        for (std::size_t r = 0U; r < 3U; ++r) {
            for (std::size_t c = 0U; c < 3U; ++c) {
                for (std::size_t k = 0U; k < 3U; ++k) out[r * 3U + c] += a[r * 3U + k] * b[k * 3U + c];
            }
        }
        return out;
    };
    const auto m = mul(mul(rz, ry), rx);
    Matrix4 out;
    for (std::size_t r = 0U; r < 3U; ++r) {
        for (std::size_t c = 0U; c < 3U; ++c) out.values[r * 4U + c] = m[r * 3U + c];
    }
    out.values[12] = translation[0];
    out.values[13] = translation[1];
    out.values[14] = translation[2];
    return out;
}

std::optional<WeaponMotionBank> weapon_motion_bank(std::string_view archive_name) noexcept {
    const auto slash = archive_name.find_last_of("/\\");
    if (slash != std::string_view::npos) archive_name.remove_prefix(slash + 1U);
    constexpr std::string_view prefix = "pl000_00_";
    constexpr std::string_view suffix = ".pac";
    if (archive_name.size() <= prefix.size() + suffix.size()) return std::nullopt;
    const auto lower = [](char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    for (std::size_t i = 0U; i < prefix.size(); ++i) {
        if (lower(archive_name[i]) != prefix[i]) return std::nullopt;
    }
    const auto tail = archive_name.substr(archive_name.size() - suffix.size());
    for (std::size_t i = 0U; i < suffix.size(); ++i) {
        if (lower(tail[i]) != suffix[i]) return std::nullopt;
    }
    const auto digits = archive_name.substr(prefix.size(),
                                            archive_name.size() - prefix.size() - suffix.size());
    if (digits.empty() || digits.size() > 2U) return std::nullopt;
    unsigned value = 0U;
    for (const char c : digits) {
        if (c < '0' || c > '9') return std::nullopt;
        value = value * 10U + static_cast<unsigned>(c - '0');
    }
    for (const auto& bank : kDanteWeaponMotionBanks) {
        if (bank.file_index == value) return bank;
    }
    return std::nullopt;
}

std::optional<WeaponSecondPart> weapon_second_part(std::string_view class_name) noexcept {
    for (const auto& part : kWeaponSecondParts) {
        if (part.class_name == class_name) return part;
    }
    return std::nullopt;
}

}  // namespace dmc::rengine::profiles::dmc3::motion
