#include "Logger.hpp"
#include "pivot/ecs/Core/Scripting/Engine.hxx"

namespace pivot::ecs::script {

Engine::Engine(systems::Index &systemIndex, component::Index &componentIndex)
:	_systemIndex(systemIndex), _componentIndex(componentIndex)
{
}

std::string Engine::loadFile(const std::string &fileName) { // Load a file to register all descriptions in the file, as well as store the trees for the system entry points

	std::cout << "Loading file " << fileName << std::endl; // debug
	Node file = parser::ast_from_file(fileName, false); // generate abstract syntax tree from file
	parser::printFileNode(file); // debug

	std::vector<systems::Description> sysDescs = interpreter::registerDeclarations(file, _componentIndex); // register component descriptions, and retrieve system descriptions
	for (systems::Description &desc: sysDescs) {
		desc.system = std::bind(&Engine::systemCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); // add the callback to the descriptions
		_systemIndex.registerSystem(desc); // register system in the not-so-global index
		try {
			Node systemEntry = getEntryPointFor(desc.name, file);  // only store the entry point for the system, the rest is in the description
			_systems.insert({desc.name, systemEntry});
		} catch (std::exception e) {
			std::cerr << e.what() << std::endl;
		}
	}
	return ""; // no errors
}

void Engine::systemCallback(const systems::Description &system, component::ArrayCombination &entities, const event::EventWithComponent &trigger) {
	if (!_systems.contains(system.name)) {
		// std::cerr << std::format("Unregistered system '{}'", system.name) << std::endl; // format not available in c++20 gcc yet
		logger.err("Unregistered system '") << system.name << "'";
		return ;
	}

}


// Private functions

Node Engine::getEntryPointFor(const std::string &systemName, const Node &file) { // Get the entry point for a system name in a file root tree node
	for (const Node &declaration : file.children)
		if (declaration.type == NodeType::SystemDeclaration && declaration.value == systemName)
			for (const Node &child : declaration.children)
				if (child.type == NodeType::SystemEntryPoint)
					return child;
	throw(std::exception(std::format("No entry point found for {}", systemName).c_str())); // format not available in c++20 gcc yet
}


} // end of namespace pivot::ecs::script