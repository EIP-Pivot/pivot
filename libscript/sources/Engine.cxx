#include "pivot/script/Engine.hxx"
#include "Logger.hpp"
#include "pivot/script/Exceptions.hxx"

namespace pivot::ecs::script
{

Engine::Engine(systems::Index &systemIndex, component::Index &componentIndex)
    : _systemIndex(systemIndex), _componentIndex(componentIndex)
{
}

std::string Engine::loadFile(const std::string &file, bool isContent, bool verbose)
{    // Load a file to register all descriptions in the file, as well as store the trees for the system entry points
    if (verbose) logger.info("script::Engine") << ": Loading file " << file;
    // TODO: return string with error on top of the node to deamImgui
    Node fileNode = _parser.ast_from_file(file, isContent, verbose);    // generate abstract syntax tree from file
    if (verbose) parser::printFileNode(fileNode);

    // TODO: return string with error on top of the node
    std::vector<systems::Description> sysDescs = interpreter::registerDeclarations(
        fileNode, _componentIndex);    // register component descriptions, and retrieve system descriptions
    for (systems::Description &desc: sysDescs) {
        desc.system = std::bind(&Engine::systemCallback, this, std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3);    // add the callback to the descriptions
        _systemIndex.registerSystem(desc);                 // register system in the not-so-global index
        try {
            Node systemEntry = getEntryPointFor(
                desc.name, fileNode);    // only store the entry point for the system, the rest is in the description
            _systems.insert({desc.name, systemEntry});
        } catch (const std::exception &e) {
            return e.what();    // unlikely branch, return error to dearImGui
        }
    }
    return (file + " parsed succesfully.");    // no errors
}

void Engine::systemCallback(const systems::Description &system, component::ArrayCombination &entities,
                            const event::EventWithComponent &trigger)
{
    if (!_systems.contains(system.name)) {
        // std::cerr << std::format("Unregistered system '{}'", system.name) << std::endl; // format not available in
        // c++20 gcc yet
        logger.err("Unregistered system '") << system.name << "'";
        return;
    }
    const Node &systemEntry = _systems.at(system.name);    // Avoid looking up for every entity
    for (auto entity: entities) {                          // For every entity, execute the system with it as parameter
        try {
            _stack.clear();
            interpreter::executeSystem(systemEntry, system, entity, trigger, _stack);
        } catch (const InvalidOperation &e) {
            logger.err("Invalid Operation: ") << e.what();
        } catch (const InvalidException &e) {
            logger.err("Error: ") << e.what();
        } catch (const std::invalid_argument &e) {    // logic error
            logger.err("LogicError: ") << e.what();
        } catch (const std::exception &e) {
            logger.err("Unhandled std exception: ") << e.what();
        }
    }
}

// Private functions

Node Engine::getEntryPointFor(const std::string &systemName, const Node &file)
{    // Get the entry point for a system name in a file root tree node
    for (const Node &declaration: file.children)
        if (declaration.type == NodeType::SystemDeclaration && declaration.value == systemName)
            for (const Node &child: declaration.children)
                if (child.type == NodeType::SystemEntryPoint) return child;
    // throw(std::exception(std::format("No entry point found for {}", systemName).c_str())); // format not available in
    // c++20 gcc yet
    throw std::runtime_error(("No entry point found for " + systemName).c_str());
}

}    // end of namespace pivot::ecs::script