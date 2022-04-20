// #include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>

// TEST_CASE("Scripting-parser") {
// 	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
// 		// Commented test files are not supported yet
// 		// TODO : replace 'C:/Users/jonme/eip/pivot' by '..'
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
// 		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/component/invalid_component1.pvt",
// 			"\nERROR:\tC:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/component/invalid_component1.pvt\n\tline 1:\tTransform \nLogicError: Unknown expression.\n" },
// 		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/component/invalid_component2.pvt",
// 			"\nERROR:\tC:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/component/invalid_component2.pvt\n\tline 1:\tcomponent Transform { \nSyntaxError: Invalid component declaration. Try declaring it like this : 'component Name'.\n" },
// 		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/property/invalid_property1.pvt",
// 			"\nERROR:\tC:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/property/invalid_property1.pvt\n\tline 2:\t    pos Vector3\nLogicError: Unknown expression.\n" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt",
// 		// 	"\nERROR:\tC:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/property/invalid_property2.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt",
// 		// 	"\nERROR:\tC:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/invalid/variable/invalid_variable1.pvt\n\tline 2:\tpos Vector3\nLogicError: Unknown expression.\n" },
// 		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/component/valid_component.pvt", "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/component/valid_component.pvt succesfully parsed.\n" },
// 		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/property/valid_property.pvt",	"C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/property/valid_property.pvt succesfully parsed.\n" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/jeu.pvt",	"" },
// 	};
// 	script::ScriptEngine engine;
// 	for (auto [file, output] : testFilesOutput) {
// 		script::LoadResult r = engine.loadFile(file, false);
// 		REQUIRE(output == r.output);
// 		engine.totalReset();
// 	}
// }

// TEST_CASE("Scripting-parser-main") {
// 	// std::cout << "--------- Start Parser main --------" << std::endl;
// 	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/physics.pvt",	"" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
// 		// { "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/jeu.pvt",	"" },
// 	};

// 	script::ScriptEngine engine;
// 	for (auto [file, output] : testFilesOutput) {
// 		script::LoadResult r = engine.loadFile(file, true);
// 		engine.totalReset();
// 	}
// 	// std::cout << "--------- End Parser main --------" << std::endl;
// }

