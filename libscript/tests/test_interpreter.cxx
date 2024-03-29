#include "pivot/script/Engine.hxx"
// #include "pivot/ecs/Core/Scripting/Parser.hxx"
// #include "pivot/ecs/Core/Scripting/Interpreter.hxx"
// #include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace pivot::ecs;

TEST_CASE("Scripting-Refacto-Engine")
{
    std::cout << "------Engine------start" << std::endl;

    std::string file =
        "\n"
        "# component Size : Number\n"
        "# struct Size\n"
        "# 	Number x\n"
        "# 	Number y\n"
        "	 	 	 	 \n"
        "component Mdr = Number\n"
        "component Position\n"
        "	Number pos_x\n"
        "	Number pos_y\n"
        "	Number pos_z\n"
        "component Velocity\n"
        "	Number vel_x\n"
        "	Number vel_y\n"
        "	Number vel_z\n"
        "system	onTickPhysics( anyEntity<	Position, Velocity> ) event Tick(mdr<Position, \n"
        "Velocity>,lol<Velocity>, Number deltaTime )\n"
        "	# if anyEntity.Position.pos_x == 0\n"
        "	# 	print(anyEntity.Position.pos_x)\n"
        "	# 	anyEntity.Position.pos_x = 17 * 23.13 / 15 - 37.8\n"
        "	# 	print(anyEntity.Position.pos_x)\n"
        "	# 	anyEntity.Position.pos_x = 0\n"
        "	# 	anyEntity.Position.pos_x = 0\n"
        "	# isPressed(\"w\")\n"
        "	# print(\"Franchement\")\n"
        "	# print(anyEntity.Position.pos_x)\n"
        "	while anyEntity.Position.pos_x >= 0\n"
        "		print( \"During\", anyEntity.Position.pos_x )\n"
        "		anyEntity.Position.pos_x = anyEntity.Position.pos_x - 1\n"
        "	print(\"Done\")\n"
        "	# print(anyEntity.Position.pos_x)\n"
        "	# while anyEntity.Position.pos_y >= 0\n"
        "	# 	anyEntity.Position.pos_y = anyEntity.Position.pos_y - 1\n"
        "	# Number gravity = 9.98\n"
        "	# # appel(gravity, anyEntity.Position.pos_x)\n"
        "	# anyEntity.Position.pos_x=anyEntity.Position.pos_x+anyEntity.Velocity.vel_x# * deltaTime\n"
        "	# anyEntity.Position.pos_y = anyEntity.Position.pos_y + anyEntity.Velocity.vel_y# * deltaTime\n"
        "	# anyEntity.Position.pos_z = anyEntity.Position.pos_z + anyEntity.Velocity.vel_z# * deltaTime\n";
    component::Index cind;
    systems::Index sind;
    event::Index eind;
    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());
    engine.loadFile(file, true);

    REQUIRE(sind.getDescription("onTickPhysics").has_value());
    REQUIRE(cind.getDescription("Mdr").has_value());
    REQUIRE(cind.getDescription("Position").has_value());
    REQUIRE(cind.getDescription("Velocity").has_value());

    auto velCdescription = cind.getDescription("Velocity").value();
    auto posCdescription = cind.getDescription("Position").value();
    auto sysdescription = sind.getDescription("onTickPhysics").value();
    auto array1 = posCdescription.createContainer(posCdescription);
    auto array2 = velCdescription.createContainer(velCdescription);
    std::vector<std::vector<data::Value>> entities = {
        {// entity id 0 = entity1
         data::Record{{"pos_x", 0.0}, {"pos_y", 0.0}, {"pos_z", 0.0}},
         data::Record{{"vel_x", 1.0}, {"vel_y", -1.0}, {"vel_z", 0.0}}},
        {// entity id 1 = mdr
         data::Record{{"pos_x", 100.0}, {"pos_y", 100.0}, {"pos_z", 100.0}},
         data::Record{{"vel_x", 1.0}, {"vel_y", -1.0}, {"vel_z", 0.0}}},
        {// entity id 2 = lol
         data::Record{{"vel_x", 1.0}, {"vel_y", -1.0}, {"vel_z", 0.0}}}};
    size_t eId = 0;
    for (auto entity: entities) {
        array1->setValueForEntity(eId, entity.at(0));
        if (entity.size() > 1) array2->setValueForEntity(eId, entity.at(1));
        eId++;
    }
    component::ArrayCombination combinations{{std::ref(*array1), std::ref(*array2)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = sysdescription.eventListener, .entities = {1, 2}, .payload = 0.12},
        .components = {{// mdr<Position, Velocity> event parameter
                        component::ComponentRef(*array1, 1), component::ComponentRef(*array2, 1)},
                       {// lol<Velocity> event parameter
                        component::ComponentRef(*array2, 2)}}};

    // sysdescription.system(sysdescription, combinations, evt);

    std::cout << "------Engine------end" << std::endl;
}

