#include <catch2/catch_test_macros.hpp>

#include "pivot/containers/Node.hxx"

using NodeType = pivot::Node<std::string>;
TEST_CASE("Node test", "[node]")
{
    auto parent = std::make_shared<NodeType>("parent");
    auto child1 = parent->emplaceChild("child1");
    auto child2 = parent->emplaceChild("child2");
    auto child11 = child1->emplaceChild("child11");

    REQUIRE(parent->isRoot());
    REQUIRE(not parent->isLeaf());
    REQUIRE(not child1->isRoot());
    REQUIRE(child2->isLeaf());
    REQUIRE(parent->size() == 4);

    REQUIRE(parent->value == "parent");
    REQUIRE(child1->value == "child1");
    REQUIRE(child2->value == "child2");
    REQUIRE(child11->value == "child11");

    SECTION("Traversal Down")
    {
        std::vector<std::string> expect = {"parent", "child1", "child11", "child2"};
        std::vector<std::string> test;

        parent->traverseDown([&test](const NodeType &node) { test.push_back(node.value); });
        REQUIRE(test == expect);
    }

    SECTION("Traversal Up")
    {
        std::vector<std::string> expect = {"child11", "child1", "parent"};
        std::vector<std::string> test;

        child11->traverseUp([&test](const NodeType &node) { test.push_back(node.value); });
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
