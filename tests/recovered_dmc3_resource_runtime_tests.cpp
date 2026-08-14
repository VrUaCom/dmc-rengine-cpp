#include "runtime/resources/resource_lifecycle.hpp"
#include "runtime/resources/resource_manager.hpp"
#include "runtime/resources/shw_postload.hpp"
#include "recovered_dmc3_mod_efm_postload_cases.hpp"
#include "recovered_dmc3_wave3_execution_cases.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace {

class RecordingPostLoadBackend final
    : public dmc::recovered::dmc3::runtime::resources::IConfirmedPostLoadBackend {
public:
    bool succeed{true};
    std::size_t call_count{};
    std::optional<dmc::recovered::dmc3::runtime::resources::PostLoadFormat>
        last_format;

    [[nodiscard]] bool apply(
        dmc::recovered::dmc3::runtime::resources::PostLoadFormat format,
        std::span<std::byte> bytes) override {
        ++call_count;
        last_format = format;
        if (bytes.size() > 3U) {
            bytes[3] = std::byte{0x7F};
        }
        return succeed;
    }
};

[[nodiscard]] std::uint64_t read_u64_le_for_test(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint64_t value{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

void write_u64_le_for_test(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

} // namespace

int main() {
    using namespace dmc::recovered::dmc3::runtime::resources;

    const auto& model = wave2_resource_runtime_model();
    assert(model.valid());
    assert(model.artifact_sha256 ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(model.evidence_packet_id == "dmc3-stage-wave2");

    const auto& pool = model.pool;
    assert(pool.valid());
    assert(pool.base_va == 0x140C99D30ULL);
    assert(pool.entry_count == 363U);
    assert(pool.entry.stride == 0x48U);
    assert(pool.entry.group_index_offset == 0x00U);
    assert(pool.entry.state_offset == 0x04U);
    assert(pool.entry.subtype_index_offset == 0x08U);
    assert(pool.entry.source_descriptor_offset == 0x18U);
    assert(pool.entry.loaded_payload_offset == 0x20U);
    assert(pool.entry.owned_state_offset == 0x28U);

    constexpr std::array<std::uint32_t, 7> counts{
        4U, 136U, 60U, 28U, 1U, 128U, 6U,
    };
    constexpr std::array<std::uint32_t, 7> offsets{
        0U, 4U, 140U, 200U, 228U, 229U, 357U,
    };
    for (std::uint32_t index = 0U; index < counts.size(); ++index) {
        const auto* group = pool.group(index);
        assert(group != nullptr);
        assert(group->first_entry == offsets[index]);
        assert(group->entry_count == counts[index]);
    }
    assert(pool.group(7U) == nullptr);
    assert(pool.groups.back().end_entry() == 363U);

    constexpr std::array<ResourceLoadState, 4> expected_state_chain{
        ResourceLoadState::free_or_unstarted,
        ResourceLoadState::io_scheduled_or_loading,
        ResourceLoadState::io_complete_pending_postprocess,
        ResourceLoadState::ready_postprocessed,
    };
    assert(model.successful_state_chain == expected_state_chain);
    assert(model.cleanup_transition.from ==
        ResourceLoadState::teardown_or_cancel_pending);
    assert(model.cleanup_transition.to ==
        ResourceLoadState::free_or_unstarted);
    assert(!model.teardown_source_state_domain_complete);

    const auto* mod = model.fixup(PostLoadFormat::mod);
    const auto* efm = model.fixup(PostLoadFormat::efm);
    const auto* scm = model.fixup(PostLoadFormat::scm);
    const auto* shw = model.fixup(PostLoadFormat::shw);
    assert(mod != nullptr && mod->function_va == 0x1402FE3B0ULL);
    assert(efm != nullptr && efm->function_va == 0x1402F7A90ULL);
    assert(scm != nullptr && scm->function_va == 0x1403051B0ULL);
    assert(shw != nullptr && shw->function_va == 0x1403204C0ULL);

    static_assert(sizeof(ResourceRuntimeEntry) == 0x48U);
    static_assert(offsetof(ResourceRuntimeEntry, group_index) == 0x00U);
    static_assert(offsetof(ResourceRuntimeEntry, state) == 0x04U);
    static_assert(offsetof(ResourceRuntimeEntry, subtype_index) == 0x08U);
    static_assert(offsetof(ResourceRuntimeEntry, source_descriptor) == 0x18U);
    static_assert(offsetof(ResourceRuntimeEntry, loaded_payload) == 0x20U);
    static_assert(offsetof(ResourceRuntimeEntry, owned_state) == 0x28U);

    ResourceRuntimeEntry entry{};
    entry.group_index = 5U;
    entry.subtype_index = 12U;
    entry.source_descriptor = 0x11110000ULL;
    entry.loaded_payload = 0x33330000ULL;
    entry.owned_state = 0x44440000ULL;

    assert(transition_free_to_loading(entry) ==
        ResourceTransitionResult::applied);
    assert(entry.state == ResourceLoadState::io_scheduled_or_loading);
    assert(entry.subtype_index == 12U);
    assert(entry.source_descriptor == 0x11110000ULL);
    assert(entry.loaded_payload == 0x33330000ULL);
    assert(entry.owned_state == 0x44440000ULL);

    assert(transition_free_to_loading(entry) ==
        ResourceTransitionResult::wrong_state);

    assert(transition_loading_to_io_complete(entry) ==
        ResourceTransitionResult::applied);
    assert(entry.state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(entry.loaded_payload == 0x33330000ULL);
    assert(entry.owned_state == 0x44440000ULL);

    std::array<std::byte, 8> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{0},
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4},
    };
    RecordingPostLoadBackend backend;
    backend.succeed = false;
    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::postload_failed);
    assert(entry.state ==
        ResourceLoadState::io_complete_pending_postprocess);
    assert(backend.call_count == 1U);
    assert(backend.last_format == PostLoadFormat::mod);

    backend.succeed = true;
    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::applied);
    assert(entry.state == ResourceLoadState::ready_postprocessed);
    assert(backend.call_count == 2U);
    assert(bytes[3] == std::byte{0x7F});

    assert(run_confirmed_postload(
        entry, PostLoadFormat::mod, bytes, backend) ==
        ResourceTransitionResult::wrong_state);

    constexpr std::array<PostLoadFormat, 4> formats{
        PostLoadFormat::mod,
        PostLoadFormat::efm,
        PostLoadFormat::scm,
        PostLoadFormat::shw,
    };
    for (const auto format : formats) {
        ResourceRuntimeEntry typed{};
        typed.state = ResourceLoadState::io_complete_pending_postprocess;
        assert(run_confirmed_postload(typed, format, bytes, backend) ==
            ResourceTransitionResult::applied);
        assert(typed.state == ResourceLoadState::ready_postprocessed);
    }

    std::array<std::byte, 0xB0> shw_bytes{};
    shw_bytes[shw_record_count_offset] = std::byte{2};
    constexpr std::array<std::uint64_t, 8> shw_relative_offsets{
        0x10ULL, 0x20ULL, 0x30ULL, 0x40ULL,
        0x50ULL, 0x60ULL, 0x70ULL, 0x80ULL,
    };
    for (std::size_t index = 0U; index < 4U; ++index) {
        write_u64_le_for_test(
            shw_bytes,
            shw_first_record_offset + index * 8U,
            shw_relative_offsets[index]);
        write_u64_le_for_test(
            shw_bytes,
            shw_first_record_offset + shw_record_stride + index * 8U,
            shw_relative_offsets[index + 4U]);
    }

    ResourceRuntimeEntry shw_entry{};
    shw_entry.state = ResourceLoadState::io_complete_pending_postprocess;
    ShwPostLoadBackend shw_backend;
    assert(run_confirmed_postload(
        shw_entry, PostLoadFormat::shw, shw_bytes, shw_backend) ==
        ResourceTransitionResult::applied);
    assert(shw_entry.state == ResourceLoadState::ready_postprocessed);

    const auto shw_base = static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(shw_bytes.data()));
    for (std::size_t index = 0U; index < 4U; ++index) {
        assert(read_u64_le_for_test(
            shw_bytes,
            shw_first_record_offset + index * 8U) ==
            shw_base + shw_relative_offsets[index]);
        assert(read_u64_le_for_test(
            shw_bytes,
            shw_first_record_offset + shw_record_stride + index * 8U) ==
            shw_base + shw_relative_offsets[index + 4U]);
    }

    std::array<std::byte, 0x50> truncated_shw{};
    truncated_shw[shw_record_count_offset] = std::byte{2};
    ResourceRuntimeEntry truncated_shw_entry{};
    truncated_shw_entry.state =
        ResourceLoadState::io_complete_pending_postprocess;
    assert(run_confirmed_postload(
        truncated_shw_entry,
        PostLoadFormat::shw,
        truncated_shw,
        shw_backend) == ResourceTransitionResult::postload_failed);
    assert(truncated_shw_entry.state ==
        ResourceLoadState::io_complete_pending_postprocess);

    ResourceRuntimeEntry wrong_shw_format{};
    wrong_shw_format.state =
        ResourceLoadState::io_complete_pending_postprocess;
    assert(run_confirmed_postload(
        wrong_shw_format,
        PostLoadFormat::mod,
        shw_bytes,
        shw_backend) == ResourceTransitionResult::postload_failed);
    assert(wrong_shw_format.state ==
        ResourceLoadState::io_complete_pending_postprocess);

    ResourceRuntimeEntry teardown_entry{};
    teardown_entry.group_index = 6U;
    teardown_entry.state = ResourceLoadState::teardown_or_cancel_pending;
    teardown_entry.loaded_payload = 0x77770000ULL;
    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::applied);
    assert(teardown_entry.state == ResourceLoadState::free_or_unstarted);
    assert(teardown_entry.loaded_payload == 0x77770000ULL);

    assert(apply_observed_cleanup_transition(teardown_entry) ==
        ResourceTransitionResult::wrong_state);

    // Disassembly-complete MOD/EFM helper bodies are now compiled and executed.
    // Corpus validation remains a separate evidence gate.
    dmc::recovered::dmc3::tests::mod_efm::run();

    // Directly recovered and evidence-bounded Wave-3 slices execute here too.
    dmc::recovered::dmc3::tests::wave3::run();

    return 0;
}
