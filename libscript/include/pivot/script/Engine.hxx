#pragma once

#include <string>

#include "pivot/script/Builtins.hxx"
#include "pivot/script/Interpreter.hxx"
#include "pivot/script/Parser.hxx"

namespace pivot::ecs::script
{

/*! \brief Script Engine handler to manage loaded scripts and execute them
 *
 *  It contains a Parser object to parse files into ASTs, and will
 * 	call the different methods from script::interpreter to anyalyze
 * 	these ASTs and generate descriptions or execute systems.
 */
class Engine
{
public:
    ~Engine() = default;
    /// An engine needs a reference to the indexes to register components and systems
    Engine(systems::Index &systemIndex, component::Index &componentIndex,
           interpreter::builtins::BuiltinContext context);

    ///	Parse a file to generate the abstract tree from it
    ///	Interpret the tree to register component and system declaration
    std::string loadFile(const std::string &file, bool isContent = false, bool verbose = false);

    ///	This is the callback the SystemManager will call to execute a system on entities with a given event trigger
    std::vector<ecs::event::Event> systemCallback(const systems::Description &system,
                                                  component::ArrayCombination &entities,
                                                  event::EventWithComponent &trigger);

private:
    systems::Index &_systemIndex;
    component::Index &_componentIndex;

    std::unordered_map<std::string, Node> _systems;
    interpreter::Stack _stack;
    parser::Parser _parser;
    interpreter::Interpreter _interpreter;

    // Find entry point (TODO: make system decl into a node to avoid searching)
    Node getEntryPointFor(const std::string &systemName, const Node &file);
};

}    // end of namespace pivot::ecs::script
