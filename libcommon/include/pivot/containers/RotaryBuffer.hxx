#pragma once

#include <array>

namespace pivot
{

///
/// @brief A wrapper around and std::array that hold a internal index to access a specific index at each rotation
///
/// @tparam T The
/// @tparam Size
///
template <typename T, std::size_t Size>
class RotaryBuffer : public std::array<T, Size>
{
public:
    ///
    /// @brief Advance the internal index (rotate the buffer)
    ///
    /// @return std::size_t the new index
    ///
    constexpr std::size_t advance() noexcept
    {
        currentIndex = (currentIndex + 1) % Size;
        return currentIndex;
    }

    /// Return the current object pointer by the internal index
    constexpr T &get() { return this->at(currentIndex); }
    /// Return the current object pointer by the internal index
    constexpr const T &get() const { return this->at(currentIndex); }
    /// Return the internal index
    constexpr std::size_t getIndex() const noexcept { return currentIndex; }

    /// Return the current object pointer by the internal index
    constexpr T *operator->() noexcept { return this->operator[](currentIndex); }
    /// Return the current object pointer by the internal index
    constexpr const T *operator->() const noexcept { return this->operator[](currentIndex); }

private:
    std::size_t currentIndex = 0;
};

}    // namespace pivot
