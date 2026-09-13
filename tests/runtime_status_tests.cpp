#include "dmc_rengine/runtime/status.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>

namespace {

using dmc::rengine::runtime::Expected;
using dmc::rengine::runtime::RuntimeError;
using dmc::rengine::runtime::RuntimeErrorCode;
using dmc::rengine::runtime::Status;
using dmc::rengine::runtime::fail;
using dmc::rengine::runtime::make_error;
using dmc::rengine::runtime::ok;

Expected<int> succeed() {
    return 7;
}

Expected<int> refuse() {
    return fail(make_error(RuntimeErrorCode::unsupported, "test", "not implemented"));
}

Status void_refuse() {
    return fail(make_error(RuntimeErrorCode::device_lost, "test", "device vanished"));
}

void error_description_is_readable() {
    const RuntimeError error{RuntimeErrorCode::resource_missing, "bridge", "no payload"};
    assert(error.describe() == "resource-missing [bridge]: no payload");

    const RuntimeError bare{RuntimeErrorCode::internal, {}, {}};
    assert(bare.describe() == "internal");
}

void expected_carries_values_and_errors() {
    const auto good = succeed();
    assert(good.has_value());
    assert(*good == 7);

    const auto bad = refuse();
    assert(!bad.has_value());
    assert(bad.error().code == RuntimeErrorCode::unsupported);
    assert(bad.error().context == "test");
}

void void_status_distinguishes_success_from_failure() {
    const auto good = ok();
    assert(good.has_value());

    const auto bad = void_refuse();
    assert(!bad.has_value());
    assert(bad.error().code == RuntimeErrorCode::device_lost);
}

void every_error_code_has_a_distinct_name() {
    const RuntimeErrorCode codes[]{
        RuntimeErrorCode::invalid_argument, RuntimeErrorCode::unsupported,
        RuntimeErrorCode::unavailable,      RuntimeErrorCode::not_initialized,
        RuntimeErrorCode::already_initialized, RuntimeErrorCode::resource_missing,
        RuntimeErrorCode::resource_unreadable, RuntimeErrorCode::surface_lost,
        RuntimeErrorCode::device_lost,      RuntimeErrorCode::internal,
    };

    for (std::size_t left = 0; left < std::size(codes); ++left) {
        assert(!to_string(codes[left]).empty());
        for (std::size_t right = left + 1; right < std::size(codes); ++right) {
            assert(to_string(codes[left]) != to_string(codes[right]));
        }
    }
}

} // namespace

int main() {
    error_description_is_readable();
    expected_carries_values_and_errors();
    void_status_distinguishes_success_from_failure();
    every_error_code_has_a_distinct_name();
    return 0;
}
