#pragma once

#include <array>

namespace pivot
{

template <typename T, std::size_t Size>
class RotaryBuffer
{
public:
    constexpr RotaryBuffer() = default;
    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr RotaryBuffer(Args... args) { fill(T(args...)); }
    constexpr RotaryBuffer(const RotaryBuffer &) = default;
    constexpr RotaryBuffer(RotaryBuffer &&) = default;
    constexpr auto begin() noexcept { return m_InternalArray.begin(); }
    constexpr auto end() noexcept { return m_InternalArray.end(); }
    constexpr auto begin() const noexcept { return m_InternalArray.begin(); }
    constexpr auto end() const noexcept { return m_InternalArray.end(); }
    constexpr auto cbegin() const noexcept { return m_InternalArray.cbegin(); }
    constexpr auto cend() const noexcept { return m_InternalArray.cend(); }
    constexpr auto rbegin() const noexcept { return m_InternalArray.rbegin(); }
    constexpr auto rend() const noexcept { return m_InternalArray.rend(); }

    constexpr std::size_t size() const noexcept { return Size; }
    constexpr void fill(const T &value) { m_InternalArray.fill(value); }
    constexpr std::size_t advance() noexcept
    {
        currentIndex = (currentIndex + 1) % Size;
        return currentIndex;
    }

    constexpr T &get() { return m_InternalArray[currentIndex]; }
    constexpr const T &get() const { return m_InternalArray[currentIndex]; }
    constexpr T &at(unsigned idx) { return m_InternalArray.at(idx); }
    constexpr const T &at(unsigned idx) const { return m_InternalArray.at(idx); }

    constexpr std::size_t getIndex() const noexcept { return currentIndex; }

    constexpr RotaryBuffer &operator=(const RotaryBuffer &) = default;
    constexpr T *operator->() noexcept { return m_InternalArray[currentIndex]; }
    constexpr const T *operator->() const noexcept { return m_InternalArray[currentIndex]; }

private:
    std::array<T, Size> m_InternalArray;
    std::size_t currentIndex = 0;
};

}    // namespace pivot
