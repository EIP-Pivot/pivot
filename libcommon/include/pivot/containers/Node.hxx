#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace pivot
{

template <typename Key, typename Value>
/// @class Node
///
/// Implementation for a generic tree
class Node : public std::enable_shared_from_this<Node<Key, Value>>
{
public:
    /// alias for std::shared_ptr
    using NodePtr = std::shared_ptr<Node>;
    /// alias for std::reference_wrapper
    using NodeRef = Node &;

public:
    Node() = delete;
    /// Construct a node with a value
    constexpr Node(Key key, Value value): key(std::move(key)), value(std::move(value)) {}
    /// Construct a node with a default value
    constexpr Node(Key key): Node(std::move(key), {}) {}

    /// Set parent Node
    constexpr void setParent(NodePtr p) { parent = p; }
    /// add a child to the current node
    ///
    /// @return the amount of child of the current node
    constexpr std::size_t addChild(NodePtr child)
    {
        /// Need to make sure the key does not repeat
        if (getNodeInAllTree(child->key)) throw std::logic_error("Duplicated key found in Node tree!");

        child->setParent(this->shared_from_this());
        children.push_back(std::move(child));
        return children.size();
    }

    /// Construct a child node
    NodePtr emplaceChild(Key key, Value val = {})
    {
        auto ptr = std::make_shared<Node>(key, val);
        addChild(ptr);
        return ptr;
    }

    /// does the current node does not have a parent
    constexpr bool isRoot() const noexcept { return parent == nullptr; }
    /// does the current node has any children
    constexpr bool isLeaf() const noexcept { return children.empty(); }
    /// return the root of the tree. Can't handle tree with multiple root
    constexpr NodePtr getRoot() noexcept
    {
        if (isRoot())
            return this->shared_from_this();
        else
            return parent->getRoot();
    }
    /// return the number of node down the tree, starting from the current node
    std::size_t size() const noexcept { return size_imp(0); }

    /// return the number of node up the tree, starting from the current node
    std::size_t depth() const noexcept { return depth_imp(0); }

    /// Traverse down the tree and execute function on each node
    void traverseDown(std::invocable<Node &> auto function)
    {
        function(*this);
        for (auto &child: children) { child->traverseDown(function); }
    }
    /// @copydoc traverseDown
    void traverseDown(std::invocable<const Node &> auto function) const
    {
        function(*this);
        for (const auto &child: children) { child->traverseDown(function); }
    }

    /// Traverse up the tree and execute function on eache node
    void traverseUp(std::invocable<Node &> auto function)
    {
        function(*this);
        if (parent) { parent->traverseUp(function); }
    }
    /// @copydoc traverseUp
    void traverseUp(std::invocable<const Node &> auto function) const
    {
        function(*this);
        if (parent) { parent->traverseUp(function); }
    }

    /// @brief get the node with the key, searching from this node down
    ///
    /// return std::nullopt if none was found
    std::optional<NodePtr> getNode(const Key &k) requires std::equality_comparable<Key>
    {
        if (key == k) {
            return this->shared_from_this();
        } else {
            for (auto &child: children) {
                auto tmp = child->getNode(k);
                if (tmp) return tmp;
            }
        }
        return std::nullopt;
    }
    /// @brief get the node with the key; from the root node
    ///
    /// return std::nullopt if none was found
    std::optional<NodePtr> getNodeInAllTree(const Key &k) requires std::equality_comparable<Key>
    {
        if (key == k) {
            return this->shared_from_this();
        } else {
            return getRoot()->getNode(k);
        }
    }

    /// Equality operator
    bool operator==(const Node &) const requires std::equality_comparable<Key> && std::equality_comparable<Value>
    = default;

    /// Starship operator
    auto operator<=>(const Node &) const requires std::three_way_comparable<Key> && std::three_way_comparable<Value>
    = default;

private:
    std::size_t size_imp(std::size_t size = 0) const
    {
        traverseDown([&size](const Node &) { size += 1; });
        return size;
    }

    std::size_t depth_imp(std::size_t size = 0) const
    {
        traverseUp([&size](const Node &) { size += 1; });
        return size;
    }

public:
    /// the key
    Key key;
    /// the value
    Value value;

private:
    /// the children of the node
    std::vector<NodePtr> children;
    /// the parent of the node
    NodePtr parent = nullptr;
};

}    // namespace pivot
