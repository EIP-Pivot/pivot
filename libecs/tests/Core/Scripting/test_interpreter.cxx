#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch.hpp>
#include <vector>
#include <string>

using namespace pivot::ecs;

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
	auto velCdescription = component::GlobalIndex::getSingleton().getDescription("Velocity").value();
	auto posCdescription = component::GlobalIndex::getSingleton().getDescription("Position").value();
	auto sysdescription = systems::GlobalIndex::getSingleton().getDescription("onTickPhysics").value();
	auto array1 = posCdescription.createContainer(posCdescription);
	auto array2 = velCdescription.createContainer(velCdescription);
	std::cout << "--------- End Interpreter main --------" << std::endl;
	data::Value pos1{data::Record{ {"pos_x", "mdr"},{"pos_y", 0.5},{"pos_z", 34.0}}};
	data::Value pos2{data::Record{ {"pos_x", "soleil"},{"pos_y", 43.5},{"pos_z", 345.0}}};
	data::Value vel1{data::Record{ {"vel_x", 123.0},{"vel_y", 456.0},{"vel_z", 789.0}}};
	data::Value vel2{data::Record{ {"vel_x", 789.0},{"vel_y", 456.0},{"vel_z", 123.0}}};

	array1->setValueForEntity(0, pos1);
	array1->setValueForEntity(1, pos2);
	array2->setValueForEntity(0, vel1);
	array2->setValueForEntity(1, vel2);
	component::ArrayCombination combinations{{std::ref(*array1), std::ref(*array2)}};

	// for (auto combination: combinations) {
	// 	std::cout << std::get<std::string>(std::get<data::Record>(combination[0].get())["name"]) << std::endl;
	// 	std::cout << std::get<std::string>(std::get<data::Record>(combination[1].get())["name"]) << std::endl;
	// }
	event::Event e =  {
		.description = sysdescription.eventListener,
		.entities = {},
		.payload = 0.12
	};
	sysdescription.system(sysdescription, combinations, e);
}


/*

Structure C qui permet de stocker un component

Description qui contient un any de ce type
et les fonctions pour le lire et le changer

push ces descriptions dans le global index

*/

