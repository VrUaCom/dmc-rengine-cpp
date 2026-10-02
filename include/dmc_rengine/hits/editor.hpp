#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::hits::editor {

using StableSurfaceId = writer::StableSurfaceId;
using Surface = writer::Surface;

class Session final {
public:
    [[nodiscard]] static std::optional<Session> open(
        std::span<const std::byte> source_bytes);

    [[nodiscard]] const formats::hits::ScanResult& source_scan() const noexcept;
    [[nodiscard]] std::span<const std::byte> source_bytes() const noexcept;
    [[nodiscard]] std::span<const Surface> surfaces() const noexcept;

    [[nodiscard]] bool dirty() const noexcept;
    [[nodiscard]] bool can_undo() const noexcept;
    [[nodiscard]] bool can_redo() const noexcept;
    [[nodiscard]] std::uint64_t revision() const noexcept;

    [[nodiscard]] std::optional<std::size_t> index_of(
        StableSurfaceId stable_id) const noexcept;

    [[nodiscard]] std::optional<StableSurfaceId> add_surface(
        std::uint32_t flags,
        const formats::hits::Vec3& point_a,
        const formats::hits::Vec3& point_b,
        const formats::hits::Vec3& point_c);

    [[nodiscard]] std::optional<StableSurfaceId> duplicate_surface(
        StableSurfaceId stable_id);

    [[nodiscard]] bool erase_surface(StableSurfaceId stable_id);

    [[nodiscard]] bool set_flags(
        StableSurfaceId stable_id,
        std::uint32_t flags);

    [[nodiscard]] bool set_geometry(
        StableSurfaceId stable_id,
        const formats::hits::Vec3& point_a,
        const formats::hits::Vec3& point_b,
        const formats::hits::Vec3& point_c);

    [[nodiscard]] bool translate_surface(
        StableSurfaceId stable_id,
        const formats::hits::Vec3& delta);

    [[nodiscard]] bool translate_surfaces(
        std::span<const StableSurfaceId> stable_ids,
        const formats::hits::Vec3& delta);

    [[nodiscard]] bool undo();
    [[nodiscard]] bool redo();
    [[nodiscard]] bool reset_to_source();

    [[nodiscard]] writer::RebuildResult rebuild() const;
    [[nodiscard]] writer::RebuildResult rebuild(
        writer::RebuildOptions options) const;

private:
    struct Snapshot final {
        std::vector<Surface> surfaces;
        StableSurfaceId next_stable_id{};
        std::uint64_t revision{};
    };

    Session(
        std::vector<std::byte> source_bytes,
        formats::hits::ScanResult source_scan,
        std::vector<Surface> surfaces,
        StableSurfaceId next_stable_id);

    [[nodiscard]] Snapshot snapshot() const;
    void restore(Snapshot state);
    void begin_mutation();

    [[nodiscard]] std::optional<StableSurfaceId> allocate_stable_id() noexcept;

    std::vector<std::byte> source_bytes_;
    formats::hits::ScanResult source_scan_;
    std::vector<Surface> source_surfaces_;
    StableSurfaceId source_next_stable_id_{};

    std::vector<Surface> surfaces_;
    StableSurfaceId next_stable_id_{};

    std::vector<Snapshot> undo_;
    std::vector<Snapshot> redo_;

    std::uint64_t revision_{};
    std::uint64_t next_revision_{1U};
};

} // namespace dmc::rengine::hits::editor
