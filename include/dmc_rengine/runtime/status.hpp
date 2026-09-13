#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <version>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
#define DMC_RENGINE_HAS_STD_EXPECTED 1
#else
#define DMC_RENGINE_HAS_STD_EXPECTED 0
#include <cassert>
#include <optional>
#endif

namespace dmc::rengine::runtime {

/// Failure categories the runtime host can report.
///
/// The runtime fails closed: anything it cannot prove it supports is reported
/// as an error rather than approximated. `unsupported` in particular means the
/// requested capability is declared but not yet implemented, which is a
/// distinct condition from `unavailable` (implemented, refused by the host).
enum class RuntimeErrorCode {
    invalid_argument,
    unsupported,
    unavailable,
    not_initialized,
    already_initialized,
    resource_missing,
    resource_unreadable,
    surface_lost,
    device_lost,
    internal,
};

[[nodiscard]] constexpr std::string_view to_string(RuntimeErrorCode code) noexcept {
    switch (code) {
    case RuntimeErrorCode::invalid_argument: return "invalid-argument";
    case RuntimeErrorCode::unsupported: return "unsupported";
    case RuntimeErrorCode::unavailable: return "unavailable";
    case RuntimeErrorCode::not_initialized: return "not-initialized";
    case RuntimeErrorCode::already_initialized: return "already-initialized";
    case RuntimeErrorCode::resource_missing: return "resource-missing";
    case RuntimeErrorCode::resource_unreadable: return "resource-unreadable";
    case RuntimeErrorCode::surface_lost: return "surface-lost";
    case RuntimeErrorCode::device_lost: return "device-lost";
    case RuntimeErrorCode::internal: return "internal";
    }
    return "internal";
}

struct RuntimeError final {
    RuntimeErrorCode code{RuntimeErrorCode::internal};
    std::string context;
    std::string message;

    [[nodiscard]] std::string describe() const {
        std::string text{to_string(code)};
        if (!context.empty()) {
            text += " [" + context + "]";
        }
        if (!message.empty()) {
            text += ": " + message;
        }
        return text;
    }

    friend bool operator==(const RuntimeError&, const RuntimeError&) = default;
};

[[nodiscard]] inline RuntimeError make_error(RuntimeErrorCode code, std::string context,
                                             std::string message) {
    return RuntimeError{code, std::move(context), std::move(message)};
}

#if DMC_RENGINE_HAS_STD_EXPECTED

/// C++23 `std::expected` is the primary result channel for the runtime layer.
template <class T>
using Expected = std::expected<T, RuntimeError>;

using Status = Expected<void>;

[[nodiscard]] inline std::unexpected<RuntimeError> fail(RuntimeError error) {
    return std::unexpected<RuntimeError>{std::move(error)};
}

[[nodiscard]] inline Status ok() {
    return Status{};
}

#else

/// Minimal stand-in used only where the toolchain ships no `<expected>`.
/// The surface is intentionally limited to what the runtime layer uses, so
/// that code written against the C++23 type keeps compiling unchanged.
template <class T>
class Expected final {
public:
    using value_type = T;
    using error_type = RuntimeError;

    Expected() : value_(T{}) {}
    Expected(T value) : value_(std::move(value)) {}
    Expected(RuntimeError error) : error_(std::move(error)) {}

    [[nodiscard]] bool has_value() const noexcept { return value_.has_value(); }
    explicit operator bool() const noexcept { return has_value(); }

    [[nodiscard]] const T& value() const {
        assert(has_value());
        return *value_;
    }
    [[nodiscard]] T& value() {
        assert(has_value());
        return *value_;
    }
    [[nodiscard]] const T& operator*() const { return value(); }
    [[nodiscard]] T& operator*() { return value(); }
    [[nodiscard]] const T* operator->() const { return &value(); }
    [[nodiscard]] T* operator->() { return &value(); }

    [[nodiscard]] const RuntimeError& error() const {
        assert(!has_value());
        return error_;
    }

    template <class U>
    [[nodiscard]] T value_or(U&& fallback) const {
        return has_value() ? *value_ : static_cast<T>(std::forward<U>(fallback));
    }

private:
    std::optional<T> value_{};
    RuntimeError error_{};
};

template <>
class Expected<void> final {
public:
    using value_type = void;
    using error_type = RuntimeError;

    Expected() = default;
    Expected(RuntimeError error) : failed_(true), error_(std::move(error)) {}

    [[nodiscard]] bool has_value() const noexcept { return !failed_; }
    explicit operator bool() const noexcept { return has_value(); }
    void value() const { assert(has_value()); }

    [[nodiscard]] const RuntimeError& error() const {
        assert(!has_value());
        return error_;
    }

private:
    bool failed_{false};
    RuntimeError error_{};
};

using Status = Expected<void>;

[[nodiscard]] inline RuntimeError fail(RuntimeError error) {
    return error;
}

[[nodiscard]] inline Status ok() {
    return Status{};
}

#endif

} // namespace dmc::rengine::runtime
