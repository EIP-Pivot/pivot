#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "pivot/pivot.hxx"

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
    void append(const IndexedStorage &other)
    {
        for (const auto &[name, idx]: other.getIndexes()) {
            if (contains(name)) throw std::runtime_error("Index already in use !");
            index.emplace(name, idx + storage.size());
        }
        storage.insert(storage.end(), other.getStorage().begin(), other.getStorage().end());
    }

    /// Remove key from storage
    void erase(const Key &key)
    {
        auto idx = getIndex(key);
        if (idx == -1) throw std::out_of_range("Key not found !");
        storage.erase(storage.begin() + idx);
        index.erase(key);
    }
    /// Add a new item to the storage
    inline void add(const Key &i, Value value)
    {
        if (contains(i)) throw std::runtime_error("Index already in use !");
        storage.push_back(std::move(value));
        index.emplace(i, storage.size() - 1);
    }
    /// @copydoc add
    inline void add(const std::pair<Key, Value> value) { add(value.first, std::move(value.second)); }
    /// Valueell if given key already exist in storage
    inline bool contains(const Key &i) const { return index.contains(i); }
    /// return the number of item in the storage
    constexpr auto size() const noexcept
    {
        pivot_assert(storage.size() == index.size(), "Index and Storage are not the same.");
        return storage.size();
    }
    /// return whether or not the size is equal to 0
    constexpr bool empty() const noexcept { return size() == 0; }

    /// return the internal vector
    constexpr const auto &getStorage() const noexcept { return storage; }
    /// @copydoc getStorage
    constexpr auto &getStorage() noexcept { return storage; }
    /// return the internal vector
    constexpr const auto &getIndexes() const noexcept { return index; }
    /// @copydoc getIndexes
    constexpr auto &getIndexes() noexcept { return index; }

    /// Get the name associated to given idx
    constexpr std::optional<Key> getName(const size_type &idx) const
    {
        auto findResult =
            std::find_if(index.begin(), index.end(), [&idx](const auto &pair) { return pair.second == idx; });
        if (findResult != index.end()) return findResult->first;
        return std::nullopt;
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
    /// Equality operator
    bool operator==(const IndexedStorage &) const = default;

private:
    std::vector<Value> storage;
    std::unordered_map<Key, size_type> index;
};

}    // namespace pivot
