#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "pivot/ecs/Core/Scripting/Parser.hxx"
#include "pivot/ecs/Core/Scripting/Interpreter.hxx"

namespace pivot::ecs::script {

class Engine {
public:
	~Engine() = default;
	Engine(systems::Index &systemIndex, component::Index &componentIndex);

	///	Parse a file to generate the abstract tree from it
	///	Interpret the tree to register component and system declaration
	std::string loadFile(const std::string &fileName);

	///	This is the callback the SystemManager will call to execute a system on entities with a given event trigger
	void systemCallback(const systems::Description &system, component::ArrayCombination &entities, const event::EventWithComponent &trigger);



private:
	systems::Index &_systemIndex;
	component::Index &_componentIndex;

	std::map<std::string, Node> _systems;

	Node getEntryPointFor(const std::string &systemName, const Node &file);
};


} // end of namespace pivot::ecs::script