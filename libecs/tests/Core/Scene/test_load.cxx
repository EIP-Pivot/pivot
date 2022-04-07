#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "pivot/ecs/Core/Scene.hxx"

using namespace nlohmann;
using namespace pivot::ecs::data;


TEST_CASE("Load the scene", "[Scene][Load]")
{
    json obj = json::parse("{\"components\": [{\"Gravity\": {\"force\": [0.0,0.0,0.0]},\"RigidBody\": {\"acceleration\": [0.0,0.0,0.0],\"velocity\": [0.0,0.0,0.0]},\"Tag\": {\"name\": \"yolo\"}}],\"name\": \"Default\"}");
   
    Scene LaS;
    LaS.load(obj);
    
    // auto &cManager = LaS.getComponentManager();
    // for (auto entities: obj["components"]) {
    // }
}