#include "pivot/script/Engine.hxx"
#include "pivot/script/Exceptions.hxx"

#include "pivot/pivot.hxx"

#include <cpplogger/Logger.hpp>
#include <cstdlib>
#include <ctime>

namespace pivot::ecs::script
{

Engine::Engine(systems::Index &systemIndex, component::Index &componentIndex,
               interpreter::builtins::BuiltinContext context)
    : _systemIndex(systemIndex), _componentIndex(componentIndex), _interpreter(context)
{
    std::srand(std::time(nullptr));    // for built-in rand_int
}

std::string Engine::loadFile(const std::string &file, bool isContent, bool verbose)
{    // Load a file to register all descriptions in the file, as well as store the trees for the system entry points
    DEBUG_FUNCTION();

    if (verbose) logger.info("script::Engine") << ": Loading file " << file;
    Node fileNode;
    try {
        fileNode = _parser.ast_from_file(file, isContent, verbose);    // generate abstract syntax tree from file
    } catch (const InvalidIndentException &e) {
        logger.err("Invalid Indent: ") << e.what();
        return std::string("Invalid Indent: ") + e.what();    // return error for imGui
    } catch (const UnexpectedEOFException &e) {
        logger.err("Unepexpected EndOfFile: ") << e.what();
        return std::string("Unepexpected EndOfFile: ") + e.what();    // return error for imGui
    } catch (const UnexpectedTokenTypeException &e) {
        logger.err("Unexpected Token Type: ") << e.what();
        return std::string("Unexpected Token Type: ") + e.what();    // return error for imGui
    } catch (const UnexpectedTokenValueException &e) {
        logger.err("Unexpected Token Value: ") << e.what();
        return std::string("Unexpected Token Value: ") + e.what();    // return error for imGui
    } catch (const std::invalid_argument &e) {                        // logic error
        logger.err("LogicError: ") << e.what();
        return std::string("LogicError: ") + e.what();    // return error for imGui
    } catch (const std::exception &e) {
        // std::cerr << std::format("\nParser !Unhandled Exception!: {}", e.what()) << std::flush; // format not
        // available in c++20 gcc yet
        logger.err("\nscript::parser !Unhandled Exception!: ") << e.what();
        return std::string("\nscript::parser !Unhandled Exception!: ") + e.what();    // return error for imGui
    }

    if (verbose) parser::printFileNode(fileNode);

    // TODO: return string with error on top of the node
    std::vector<systems::Description> sysDescs = interpreter::registerDeclarations(
        fileNode, _componentIndex);    // register component descriptions, and retrieve system descriptions
    for (systems::Description &desc: sysDescs) {
        desc.system = std::bind(&Engine::systemCallback, this, std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3);    // add the callback to the descriptions
        _systemIndex.registerSystem(desc);                 // register system in the not-so-global index
        Node systemEntry = getEntryPointFor(
            desc.name, fileNode);    // only store the entry point for the system, the rest is in the description
        _systems.insert({desc.name, systemEntry});
    }
    return (file + " parsed succesfully.");    // no errors
}

std::vector<ecs::event::Event> Engine::systemCallback(const systems::Description &system,
                                                      component::ArrayCombination &entities,
                                                      event::EventWithComponent &trigger)
{
    PROFILE_FUNCTION();

    if (!_systems.contains(system.name)) {
        // std::cerr << std::format("Unregistered system '{}'", system.name) << std::endl; // format not available in
        // c++20 gcc yet
        logger.err("Unregistered system '") << system.name << "'";
        return {};
    }
    std::vector<ecs::event::Event> emittedEvents;
    const Node &systemEntry = _systems.at(system.name);    // Avoid looking up for every entity
    for (auto entity: entities) {                          // For every entity, execute the system with it as parameter
        try {
            _stack.clear();
            std::vector<ecs::event::Event> localEvents =
                _interpreter.executeSystem(systemEntry, system, entity, trigger, _stack);
            for (auto e: localEvents) emittedEvents.push_back(e);
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
    return emittedEvents;
}

// Private functions

Node Engine::getEntryPointFor(const std::string &systemName, const Node &file)
{    // Get the entry point for a system name in a file root tree node
    DEBUG_FUNCTION();
    for (const Node &declaration: file.children)
        if (declaration.type == NodeType::SystemDeclaration && declaration.value == systemName)
            for (const Node &child: declaration.children)
                if (child.type == NodeType::SystemEntryPoint) return child;
    // throw(std::exception(std::format("No entry point found for {}", systemName).c_str())); // format not available in
    // c++20 gcc yet
    throw std::runtime_error(("No entry point found for " + systemName).c_str());
}

}    // end of namespace pivot::ecs::script
