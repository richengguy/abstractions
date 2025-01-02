#pragma once

#include <abstractions/math/types.h>

#include <expected>
#include <optional>
#include <string>
#include <type_traits>

namespace abstractions {

/// @brief Common error type
///
/// This will be empty if the function or method returning the error type
/// completed successfully.  Otherwise it will return a string describing the
/// reason for the error.
using Error = std::optional<std::string>;

/// @brief Stores the expected result of an operation.
/// @tparam T expected result type
///
/// If an error occurs then this will contain an Error value instead of the
/// expected result type.
template <typename T>
using Expected = std::expected<T, Error>;

/// @brief A wrapper that allows enums to be used like a bitmask.
/// @tparam T a scope enum type
///
/// This is adapted from https://gpfault.net/posts/typesafe-bitmasks.txt.html.
template <typename T>
class Options {
    static_assert(std::is_scoped_enum_v<T>, "Can only wrap scoped enums.");

public:
    /// @brief The wrapped enum's underlying type.
    using enum_type = std::underlying_type_t<T>;

    /// @brief Create a new Options instance with none of the options
    ///     having been set.
    constexpr Options() :
        Options(0) {}

    /// @brief Create a new Options instance from the given value.
    /// @param value enum value
    constexpr Options(T value) :
        _value{MaskValue(value)} {}

    /// @brief Sets the bit corresponding to the given enum value.
    /// @param value enum value
    constexpr void Set(T value) {
        _value = _value | MaskValue(value);
    }

    /// @brief Clears the bit corresponding to the given enum value.
    /// @param value enum value
    constexpr void Clear(T value) {
        _value = _value & ~MaskValue(value);
    }

    /// @brief Implements the `union = a | b` operation.
    /// @param other enum value
    /// @return an updated MaskedOption
    constexpr Options operator|(T other) const {
        return Options(_value | MaskValue(other));
    }

    /// @brief Checks if the given bit is set in the masked option.
    /// @param value enum value
    /// @return whether or not the bit is set
    constexpr bool operator&(T value) const {
        return _value & MaskValue(value);
    }

    constexpr bool operator==(const Options<T> &other) const {
        return _value == other._value;
    }

    constexpr bool operator!() {
        return !static_cast<bool>(*this);
    }

    constexpr operator bool() {
        return _value != 0;
    }

private:
    static constexpr enum_type MaskValue(T value) {
        return 1 << static_cast<enum_type>(value);
    }

    explicit constexpr Options(enum_type value) :
        _value{value} {}

    enum_type _value;
};

template <typename T>
constexpr Options<T> operator|(T a, T b) {
    return Options(a) | b;
}

/// @brief Defines the `|` operator for an Options-wrapped enum type.
#define ABSTRACTIONS_OPTIONS_ENUM(type)                                 \
    constexpr ::abstractions::Options<type> operator|(type a, type b) { \
        return ::abstractions::Options<type>(a) | b;                    \
    }

}  // namespace abstractions
