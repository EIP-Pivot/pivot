#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch.hpp>
#include <vector>
#include <string>

TEST_CASE("Scripting-parser") {
	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
		// Commented test files are not supported yet
		// { "../libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
		{ "../libecs/tests/Core/Scripting/tests/invalid/component/invalid_component1.pvt",
			"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/component/invalid_component1.pvt\n\tline 1:\tTransform \nLogicError: Unknown expression.\n" },
		{ "../libecs/tests/Core/Scripting/tests/invalid/component/invalid_component2.pvt",
			"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/component/invalid_component2.pvt\n\tline 1:\tcomponent Transform { \nSyntaxError: Invalid component declaration. Try declaring it like this : 'component Name'.\n" },
		{ "../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property1.pvt",
			"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property1.pvt\n\tline 2:\t    pos Vector3\nLogicError: Unknown expression.\n" },
		// { "../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt",
		// 	"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
		// { "../libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt",
		// 	"\nERROR:\t../libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
		{ "../libecs/tests/Core/Scripting/tests/valid/component/valid_component.pvt", "../libecs/tests/Core/Scripting/tests/valid/component/valid_component.pvt succesfully parsed.\n" },
		{ "../libecs/tests/Core/Scripting/tests/valid/property/valid_property.pvt",	"../libecs/tests/Core/Scripting/tests/valid/property/valid_property.pvt succesfully parsed.\n" },
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

TEST_CASE("Scripting-parser-main") {
	std::cout << "--------- Start Parser main --------" << std::endl;
	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
		// { "../libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/jeu.pvt",	"" },
	};
	ScriptEngine engine;
	for (auto [file, output] : testFilesOutput) {
		LoadResult r = engine.loadFile(file, false);
		std::cout << "[OUTPUT START]\tfor " << file << std::endl;
		std::cout << r.output << std::endl;
		std::cout << "[OUTPUT END]" << std::endl;
		engine.totalReset();
	}
	std::cout << "--------- End Parser main --------" << std::endl;
}


/*

Structure C qui permet de stocker un component

Description qui contient un any de ce type
et les fonctions pour le lire et le changer

push ces descriptions dans le global index

*/