TEST_CASE("Scripting-Refacto-Interpreter_One")
{
    std::cout << "------Interpreter------start" << std::endl;

    component::Index cind;
    systems::Index sind;
    event::Index eind;
    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());
    // std::string file = "../libscript/tests/tests/systems/interpreter/valid2.pvt";
    // engine.loadFile(file);
    std::string fileContent = "component C\n\tBoolean b\nsystem S(anyEntity<C>) event Tick(Number "
                              "deltaTime)\n\tprint(anyEntity.C.b)\n\tanyEntity.C.b = True\n\tprint(anyEntity.C.b)\n";
    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"b", false}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    Sdescription.system(Sdescription, combinations, evt);

    bool result = std::get<bool>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("b"));
    REQUIRE(result == true);
    std::cout << "Returned " << result << std::endl;

    std::cout << "------Interpreter------end" << std::endl;
}

TEST_CASE("Scripting-Event-Declaration")
{
    component::Index cind;
    systems::Index sind;
    event::Index eind;

    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());

    // std::string file = "C:/Users/jonme/eip/pivot/libscript/tests/b.pvt";
    // engine.loadFile(file);
    std::string fileContent = "event Kill\n"
                              "\tString monster\n"
                              "event Damage\n"
                              "\tNumber damage\n"
                              "event Move\n"
                              "\tNumber deltaTime\n"
                              "system killMonster() event Tick(Number deltaTime)\n"
                              "\temit Kill(\"frankenstein\")\n";
    engine.loadFile(fileContent, true);

    auto killDescription = eind.getDescription("Kill").value();
    REQUIRE(killDescription.name == "Kill");
    REQUIRE(killDescription.payload == data::Type{data::BasicType::String});
    REQUIRE(killDescription.payloadName == "monster");
    REQUIRE(killDescription.entities.size() == 0);
    REQUIRE(killDescription.provenance.isExternalRessource());

    REQUIRE(eind.getDescription("Damage").has_value());
    REQUIRE(eind.getDescription("Move").has_value());

    auto killMonsterSystem = sind.getDescription("killMonster").value();
    REQUIRE(killMonsterSystem.name == "killMonster");
    REQUIRE(killMonsterSystem.entityName == "");

    event::EventWithComponent event = {
        .event = event::Event{.description = killMonsterSystem.eventListener, .entities = {}, .payload = 0.12}};
    auto comb = component::ArrayCombination({});
    auto systemResult = killMonsterSystem.system(killMonsterSystem, comb, event);
    REQUIRE(systemResult.size() == 1);
    auto killEvent = systemResult.at(0);
    REQUIRE(killEvent.description.name == killDescription.name);
    REQUIRE(killEvent.entities.size() == 0);
    REQUIRE(killEvent.payload == data::Value{"frankenstein"});
}

TEST_CASE("Scripting-Interpreter-Vector")
{
    std::cout << "------Interpreter Vector------start" << std::endl;

    component::Index cind;
    systems::Index sind;
    event::Index eind;
    script::Engine engine(sind, cind, eind, pivot::ecs::script::interpreter::builtins::BuiltinContext());
    // std::string file = "C:/Users/Najo/eip/pivot/libscript/tests/vector.pvt";
    // engine.loadFile(file);
    std::string fileContent = "component C\n"
                              "\tVector3 vec\n"
                              "system S(anyEntity<C>) event Tick(Number deltaTime)\n"
                              "\tprint(anyEntity.C.vec)\n"
                              "\tanyEntity.C.vec.x = 5\n"
                              "\tanyEntity.C.vec.y = 2555\n"
                              "\tanyEntity.C.vec.z = 0\n"
                              "\tprint(anyEntity.C.vec)\n";
    engine.loadFile(fileContent, true);

    REQUIRE(sind.getDescription("S").has_value());
    REQUIRE(cind.getDescription("C").has_value());

    auto Cdescription = cind.getDescription("C").value();
    auto Sdescription = sind.getDescription("S").value();
    auto array1 = Cdescription.createContainer(Cdescription);
    std::vector<data::Value> entity = {data::Record{{"vec", glm::vec3{1.0, 2.0, 3.0}}}};
    array1->setValueForEntity(0, entity.at(0));
    component::ArrayCombination combinations{{std::ref(*array1)}};
    event::EventWithComponent evt = {
        .event = event::Event{.description = Sdescription.eventListener, .entities = {1, 2}, .payload = 0.12}};

    Sdescription.system(Sdescription, combinations, evt);

    REQUIRE(std::get<glm::vec3>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("vec")).x == 5);
    REQUIRE(std::get<glm::vec3>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("vec")).y == 2555);
    REQUIRE(std::get<glm::vec3>(std::get<data::Record>(array1->getValueForEntity(0).value()).at("vec")).z == 0);

    std::cout << "------Interpreter Vector------end" << std::endl;
}