#pragma once

#include <cstdint>
#include <type_traits>

#define BIT(x) (1u << x)

namespace pivot
{

/// Default type for enums
using FlagsType = std::uint32_t;

template <typename BitType>
/// Wrapper for result of flag bits
requires std::is_enum_v<BitType>
class Flags
{
public:
    /// Underlying type of the flag
    using MaskType = typename std::underlying_type<BitType>::type;

    /// Default ctor
    constexpr Flags() noexcept: m_mask(0) {}
    /// Default ctor
    constexpr Flags(BitType bit) noexcept: m_mask(static_cast<MaskType>(bit)) {}
    /// Copy ctor
    constexpr Flags(const Flags<BitType> &rhs) noexcept = default;
    /// Construct with a int value
    constexpr explicit Flags(MaskType flags) noexcept: m_mask(flags) {}

    /// default starship operator
    auto operator<=>(const Flags<BitType> &) const = default;

    /// logical operator
    constexpr bool operator!() const noexcept { return !m_mask; }

    /// bitwise AND operator
    constexpr Flags<BitType> operator&(const Flags<BitType> &rhs) const noexcept
    {
        return Flags<BitType>(m_mask & rhs.m_mask);
    }
    /// bitwise OR operator
    constexpr Flags<BitType> operator|(const Flags<BitType> &rhs) const noexcept
    {
        return Flags<BitType>(m_mask | rhs.m_mask);
    }
    /// bitwise XOR operator
    constexpr Flags<BitType> operator^(const Flags<BitType> &rhs) const noexcept
    {
        return Flags<BitType>(m_mask ^ rhs.m_mask);
    }
    /// assignment operator
    constexpr Flags<BitType> &operator=(const Flags<BitType> &rhs) noexcept = default;
    /// bitwise AND assignment operator
    constexpr Flags<BitType> &operator&=(const Flags<BitType> &rhs) noexcept
    {
        m_mask &= rhs.m_mask;
        return *this;
    }
    /// bitwise OR assignment operator
    constexpr Flags<BitType> &operator|=(const Flags<BitType> &rhs) noexcept
    {
        m_mask |= rhs.m_mask;
        return *this;
    }
    /// bitwise XOR assignment operator
    constexpr Flags<BitType> &operator^=(const Flags<BitType> &rhs) noexcept
    {
        m_mask ^= rhs.m_mask;
        return *this;
    }

    /// boolean cast operator
    explicit constexpr operator bool() const noexcept { return !!m_mask; }
    /// Underlying type cast operator
    explicit constexpr operator MaskType() const noexcept { return m_mask; }

private:
    MaskType m_mask;
};

template <typename T>
/// Enable flags bitwise operation
requires std::is_enum_v<T>
constexpr bool enable_flag_for_enum = false;

template <typename T>
/// bitwise AND operator
requires enable_flag_for_enum<T>
constexpr Flags<T> operator&(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) & lhs; }

template <typename T>
/// bitwise OR operator
requires enable_flag_for_enum<T>
constexpr Flags<T> operator|(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) | lhs; }

template <typename T>
/// bitwise XOR operator
requires enable_flag_for_enum<T>
constexpr Flags<T> operator^(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) ^ lhs; }

}    // namespace pivot

#define ENABLE_FLAGS_FOR_ENUM(type)                         \
    namespace pivot                                         \
    {                                                       \
        template <>                                         \
        constexpr bool enable_flag_for_enum<::type> = true; \
    }
