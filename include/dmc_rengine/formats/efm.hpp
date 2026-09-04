#pragma once

#include "dmc_rengine/formats/relocated_model_shell.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats {

enum class EfmParseError : std::uint8_t {
    none,
    shell_rejected,
    batch_out_of_bounds,
    array_out_of_bounds,
    array_packing_mismatch,
    invalid_document,
};

// One effect-model batch: the model batch plus one array whose element width
// the recovered routine does not reveal.
struct EfmPrimitiveBatch final {
    std::uint32_t group_index{};
    std::uint32_t batch_index{};
    std::uint64_t batch_offset{};
    std::uint32_t vertex_count{};

    std::uint64_t position_offset{};
    std::uint64_t normal_offset{};
    std::uint64_t attribute_offset{};
    std::uint64_t secondary_offset{};
    std::uint64_t control_offset{};
    // Bounded only by its base. The routine relocates it and never indexes
    // through it, so its extent is not recoverable and is not claimed.
    std::uint64_t extra_offset{};

    std::uint64_t strip_offset{};
    std::int32_t stored_strip_length{};
    bool strip_marker_present{};
    std::uint32_t break_count{};

    [[nodiscard]] bool valid(std::uint64_t document_size) const noexcept;
};

struct EfmDocument final {
    std::uint64_t document_size{};
    std::uint32_t group_count{};
    std::uint8_t document_mode{};
    std::uint64_t document_pointer{};
    std::vector<ModelShellGroup> groups;
    std::vector<EfmPrimitiveBatch> batches;

    [[nodiscard]] std::uint64_t total_vertex_count() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
};

struct EfmParseResult final {
    std::optional<EfmDocument> document;
    EfmParseError error{EfmParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == EfmParseError::none;
    }
};

// Structural reader for the DMC3 effect model, from `EfmContract::relocate_va`.
//
// No `EFM` payload exists in the supplied corpus, so unlike the scene and
// model readers this one has never been run against a real file. It implements
// the recovered routine's requirements and nothing more.
class EfmParser final {
public:
    [[nodiscard]] static EfmParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
