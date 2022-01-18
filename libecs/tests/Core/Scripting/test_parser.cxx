#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch.hpp>
#include <string>

TEST_CASE("parser") {
	ScriptEngine engine;
	LoadResult r = engine.loadFile("");
}



/*

Structure C qui permet de stocker un component

Description qui contient un any de ce type
et les fonctions pour le lire et le changer

push ces descriptions dans le global index

/*