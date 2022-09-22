#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <pivot/builtins/components/Transform.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Data/value_serialization.hxx>
#include <pivot/graphics/types/Transform.hxx>
#include <pivot/internal/TransformArray.hxx>

using namespace nlohmann;
using namespace pivot::ecs::data;
using namespace pivot::ecs::component;
using namespace pivot::builtins::components;
using namespace pivot::internal;
using namespace pivot;

void check_matrices(const glm::mat4 &received, const glm::mat4 &expected)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) { REQUIRE(received[i][j] == expected[i][j]); }
    }
}

// std::ostream &operator<<(std::ostram &)

void check_transform(const graphics::Transform &received, const graphics::Transform &expected)
{
    REQUIRE(received.position == expected.position);
    REQUIRE(received.rotation == expected.rotation);
    REQUIRE(received.scale == expected.scale);
}

TEST_CASE("Transform component works", "[graphics][component]")
{
    auto description = Transform::description;
    auto array = description.createContainer(description);

    const auto data = R"({"scale": [1,2,3], "position": [5, 5, 5], "rotation": [0, 2.7, 0.4], "root": {"entity": 42}})";

    REQUIRE_NOTHROW(array->setValueForEntity(0, json::parse(data).get<Value>()));

    const pivot::graphics::Transform expected{{5, 5, 5}, {0, 2.7, 0.4}, {1, 2, 3}, EntityRef::empty()};
    const pivot::graphics::Transform &transform =
        dynamic_cast<DenseTypedComponentArray<pivot::graphics::Transform> &>(*array).getData().front();
    REQUIRE(transform == expected);

    REQUIRE(description.defaultValue == Value{Record{{"position", Value{glm::vec3{0, 0, 0}}},
                                                     {"rotation", Value{glm::vec3{0, 0, 0}}},
                                                     {"scale", Value{glm::vec3{1, 1, 1}}},
                                                     {"root", Value{pivot::EntityRef::empty()}}}});

    const pivot::graphics::Transform object{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}, EntityRef::empty()};
    const pivot::graphics::Transform root{{3, 1, 2}, {3, 1, 2}, {3, 1, 2}, EntityRef::empty()};
    const pivot::graphics::Transform identity{{0, 0, 0}, {0, 0, 0}, {1, 1, 1}, EntityRef::empty()};

    // auto object_with_root = object.with_root(root);
    // auto model_matrix_with_root = object_with_root.getModelMatrix() * root.getModelMatrix();
    // auto model_matrix_normal = object.getModelMatrix();
    // FIXME: rotation is bad
    REQUIRE_FALSE(object.with_root(identity) == object);
    REQUIRE_FALSE(object.with_root(root).remove_root(root) == object);
}

namespace
{

void setTransform(TransformArray &array, Entity i, EntityRef root = EntityRef::empty())
{
    auto t = graphics::Transform{.position = {i, i, i}, .rotation = {i, i, i}, .scale = {i, i, i}, .root = root};
    array.setValueForEntity(i, array.unparseValue(t));
}
}    // namespace

TEST_CASE("Transform array works", "[graphics][component]")
{
    TransformArray array(Transform::description);

    setTransform(array, 0);
    REQUIRE(array.getData()[0].root == EntityRef::empty());
    setTransform(array, 1, {0});
    REQUIRE(array.getData()[1].root == EntityRef{0});

    REQUIRE_THROWS_AS(setTransform(array, 2, {1}), TransformArray::RootDepthExceeded);
    REQUIRE(array.getData()[2].root == EntityRef::empty());
    setTransform(array, 2, {0});
    REQUIRE(array.getData()[2].root == EntityRef{0});

    array.setValueForEntity(0, std::nullopt);
    REQUIRE(array.getData()[1].root == EntityRef::empty());
    REQUIRE(array.getData()[2].root == EntityRef::empty());

    setTransform(array, 0, {0});
    REQUIRE(array.getData()[0].root == EntityRef::empty());
    setTransform(array, 1, {1});
    REQUIRE(array.getData()[1].root == EntityRef::empty());
    // array.
}
