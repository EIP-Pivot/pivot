#pragma once

#include <cstdint>
#include <type_traits>

namespace pivot
{

using FlagsType = std::uint32_t;

template <typename BitType>
requires std::is_enum_v<BitType>
class Flags
{
public:
    using MaskType = typename std::underlying_type<BitType>::type;

    constexpr Flags() noexcept: m_mask(0) {}
    constexpr Flags(BitType bit) noexcept: m_mask(static_cast<MaskType>(bit)) {}
    constexpr Flags(Flags<BitType> const &rhs) noexcept = default;
    constexpr explicit Flags(MaskType flags) noexcept: m_mask(flags) {}

    auto operator<=>(Flags<BitType> const &) const = default;

    // logical operator
    constexpr bool operator!() const noexcept { return !m_mask; }

    // bitwise operators
    constexpr Flags<BitType> operator&(Flags<BitType> const &rhs) const noexcept
    {
        return Flags<BitType>(m_mask & rhs.m_mask);
    }
    constexpr Flags<BitType> operator|(Flags<BitType> const &rhs) const noexcept
    {
        return Flags<BitType>(m_mask | rhs.m_mask);
    }
    constexpr Flags<BitType> operator^(Flags<BitType> const &rhs) const noexcept
    {
        return Flags<BitType>(m_mask ^ rhs.m_mask);
    }
    // assignment operators
    constexpr Flags<BitType> &operator=(Flags<BitType> const &rhs) noexcept = default;
    constexpr Flags<BitType> &operator|=(Flags<BitType> const &rhs) noexcept
    {
        m_mask |= rhs.m_mask;
        return *this;
    }
    constexpr Flags<BitType> &operator&=(Flags<BitType> const &rhs) noexcept
    {
        m_mask &= rhs.m_mask;
        return *this;
    }
    constexpr Flags<BitType> &operator^=(Flags<BitType> const &rhs) noexcept
    {
        m_mask ^= rhs.m_mask;
        return *this;
    }

    // cast operators
    explicit constexpr operator bool() const noexcept { return !!m_mask; }
    explicit constexpr operator MaskType() const noexcept { return m_mask; }

private:
    MaskType m_mask;
};

template <typename T>
requires std::is_enum_v<T>
constexpr bool enable_flag_for_enum = false;

template <typename T>
requires enable_flag_for_enum<T>
constexpr Flags<T> operator|(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) | lhs; }

template <typename T>
requires enable_flag_for_enum<T>
constexpr Flags<T> operator^(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) ^ lhs; }

template <typename T>
requires enable_flag_for_enum<T>
constexpr Flags<T> operator&(const T &rhs, const T &lhs) noexcept { return Flags<T>(rhs) & lhs; }

}    // namespace pivot

#define ENABLE_FLAGS_FOR_ENUM(type)                         \
    namespace pivot                                         \
    {                                                       \
        template <>                                         \
        constexpr bool enable_flag_for_enum<::type> = true; \
    }
