#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace pivot
{

template <typename T, typename Idx = typename std::vector<T>::size_type>
/// Store a type in a vector, while keeping a map to the indexes
class IndexedStorage
{
public:
    IndexedStorage() = default;
    ~IndexedStorage() = default;

    /// return an iterator over the indexes
    auto begin() { return index.begin(); }
    /// return the end iterator
    auto end() { return index.end(); }
    /// Add a new item to the storage
    inline void add(const std::string &i, T value)
    {
        storage.push_back(std::move(value));
        index.insert(std::make_pair(i, storage.size() - 1));
    }
    /// @copydoc add
    inline void add(const std::pair<std::string, T> &value) { add(value.first, std::move(value.second)); }
    /// return the number of item in the storage
    constexpr auto size() const noexcept
    {
        assert(storage.size() == index.size());
        return storage.size();
    }
    /// return the internal vector
    constexpr const auto &getStorage() const noexcept { return storage; }
    /// @copydoc getStorage
    constexpr auto &getStorage() noexcept { return storage; }
    /// return the item at a given index
    constexpr const T &get(const std::int32_t &i) const { return storage.at(i); }

    /// Get the name associated to givent idx
    constexpr const std::string &getName(const std::size_t &idx) const
    {
        auto findResult =
            std::find_if(index.begin(), index.end(), [&idx](const auto &pair) { return pair.second == idx; });
        if (findResult != index.end()) return findResult->first;
        throw std::out_of_range("Out of range index: " + std::to_string(idx));
    }

    /// return the index of an item name
    inline const std::int32_t getIndex(const std::string &i) const noexcept
    {
        if (index.contains(i))
            return index.at(i);
        else
            return -1;
    }

private:
    std::vector<T> storage;
    std::unordered_map<std::string, Idx> index;
};

}    // namespace pivot