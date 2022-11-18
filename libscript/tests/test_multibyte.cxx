#include "pivot/script/Engine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-Interpreter-Multibyte")
{
    std::cout << "------Interpreter - Multibyte------start" << std::endl;

    component::Index cind;
    systems::Index sind;
    script::Engine engine(sind, cind, pivot::ecs::script::interpreter::builtins::BuiltinContext());
    // std::string file = "../libscript/tests/multibyte.pvt";
    // engine.loadFile(file);

    std::string fileContent = "component C\n"
                              "\tString str\n"
                              "system S(anyEntity<C>) event Tick(Number deltaTime)\n"
                              "\tanyEntity.C.str = "
                              "\"⒩❯⮘⦻ⵂ␨ⵝééé⢛⽶⹗⼹ⱷ≹⯹≣≫⼝ⷼ⷗⼦⾊⩵⁈◪⺈\"\n"
                              "\tprint(anyEntity.C.str)\n";
    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"str", ""}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    Sdescription.system(Sdescription, combinations, evt);

    std::string escapedString =
        std::get<std::string>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("str"));

    REQUIRE(escapedString == "⒩❯⮘⦻ⵂ␨ⵝééé⢛⽶⹗⼹ⱷ≹⯹≣≫⼝ⷼ⷗⼦⾊⩵⁈◪⺈");

    std::cout << "------Interpreter - Multibyte------end" << std::endl;
}
