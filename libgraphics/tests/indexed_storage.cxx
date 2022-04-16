#include <catch2/catch_test_macros.hpp>

#include "pivot/graphics/types/IndexedStorage.hxx"

using namespace pivot;

const std::string value1 = "This is a special value";
const std::string key1 = "potato";
const std::string value2 = "Punctuation is important";
const std::string key2 = "flower";
const std::string value3 = "The floor here is made out of floor";
const std::string key3 = "hello";
const std::pair<std::string, std::string> pair3 = std::make_pair(key3, value3);

const std::string key4 = "arrow";
const std::string value4 = "is the knee";

template <typename F, typename T>
bool testIndex(const IndexedStorage<F, T> &storage, const std::string &key, const T &value, const unsigned index)
{

    REQUIRE(storage.contains(key));
    REQUIRE(storage.get(key) == value);
    REQUIRE(storage.get(index) == value);
    REQUIRE(storage.getIndex(key) == index);
    REQUIRE(storage.getName(index) == key);
    REQUIRE(storage.getStorage().at(index) == value);
    return true;
}

TEST_CASE("Indexed Storage", "[indexedStorage]")
{
    IndexedStorage<std::string, std::string> storage;

    REQUIRE(storage.size() == 0);
    REQUIRE(storage.empty());

    REQUIRE_NOTHROW(storage.add(key1, value1));
    REQUIRE(storage.size() == 1);
    REQUIRE(!storage.empty());
    REQUIRE(testIndex(storage, key1, value1, 0));

    REQUIRE_NOTHROW(storage.add(key2, value2));
    REQUIRE(storage.size() == 2);
    REQUIRE(testIndex(storage, key1, value1, 0));
    REQUIRE(testIndex(storage, key2, value2, 1));

    REQUIRE_NOTHROW(storage.add(pair3));
    REQUIRE(storage.size() == 3);
    REQUIRE(testIndex(storage, key1, value1, 0));
    REQUIRE(testIndex(storage, key2, value2, 1));
    REQUIRE(testIndex(storage, key3, value3, 2));

    REQUIRE(storage.getStorage() == std::vector{value1, value2, value3});

    SECTION("Test appends")
    {
        IndexedStorage<std::string, std::string> storage2;
        storage2.add(key4, value4);

        REQUIRE_NOTHROW(storage.append(storage2));
        REQUIRE(testIndex(storage, key1, value1, 0));
        REQUIRE(testIndex(storage, key2, value2, 1));
        REQUIRE(testIndex(storage, key3, value3, 2));
        REQUIRE(testIndex(storage, key4, value4, 3));
    }

    SECTION("Test errors cases")
    {
        REQUIRE_THROWS_AS(storage.append(storage), std::runtime_error);
        REQUIRE_THROWS_AS(storage.add(key2, value2), std::runtime_error);
        REQUIRE_THROWS_AS(storage.add(pair3), std::runtime_error);
        REQUIRE_THROWS_AS(storage.getName(42), std::out_of_range);
        REQUIRE(storage.getIndex("Nope") == -1);

        SECTION("Test array operator")
        {
            REQUIRE(storage["Nope"] == std::string());
            REQUIRE(storage.size() == 4);
            REQUIRE(testIndex(storage, "Nope", std::string(), 3));
        }
    }
}
