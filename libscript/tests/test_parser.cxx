#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <magic_enum.hpp>
#include <string>
#include <vector>

#include "pivot/script/Parser.hxx"

using namespace pivot::ecs;
using namespace pivot::ecs::script;

TEST_CASE("Scripting-Refacto-HelperFunctions")
{
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

TEST_CASE("Scripting-Refacto-Lexer")
{
    script::parser::Parser p;
    // std::vector<Token> script::parser::tokens_from_file(const std::string &fileName)
    // Mapping file names (containing PivotScript) to the expected list of tokens the lexer should extract
    std::map<std::string, std::vector<script::Token>> _expectedTokenlists = {
        {"component Test = Number\n",
         {script::Token{.type = script::TokenType::Identifier, .value = "component", .line_nb = 1, .char_nb = 1},
          script::Token{.type = script::TokenType::Identifier, .value = "Test", .line_nb = 1, .char_nb = 11},
          script::Token{.type = script::TokenType::Symbol, .value = "=", .line_nb = 1, .char_nb = 16},
          script::Token{.type = script::TokenType::Identifier, .value = "Number", .line_nb = 1, .char_nb = 18}}}};
    for (auto &[fileContent, expectedTokens]: _expectedTokenlists) {
        // TODO : find elegant way to overload== for vectors (which isn't possible as of c++20)
        p.tokens_from_file(fileContent, true);
        std::queue<script::Token> tokens = p.getTokenQueue();
        REQUIRE(tokens.size() == expectedTokens.size());
        for (size_t i = 0; i < tokens.size(); i++) {
            REQUIRE(tokens.front() == expectedTokens.at(i));
            tokens.pop();
        }
    }
}

TEST_CASE("Scripting-Parser-ValidFiles")
{
    // std::vector<std::string> fileNames = {
    //     "../libscript/tests/tests/components/parser/valid1.pvt",
    //     "../libscript/tests/tests/components/parser/valid2.pvt",
    //     "../libscript/tests/tests/components/parser/valid3.pvt",
    // };
    std::vector<std::string> fileContents = {
        "component id\n\tNumber age\n\tString name\n",
        "component id\n\tNumber age\n\tString name\n\nsystem test(e<id>) event Tick(Number deltaTime)\n\tNumber mdr = "
        "e.age\n\tString aze = name\n\taze = \"mdr\"\n",
        "component id\n\tBoolean isHuman\n\nsystem test(e<id>) event Tick(Number deltaTime)\n\tBoolean mdr = "
        "e.isHuman\n\tBoolean test = True\n\ttest = mdr\n\tif test\n\t\tprint(\"test\")\n",
    };
    script::parser::Parser parser;
    // for (const std::string &file: fileNames) { REQUIRE_NOTHROW(parser.ast_from_file(file, false, false)); }
    for (const std::string &file: fileContents) { REQUIRE_NOTHROW(parser.ast_from_file(file, true, false)); }
}

// TEST_CASE("Scripting-Parser-InvalidFiles")
// {
//     std::cout << "------Parser/Invalidfiles------start" << std::endl;
//     std::vector<std::string> fileNames = {
//         "../libscript/tests/tests/components/parser/invalid1.pvt",
//     };
//     script::parser::Parser parser;
//     for (const std::string &file: fileNames) { REQUIRE_THROWS(parser.ast_from_file(file, false, false)); }
//     std::cout << "------Parser/Invalidfiles------end" << std::endl;
// }

TEST_CASE("Parse emit event", "[scripting]")
{
    const auto script = "system test() event Tick()\n\temit TestEvent(\"value\")\n";
    parser::Parser parser;
    Node file;
    REQUIRE_NOTHROW(file = parser.ast_from_file(script, true, true));
    Node system = file.children.at(0);
    Node statements = system.children.at(6);
    Node emitStatement = statements.children.at(0);
    REQUIRE(emitStatement.value == "emit");
    Node emitEvent = emitStatement.children.at(0);
    REQUIRE(emitEvent.type == NodeType::EmitEvent);
    REQUIRE(emitEvent.value == "TestEvent");
    REQUIRE(emitEvent.children.size() == 1);
    Node eventArg = emitEvent.children.at(0);
    REQUIRE(eventArg.type == NodeType::FunctionParams);
}