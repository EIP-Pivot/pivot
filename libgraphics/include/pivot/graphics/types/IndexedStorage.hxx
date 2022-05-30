#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace pivot
{

template <typename T>
/// Only accept hashable type
concept Hashable = requires(T a)
{
    {
        std::hash<T>{}(a)
        } -> std::convertible_to<std::size_t>;
};

template <Hashable Key, typename Value>
/// Store a type in a vector, while keeping a map to the indexes
class IndexedStorage
{
private:
    using size_type = typename std::vector<Value>::size_type;

public:
    IndexedStorage() = default;
    ~IndexedStorage() = default;

    /// return an iterator over the indexes
    auto begin() { return index.begin(); }
    /// return the end iterator
    auto end() { return index.end(); }
    /// return an iterator over the indexes
    auto begin() const { return index.begin(); }
    /// return the end iterator
    auto end() const { return index.end(); }

    /// Clear the internal storage
    void clear()
    {
        index.clear();
        storage.clear();
    }
    /// Reserve extra memory
    void reserve(size_type size)
    {
        index.reserve(size);
        storage.reserve(size);
    }

    /// Append to the current storage
    void append(const IndexedStorage &stor)
    {
        for (const auto &[name, idx]: stor) { add(name, stor.getStorage().at(idx)); }
    }

    /// Add a new item to the storage
    inline void add(const Key &i, Value value)
    {
        if (contains(i)) throw std::runtime_error("Index already in use !");
        storage.push_back(std::move(value));
        index.insert(std::make_pair(i, storage.size() - 1));
    }
    /// @copydoc add
    inline void add(const std::pair<Key, Value> &value) { add(value.first, std::move(value.second)); }
    /// Valueell if given key already exist in storage
    inline bool contains(const Key &i) const { return index.contains(i); }
    /// return the number of item in the storage
    constexpr auto size() const noexcept
    {
        assert(storage.size() == index.size());
        return storage.size();
    }
    /// return whether or not the size is equal to 0
    constexpr bool empty() const noexcept { return size() == 0; }

    /// return the internal vector
    constexpr const auto &getStorage() const noexcept { return storage; }
    /// @copydoc getStorage
    constexpr auto &getStorage() noexcept { return storage; }

    /// Get the name associated to given idx
    constexpr const Key &getName(const size_type &idx) const
    {
        auto findResult =
            std::find_if(index.begin(), index.end(), [&idx](const auto &pair) { return pair.second == idx; });
        if (findResult != index.end()) return findResult->first;
        throw std::out_of_range("Out of range index: " + std::to_string(idx));
    }
    /// return the index of an item name
    inline std::int32_t getIndex(const Key &i) const noexcept
    {
        if (index.contains(i))
            return index.at(i);
        else
            return -1;
    }

    /// return the item at a given index
    constexpr Value &get(const size_type &i) { return storage.at(i); }
    /// @copydoc get
    constexpr Value &get(const Key &i) { return get(index.at(i)); }
    /// @copydoc get
    constexpr const Value &get(const size_type &i) const { return storage.at(i); }
    /// @copydoc get
    constexpr const Value &get(const Key &i) const { return get(index.at(i)); }
    /// Get the item, if it doesnt exist, create the index
    inline Value &operator[](const std::string &i)
    {
        if (!index.contains(i)) add(i, {});
        return get(i);
    }

private:
    std::vector<Value> storage;
    std::unordered_map<Key, size_type> index;
};

}    // namespace pivot
