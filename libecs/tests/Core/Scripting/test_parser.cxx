#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch.hpp>
#include <vector>
#include <string>

TEST_CASE("parser") {
	std::cout << "--------- Start Parser tests --------" << std::endl;
	std::vector<std::pair<std::string, std::string>> testFilesOutput = {
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
		{ "../libecs/tests/Core/Scripting/tests/valid/component/valid_component.pvt", "" },
		{ "../libecs/tests/Core/Scripting/tests/valid/property/valid_property.pvt",	"" },
		// { "../libecs/tests/Core/Scripting/tests/valid/variable/valid_variable.pvt",	"" },
	};
	ScriptEngine engine;
	for (auto [file, output] : testFilesOutput) {
		LoadResult r = engine.loadFile(file, false);
		if (output != r.output) {
			std::cout << "--------------------------------" << std::endl;
			std::cout << output << std::endl;
			std::cout << "--------------------------------" << std::endl;
			std::cout << r.output << std::endl;
			std::cout << "--------------------------------" << std::endl;
			std::cout << "Test " << file.substr(file.find_last_of('/') + 1, file.size() - file.find_last_of('/') - 1) <<  " failed" << std::endl;
		}
		engine.totalReset();
	}
	std::cout << "--------- End Parser tests --------" << std::endl;
}



/*

Structure C qui permet de stocker un component

Description qui contient un any de ce type
et les fonctions pour le lire et le changer

push ces descriptions dans le global index
*/

