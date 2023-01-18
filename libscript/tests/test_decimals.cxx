#include <catch2/catch_test_macros.hpp>

#include "pivot/script/Engine.hxx"

#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-Interpreter-Decimals")
{
    // logger.start();

    logger.info("------Interpreter Decimals------") << "Start";

    component::Index cind;
    systems::Index sind;
    event::Index eind;
    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());
    // std::string file = "C:/Users/Najo/eip/pivot/libscript/tests/decimals.pvt";
    // engine.loadFile(file);
    std::string fileContent = "component C\n"
                              "\tVector3 vec\n"
                              "system S(anyEntity<C>) event Tick(Number deltaTime)\n"
                              "\tprint(anyEntity.C.vec)\n"
                              "\tanyEntity.C.vec.x = 39.45\n"
                              "\tanyEntity.C.vec.y = (148 % 12)\n"
                              "\tNumber n = 12 * 48.415 * 65.4\n"
                              "\tn = n * 0.648 + 41.46 * 12 * 48.415 * 65.4\n"
                              "\tanyEntity.C.vec.z = n\n"
                              "\tprint(anyEntity.C.vec)\n";
    engine.loadFile(fileContent, true);

    // REQUIRE(sind.getDescription("S").has_value());
    // REQUIRE(cind.getDescription("C").has_value());
    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"vec", glm::vec3{1.0, 2.0, 3.0}}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event =
            event::Event{
                .description = Sdescription.eventListener,
                .entities = {1, 2},
                .payload = 0.12,
            },
    };

    Sdescription.system(Sdescription, combinations, evt);

    glm::vec3 vec = std::get<glm::vec3>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("vec"));
    REQUIRE(vec.x == 39.45f);
    REQUIRE(vec.y == 4.0f);
    REQUIRE(vec.z == 1599939.44194f);

    logger.info("------Interpreter Decimals------") << "End";
}
