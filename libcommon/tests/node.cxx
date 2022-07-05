#include <catch2/catch_test_macros.hpp>

#include "pivot/containers/Node.hxx"

using NodeType = pivot::Node<std::string, std::string>;
TEST_CASE("Node test", "[node]")
{
    auto parent = std::make_shared<NodeType>("parent", "Coléoptère");
    auto child1 = parent->emplaceChild("child1", "Billy");
    auto child2 = parent->emplaceChild("child2", "Pyro-Barbarian");
    auto child11 = child1->emplaceChild("child11", "Rope");

    REQUIRE(parent->isRoot());
    REQUIRE(not parent->isLeaf());
    REQUIRE(not child1->isRoot());
    REQUIRE(child2->isLeaf());
    REQUIRE(parent->size() == 4);

    SECTION("Emplace")
    {
        REQUIRE(parent->key == "parent");
        REQUIRE(parent->value == "Coléoptère");

        REQUIRE(child1->key == "child1");
        REQUIRE(child1->value == "Billy");

        REQUIRE(child2->key == "child2");
        REQUIRE(child2->value == "Pyro-Barbarian");

        REQUIRE(child11->key == "child11");
        REQUIRE(child11->value == "Rope");
    }

    SECTION("Traversal Down")
    {
        std::vector<std::string> expect = {"parent", "child1", "child11", "child2"};
        std::vector<std::string> test;

        parent->traverseDown([&test](const NodeType &node) { test.push_back(node.key); });
        REQUIRE(test == expect);
    }

    SECTION("Traversal Up")
    {
        std::vector<std::string> expect = {"child11", "child1", "parent"};
        std::vector<std::string> test;

        child11->traverseUp([&test](const NodeType &node) { test.push_back(node.key); });
        REQUIRE(test == expect);
    }

    SECTION("getNode")
    {
        auto get = parent->getNode("child11");
        REQUIRE(get.has_value());
        REQUIRE(get.value() == child11);
    }

    SECTION("getRoot")
    {
        auto root = child11->getRoot();
        REQUIRE(root);
        REQUIRE(root == parent);
    }
}
