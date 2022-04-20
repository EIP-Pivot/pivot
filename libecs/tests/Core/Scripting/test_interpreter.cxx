#include "pivot/ecs/Core/Scripting/Engine.hxx"
// #include "pivot/ecs/Core/Scripting/Parser.hxx"
// #include "pivot/ecs/Core/Scripting/Interpreter.hxx"
// #include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>

using namespace pivot::ecs;

/*
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
	script::ScriptEngine engine;
	for (auto [file, output] : testFilesOutput) {
		script::LoadResult r = engine.loadFile(file, false);
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
	script::ScriptEngine engine;
	for (auto [file, output] : testFilesOutput)
		script::LoadResult r = engine.loadFile(file, true);
	auto velCdescription = component::GlobalIndex::getSingleton().getDescription("Velocity").value();
	auto posCdescription = component::GlobalIndex::getSingleton().getDescription("Position").value();
	auto sysdescription = systems::GlobalIndex::getSingleton().getDescription("onTickPhysics").value();
	auto array1 = posCdescription.createContainer(posCdescription);
	auto array2 = velCdescription.createContainer(velCdescription);
	std::vector<std::vector<data::Value>> entities = {
		{
			data::Record{ {"pos_x", 0.0},{"pos_y", 0.0},{"pos_z", 0.0}},
			data::Record{ {"vel_x", 1.0},{"vel_y", -1.0},{"vel_z", 0.0}}
		},
		// {
		// 	data::Record{ {"pos_x", "soleil"},{"pos_y", 43.5},{"pos_z", 345.0}},
		// 	data::Record{ {"vel_x", 789.0},{"vel_y", 456.0},{"vel_z", 123.0}}
		// }
	};
	// data::Value pos1{data::Record{ {"pos_x", "mdr"},{"pos_y", 0.5},{"pos_z", 34.0}}};
	// data::Value pos2{data::Record{ {"pos_x", "soleil"},{"pos_y", 43.5},{"pos_z", 345.0}}};
	// data::Value vel1{data::Record{ {"vel_x", 123.0},{"vel_y", 456.0},{"vel_z", 789.0}}};
	// data::Value vel2{data::Record{ {"vel_x", 789.0},{"vel_y", 456.0},{"vel_z", 123.0}}};
	// array1->setValueForEntity(0, pos1);
	// array1->setValueForEntity(1, pos2);
	// array2->setValueForEntity(0, vel1);
	// array2->setValueForEntity(1, vel2);

	size_t eId = 0;
	for (auto entity : entities) {
		array1->setValueForEntity(eId, entity.at(0));
		array2->setValueForEntity(eId, entity.at(1));
		eId++;
	}
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
	int numberOfTicks = 1;
	std::cout << "Entity 0\tTick 0" << "\n";
	std::cout << "\tPos_x: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_x"]) << std::endl;
	std::cout << "\tPos_y: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_y"]) << std::endl;
	std::cout << "\tPos_z: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_z"]) << std::endl;
	std::chrono::high_resolution_clock clock;
	auto start = clock.now();
	for (int i = 1; i <= numberOfTicks; i++) {
		sysdescription.system(sysdescription, combinations, e);
		std::cout << "Entity 0\tTick " << i << "\n";
		std::cout << "\tPos_x: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_x"]) << std::endl;
		std::cout << "\tPos_y: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_y"]) << std::endl;
		std::cout << "\tPos_z: " << std::get<double>(std::get<data::Record>(array1->getValueForEntity(0).value())["pos_z"]) << std::endl;
	}
	long long totalMs = std::chrono::duration(clock.now() - start).count() / 1000000;
	std::cout << "Time to execute " << numberOfTicks << " ticks:\t" << totalMs << " ms" << std::endl;
	std::cout << "Time to execute computeExpression:\t\t" << engine._totalCompute / 1000000 << "ms\t" << (double(engine._totalCompute / 1000000) / totalMs) * 100 << "%" << std::endl;
	std::cout << "Time to execute std::visit and ReplaceAll:\t" << engine._totalReplaceall / 1000000 << "ms\t" << (double(engine._totalReplaceall / 1000000) / totalMs) * 100 << "%" << std::endl;
	std::cout << "Time to execute parser.compile:\t\t\t" << engine._totalCompile / 1000000 << "ms\t" << (double(engine._totalCompile / 1000000) / totalMs) * 100 << "%" << std::endl;
	std::cout << "--------- End Interpreter main --------" << std::endl;
}
*/

