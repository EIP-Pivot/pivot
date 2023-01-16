#include "pivot/script/Engine.hxx"
// #include "pivot/ecs/Core/Scripting/Parser.hxx"
// #include "pivot/ecs/Core/Scripting/Interpreter.hxx"
// #include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-negatives-expressions")
{
    std::cout << "------Interpreter-negatives-----start" << std::endl;

    component::Index cind;
    systems::Index sind;
    event::Index eind;
    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());

    std::string fileContent = "component C\n"
                              "\tNumber Mdr\n"
                              "\tNumber Ptdr\n"
                              "system S (anyEntity<C>) event Tick(Number deltaTime)\n"
                              "\tanyEntity.C.Mdr = -12\n"
                              "\tNumber lul = -12\n"
                              "\tanyEntity.C.Ptdr = -lul\n";

    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"Mdr", 0.0}, {"Ptdr", 0.0}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    Sdescription.system(Sdescription, combinations, evt);
    REQUIRE(std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("Mdr")) == -12);
    REQUIRE(std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("Ptdr")) == 12);

    std::cout << "------Interpreter------end" << std::endl;
}
