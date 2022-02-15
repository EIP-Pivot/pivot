#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch.hpp>
#include <vector>
#include <string>

TEST_CASE("Scripting-interpreter") {
	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
		// Commented test files are not supported yet
		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt",
		// 	"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
		// { "../libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt",
		// 	"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
		// { "../libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/jeu.pvt",	"" },
	};
	ScriptEngine engine;
	for (auto [file, output] : testFilesOutput) {
		LoadResult r = engine.loadFile(file, false);
		REQUIRE(output == r.output);
		engine.totalReset();
	}
}

TEST_CASE("Scripting-interpreter-main") {
	std::cout << "--------- Start Interpreter main --------" << std::endl;
	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/jeu.pvt",	"" },
	};
	ScriptEngine engine;
	for (auto [file, output] : testFilesOutput)
		LoadResult r = engine.loadFile(file, false);
    // auto description = pivot::ecs::component::GlobalIndex::getSingleton().getDescription("Velocity").value();
	// std::cout << "Position component in global index: " << description.name << std::endl;
	std::cout << "--------- End Interpreter main --------" << std::endl;
}


/*

Structure C qui permet de stocker un component

Description qui contient un any de ce type
et les fonctions pour le lire et le changer

push ces descriptions dans le global index

*/