TEST_CASE("Scripting-Refacto-HelperFunctions") {
	// bool script::parser::line_is_empty(const std::string &line)
	REQUIRE(script::parser::line_is_empty("") == true);
	REQUIRE(script::parser::line_is_empty("    ") == true);
	REQUIRE(script::parser::line_is_empty("\t\t\t") == true);
	REQUIRE(script::parser::line_is_empty(" \t \t") == true);
	REQUIRE(script::parser::line_is_empty(" \t \taze") == false);
	REQUIRE(script::parser::line_is_empty("a") == false);

	// std::string script::parser::remove_comments(const std::string &line)
	REQUIRE(script::parser::remove_comments("# mdr") == "");
	REQUIRE(script::parser::remove_comments(" # mdr") == " ");
	REQUIRE(script::parser::remove_comments("mdr") == "mdr");
	REQUIRE(script::parser::remove_comments("mdr # lol") == "mdr ");

	// IndentType script::parser::indent_type_of_char(char c)
	REQUIRE(script::parser::indent_type_of_char(' ') == script::IndentType::Spaces);
	REQUIRE(script::parser::indent_type_of_char('\t') == script::IndentType::Tabs);
	REQUIRE(script::parser::indent_type_of_char('a') == script::IndentType::NoIndent);

	// IndentType script::parser::indent_type_of_line(const std::string &fileLine)
	REQUIRE(script::parser::indent_type_of_line("    mdr") == script::IndentType::Spaces);
	REQUIRE(script::parser::indent_type_of_line("\t\t\tmdr") == script::IndentType::Tabs);
	REQUIRE(script::parser::indent_type_of_line("mdr") == script::IndentType::NoIndent);
	REQUIRE(script::parser::indent_type_of_line(" \t \tmdr") == script::IndentType::Invalid);

	// IndentType script::parser::indent_type_of(const std::string &fileString)
	REQUIRE(script::parser::indent_type_of("\t \t \n    mdr") == script::IndentType::Spaces);
	REQUIRE(script::parser::indent_type_of("  mdr\n\tmdr") == script::IndentType::Spaces);
	REQUIRE(script::parser::indent_type_of("mdr\n   mdr") == script::IndentType::Spaces);
	REQUIRE(script::parser::indent_type_of("\tmdr\n   mdr") == script::IndentType::Tabs);
	REQUIRE(script::parser::indent_type_of("mdr\n\t\tmdr") == script::IndentType::Tabs);
	REQUIRE(script::parser::indent_type_of("mdr\nmdr") == script::IndentType::NoIndent);
	REQUIRE(script::parser::indent_type_of("mdr\n") == script::IndentType::NoIndent);
	REQUIRE(script::parser::indent_type_of("mdr") == script::IndentType::NoIndent);
	REQUIRE(script::parser::indent_type_of("") == script::IndentType::NoIndent);

	// size_t script::parser::indent_size_of_line(const std::string &fileLine)
	REQUIRE(script::parser::indent_size_of_line("\t\tNumber x\n") == 2);
	REQUIRE(script::parser::indent_size_of_line("\tNumber x\n") == 1);
	REQUIRE(script::parser::indent_size_of_line("    Number x\n") == 4);
	REQUIRE(script::parser::indent_size_of_line("Number x\n") == 0);

	// size_t script::parser::indent_size_of(const std::string &fileString)
	REQUIRE(script::parser::indent_size_of("\t \t \n    mdr") == 4);
	REQUIRE(script::parser::indent_size_of("\t\tNumber x\n") == 2);
	REQUIRE(script::parser::indent_size_of("\tNumber x\n") == 1);
	REQUIRE(script::parser::indent_size_of("    Number x\n") == 4);
	REQUIRE(script::parser::indent_size_of("Number x\n") == 0);
	REQUIRE(script::parser::indent_size_of("Number x\n   mdr") == 3);
	REQUIRE(script::parser::indent_size_of("Number x\n\t\tmdr") == 2);

}

TEST_CASE("Scripting-Refacto-Lexer") {
	// std::vector<Token> script::parser::tokens_from_file(const std::string &fileName)
	// Mapping file names (containing PivotScript) to the expected list of tokens the lexer should extract
	std::map<std::string, std::vector<script::Token>> _expectedTokenlists = {
		{ "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/lexer/OneLineComponent.pvt",
			{	script::Token{.type=script::TokenType::Identifier,.value="component",.line_nb=1,.char_nb=1},
				script::Token{.type=script::TokenType::Identifier,.value="Test",.line_nb=1,.char_nb=11},
				script::Token{.type=script::TokenType::Symbol,.value="=",.line_nb=1,.char_nb=16},
				script::Token{.type=script::TokenType::Identifier,.value="Number",.line_nb=1,.char_nb=18}		}
		}
	};
	for (auto &[fileName, expectedTokens] : _expectedTokenlists) {
		// TODO : find elegant way to overload== for vectors (which isn't possible as of c++20)
		std::vector<script::Token> tokens = script::parser::tokens_from_file(fileName);
		REQUIRE(tokens.size() == expectedTokens.size());
		for (size_t i = 0; i < tokens.size(); i++)
			REQUIRE(tokens.at(i) == expectedTokens.at(i));
	}
}

TEST_CASE("Scripting-Refacto-Engine") {
	std::cout << "------Engine------start" << std::endl;

	std::string file = "C:/Users/jonme/eip/pivot/libecs/tests/Core/Scripting/tests/physics.pvt";
	component::Index cind;
	systems::Index sind;
	script::Engine engine(sind, cind);
	engine.loadFile(file);

	std::cout << "------Engine------end" << std::endl;
}
