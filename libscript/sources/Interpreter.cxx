#include "pivot/script/Interpreter.hxx"
#include "pivot/script/Builtins.hxx"
#include "pivot/script/Exceptions.hxx"
#include <cpplogger/Logger.hpp>
#include <limits>
#include <magic_enum.hpp>
#include <unordered_map>

#include "pivot/pivot.hxx"

namespace pivot::ecs::script::interpreter
{

const std::map<std::string, data::BasicType> gVariableTypes{
    {"Vector3", data::BasicType::Vec3},  {"Vector2", data::BasicType::Vec2},    {"Asset", data::BasicType::Asset},
    {"Number", data::BasicType::Number}, {"Boolean", data::BasicType::Boolean}, {"Color", data::BasicType::Color},
    {"String", data::BasicType::String}};
// Map builtin binary (two operands) operators, to their operator enum
const std::map<std::string, std::function<data::Value(const data::Value &, const data::Value &)>> gOperatorCallbacks = {
    {"*", interpreter::builtins::builtin_operator<builtins::Operator::Multiplication>},
    {"/", interpreter::builtins::builtin_operator<builtins::Operator::Divison>},
    {"%", interpreter::builtins::builtin_operator<builtins::Operator::Modulo>},
    {"+", interpreter::builtins::builtin_operator<builtins::Operator::Addition>},
    {"-", interpreter::builtins::builtin_operator<builtins::Operator::Substraction>},
    {"<", interpreter::builtins::builtin_operator<builtins::Operator::LowerThan>},
    {"<=", interpreter::builtins::builtin_operator<builtins::Operator::LowerOrEqual>},
    {">", interpreter::builtins::builtin_operator<builtins::Operator::GreaterThan>},
    {">=", interpreter::builtins::builtin_operator<builtins::Operator::GreaterOrEqual>},
    // {"<", interpreter::builtins::generic_builtin_comparator(std::less<void>{}, "<")},
    // {"<=", interpreter::builtins::generic_builtin_comparator(std::less_equal<void>{}, "<=")},
    // {">", interpreter::builtins::generic_builtin_comparator(std::greater<void>{}, ">")},
    // {">=", interpreter::builtins::generic_builtin_comparator(std::greater_equal<void>{}, ">=")},
    {"==", interpreter::builtins::generic_builtin_comparator(std::equal_to<void>{}, "==")},
    {"!=", interpreter::builtins::generic_builtin_comparator(std::not_equal_to<void>{}, "!=")},
    {"&&", interpreter::builtins::generic_builtin_comparator(std::logical_and<void>{}, "&&")},
    {"||", interpreter::builtins::generic_builtin_comparator(std::logical_or<void>{}, "||")}};

/// Builtins are callbacks taking values as parameters and returning a single value
using BuiltinFunctionCallback =
    std::function<data::Value(const std::vector<data::Value> &, const builtins::BuiltinContext &)>;
/// The signature can be represented as a number of parameters, and to each their possible types
using ParameterPair = std::pair<size_t, std::vector<std::vector<data::Type>>>;

/// This map will map the name of a builtin, to its callback paired with its signature
const std::unordered_map<std::string, std::pair<BuiltinFunctionCallback, ParameterPair>> gBuiltinsCallbacks = {
    {"isPressed", {interpreter::builtins::builtin_isPressed, {1, {{data::BasicType::String}}}}},
    {"selectCamera", {interpreter::builtins::builtin_selectCamera, {1, {{data::BasicType::EntityRef}}}}},
    {"cos", {interpreter::builtins::builtin_cos, {1, {{data::BasicType::Number}}}}},
    {"sin", {interpreter::builtins::builtin_sin, {1, {{data::BasicType::Number}}}}},
    {"toString",
     {interpreter::builtins::builtin_toString,
      {std::numeric_limits<size_t>::max(),
       {{data::BasicType::String, data::BasicType::Number, data::BasicType::Integer, data::BasicType::Boolean,
         data::BasicType::Asset, data::BasicType::Vec3, data::BasicType::Vec2, data::BasicType::Color}}}}},
    {"print",
     {interpreter::builtins::builtin_print,
      {std::numeric_limits<size_t>::max(),
       {{data::BasicType::String, data::BasicType::Number, data::BasicType::Integer, data::BasicType::Boolean,
         data::BasicType::Asset, data::BasicType::Vec3, data::BasicType::EntityRef}}}}},
    {"randint", {interpreter::builtins::builtin_randint, {1, {{data::BasicType::Number}}}}},
    {"pow", {interpreter::builtins::builtin_power, {2, {{data::BasicType::Number}, {data::BasicType::Number}}}}},
    {"sqrt", {interpreter::builtins::builtin_sqrt, {1, {{data::BasicType::Number}}}}},
    {"abs", {interpreter::builtins::builtin_abs, {1, {{data::BasicType::Number}}}}},
    {"vec3",
     {interpreter::builtins::builtin_vec3,
      {3, {{data::BasicType::Number}, {data::BasicType::Number}, {data::BasicType::Number}}}}},
    {"color",
     {interpreter::builtins::builtin_color,
      {4,
       {{data::BasicType::Number}, {data::BasicType::Number}, {data::BasicType::Number}, {data::BasicType::Number}}}}}};

// Public functions ( can be called anywhere )

// This will go through a file's tree and register all component/system declarations into the global index
std::vector<systems::Description> registerDeclarations(const Node &file, component::Index &componentIndex,
                                                       event::Index &eventIndex)
{
    DEBUG_FUNCTION();
    std::vector<systems::Description> result;
    if (file.type != NodeType::File) {
        // std::cerr << std::format("registerDeclarations(const Node &file): can't interpret node {} (not a file)",
        // magic_enum::enum_name(file.type) << std::endl; // format not available in c++20 gcc yet
        logger.warn("registerDeclarations(const Node &file): can't interpret node ")
            << magic_enum::enum_name(file.type) << " (not a file)";
        return result;
    }
    try {    // handle exceptions from register functions
        // Loop through every declaration in the file
        for (const Node &node: file.children) {
            if (node.type == NodeType::ComponentDeclaration) {    // register component
                registerComponentDeclaration(node, componentIndex, file.value);
            } else if (node.type == NodeType::EventDeclaration) {    // register event
                registerEventDeclaration(node, eventIndex, file.value);
            } else if (node.type == NodeType::SystemDeclaration) {    // store system declaration for return
                systems::Description r = registerSystemDeclaration(node, file.value);
                if (r.name == "Error 1") {
                    // std::cerr << std::format("registerDeclarations(const Node &file): failed to interpret sub node
                    // {}", magic_enum::enum_name(node.type)) << std::endl; // format not available in c++20 gcc yet
                    logger.err("registerDeclarations(const Node &file): failed to register declarations for sub node ")
                        << magic_enum::enum_name(node.type);
                    return result;
                }
                result.push_back(r);
            } else {    // ??
                // std::cerr << std::format("registerDeclarations(const Node &file): can't interpret sub node {} (not a
                // component nor a system)", magic_enum::enum_name(node.type)) << std::endl; // format not available in
                // c++20 gcc yet
                logger.err("Register Declarations:")
                    << "registerDeclarations(const Node &file): can't register declarations for sub node "
                    << magic_enum::enum_name(node.type) << " (not a component nor a system)";
                return result;
            }
        }
    } catch (const UnexpectedEOFException &e) {
        logger.err("Unepexpected EndOfFile: ") << e.what();
    } catch (const UnexpectedNodeTypeException &e) {
        logger.err("Unepexpected Node Type: ") << e.what();
    } catch (const UnexpectedNodeValueException &e) {
        logger.err("Unepexpected Node Value: ") << e.what();
    } catch (const UnknownTypeException &e) {
        logger.err("Unknown Type: ") << e.what();
    } catch (const std::invalid_argument &e) {    // logic error
        logger.err("LogicError: ") << e.what();
    } catch (const std::exception &e) {
        // std::cerr << std::format("Unhandled exception in file {}: {}", file.value, e.what()) << std::endl; // format
        // not available in c++20 gcc yet
        logger.err("Unhandled exception in file ") << file.value << ": " << e.what();
    }
    return result;
}

// This will execute a SystemEntryPoint node by executing all of its statements
void Interpreter::executeSystem(const Node &systemEntry, const systems::Description &desc,
                                component::ArrayCombination::ComponentCombination &entityComponentCombination,
                                event::EventWithComponent &trigger, Stack &stack)
{
    PROFILE_FUNCTION();
    // systemComponents : [ "Position", "Velocity" ]
    // entity : [ PositionRecord, VelocityRecord ]
    logger.trace() << "Executing block " << systemEntry.value;
    auto entityComponents = entityComponentCombination.getAllComponents();
    // Push input entity to stack
    stack.pushEntity(desc.entityName, entityComponentCombination.entity, entityComponents);
    // Push event entities to the stack
    for (std::size_t i = 0; i < desc.eventListener.entities.size(); i++) {
        stack.pushEntity(desc.eventListener.entities[i], trigger.event.entities[i], trigger.components[i]);
    }
    // Push payload to stack
    if (!trigger.event.description.payloadName.empty()) {
        stack.push(trigger.event.description.payloadName, trigger.event.payload);
    }

    for (const Node &statement: systemEntry.children) {    // execute all statements
        executeStatement(statement, stack);
    }

    stack.updateEntity(desc.entityName, entityComponents);
    for (std::size_t i = 0; i < desc.eventListener.entities.size(); i++) {
        stack.updateEntity(desc.eventListener.entities[i], trigger.components[i]);
    }
}

// Private functions (never called elsewhere than this file and tests)

// Execute a statement (used for recursion for blocks)
void Interpreter::executeStatement(const Node &statement, Stack &stack)
{
    PROFILE_FUNCTION();
    if (statement.value == "functionCall") {
        executeFunction(statement, stack);
    } else if (statement.value == "if") {
        if (statement.children.size() < 2 || statement.children.at(0).type != NodeType::Expression) {
            logger.err("ERROR") << " at node " << statement.value;
            throw InvalidException("Invalid If Statement: Expected if condition expression.");
        }
        data::Value exprResult = evaluateExpression(statement.children.at(0), stack);
        try {    // check that expression resulted in data::BasicType::Boolean specifically
            if (std::get<data::BasicType>(exprResult.type()) != data::BasicType::Boolean) {
                logger.err("ERROR") << " at node " << exprResult.type().toString();
                throw InvalidException(
                    "Invalid If Condition: If condition should result in data::BasicType::Boolean instead.");
            }
            if (std::get<bool>(exprResult)) {    // if the condition is true, execute the block statements
                for (size_t statementIndex = 1; statementIndex < statement.children.size(); statementIndex++)
                    executeStatement(statement.children.at(statementIndex), stack);
            }    // else ignore the block
        } catch (const std::bad_variant_access &) {
            logger.err("ERROR") << " at node " << exprResult.type().toString();
            throw InvalidException("Invalid If Condition: If condition should result in data::BasicType instead.");
        }
    } else if (statement.value == "while") {
        if (statement.children.size() < 2 || statement.children.at(0).type != NodeType::Expression) {
            logger.err("ERROR") << " at node " << statement.value;
            throw InvalidException("Invalid While Statement: Expected while condition expression.");
        }
        data::Value exprResult = evaluateExpression(statement.children.at(0), stack);
        try {    // check that expression resulted in data::BasicType::Boolean specifically
            if (std::get<data::BasicType>(exprResult.type()) != data::BasicType::Boolean) {
                logger.err("ERROR") << " at node " << exprResult.type().toString();
                throw InvalidException(
                    "Invalid While Condition: While condition should result in data::BasicType::Boolean instead.");
            }
            size_t infinitePrevent = 0;
            while (std::get<bool>(exprResult) &&
                   infinitePrevent < 1000) {    // while the condition is true, execute the block statements
                for (size_t statementIndex = 1; statementIndex < statement.children.size();
                     statementIndex++)    // execute all statements
                    executeStatement(statement.children.at(statementIndex), stack);
                exprResult = evaluateExpression(statement.children.at(0), stack);    // re-check while condition
                infinitePrevent++;
            }    // when the condition is not true anymore, leave the block
            if (infinitePrevent == 1000)
                logger.warn("InfiniteLoop")
                    << "while loop ran for more than 1000 times. This may be the result of an infinite loop.";
        } catch (const std::bad_variant_access &) {
            logger.err("ERROR") << " at node " << exprResult.type().toString();
            throw InvalidException(
                "Invalid While Condition: While condition should result in data::BasicType instead.");
        }
    } else if (statement.value == "assign") {

        if (statement.children.size() != 2 || statement.children.at(1).type != NodeType::Expression) {
            logger.err("ERROR") << " at node " << statement.children.at(1).value;
            throw InvalidException("Invalid Assign Statement: Expected assign expression.");
        }

        if (statement.children.at(0).type == NodeType::NewVariable) {    // assign to a new variable
            const std::string &newVarType =
                statement.children.at(0).children.at(0).value;    // validated by script::parser
            const std::string &newVarName = statement.children.at(0).children.at(1).value;
            data::Value exprResult = evaluateExpression(statement.children.at(1), stack);

            if (std::get<data::BasicType>(exprResult.type()) !=
                gVariableTypes.at(newVarType)) {    // Expression result not of same type as declared variable
                logger.err("ERROR") << " at node '" << newVarType << " = " << exprResult.type().toString() << "'";
                throw InvalidException("InvalidAssign: Cannot convert expression type to variable type.");
            }
            stack.push(newVarName, exprResult);    // assign is valid

        } else if (statement.children.at(0).type == NodeType::ExistingVariable) {    // assign to an existing variable
            const data::Value &existingVar = stack.find(statement.children.at(0).value);
            data::Value exprResult = evaluateExpression(statement.children.at(1), stack);

            if (!std::holds_alternative<data::BasicType>(exprResult.type())) {
                logger.err("ERROR") << " at node " << existingVar.type().toString() << " = "
                                    << exprResult.type().toString();
                throw InvalidException("InvalidAssign: Cannot assign to a Record (yet).");
            }
            if (std::get<data::BasicType>(exprResult.type()) !=
                std::get<data::BasicType>(
                    existingVar.type())) {    // Expression result not of same type as declared variable
                logger.err("ERROR") << " at node " << existingVar.type().toString() << " = "
                                    << exprResult.type().toString();
                throw InvalidException("InvalidAssign: Cannot convert expression type to variable type.");
            }

            stack.setValue(statement.children.at(0).value, exprResult);
        } else {
            logger.err("ERROR") << " at node " << statement.children.at(1).value;
            throw InvalidException(
                "Invalid Assign Statement: Expected variable term as left-hand of assign statement.");
        }

        // data::Value exprResult = evaluateExpression(statement.children.at(0), stack);
        // std::cout << "ASSIGN:  " << statement.children.at(2).value << std::endl;
    } else {    // unsupported yet
        logger.err("ERROR") << " at node " << statement.value;
        throw InvalidException("Invalid Statement: Unsupported statement.");
    }
}

// Execute a function
data::Value Interpreter::executeFunction(const Node &functionCall, const Stack &stack)
{
    PROFILE_FUNCTION();
    if (functionCall.children.size() != 2) {    // should be [Variable, FunctionParams]
        logger.err("ERROR") << " at node " << functionCall.value;
        throw InvalidException("Invalid Function Statement: Expected callee and params children node.");
    }
    const Node &callee = functionCall.children.at(0);
    if (!gBuiltinsCallbacks.contains(callee.value)) {    // Only support builtin functions for now
        logger.err("ERROR") << " at node " << callee.value;
        throw InvalidException("Unknown Function: Pivotscript only supports built-in functions for now.");
    }
    std::vector<data::Value> parameters;
    for (const Node &param: functionCall.children.at(1).children) {    // get all the parameters for the callback
        if (param.type == NodeType::FunctionCall)                      // parameter is function call
            parameters.push_back(executeFunction(param, stack));
        else if (param.type == NodeType::Expression)    // parameter is expression
            parameters.push_back(evaluateExpression(param, stack));
        else    // parameter is variable
            parameters.push_back(valueOf(param, stack));
    }
    // validate the parameters and call the callback for the built-in function
    validateParams(parameters, gBuiltinsCallbacks.at(callee.value).second.first,
                   gBuiltinsCallbacks.at(callee.value).second.second,
                   callee.value);    // pair is <size_t numberOfParams, vector<data::Type> types>
    return gBuiltinsCallbacks.at(callee.value)
        .first(parameters, m_builtinContext);    // return the return value of the built-in
}

// Validate the parameters for a builtin
void validateParams(const std::vector<data::Value> &toValidate, size_t expectedSize,
                    const std::vector<std::vector<data::Type>> &expectedTypes, const std::string &name)
{
    PROFILE_FUNCTION();
    if (expectedSize != std::numeric_limits<size_t>::max()) {    // limited number of parameters
        // check expected size
        if (toValidate.size() != expectedSize) {
            logger.err("ERROR") << " at node " << name;
            logger.err("Expected ") << expectedSize << " parameters, got " << toValidate.size() << ".";
            throw InvalidException("Bad Number Of Parameters: Wrong number of parameters.");
        }
        // check expected types (assume expectedTypes is of right size)
        for (size_t i = 0; i < expectedSize; i++)
            if (std::find(expectedTypes.at(i).begin(), expectedTypes.at(i).end(), toValidate.at(i).type()) ==
                expectedTypes.at(i).end()) {
                logger.err("ERROR") << " at node " << name;
                logger.err("Expected one of ") << stringifyVectorType(expectedTypes.at(i)) << " parameter types, got "
                                               << toValidate.at(i).type().toString() << ".";
                throw InvalidException("BadParameterType");
            }
    } else {    // unlimited number of parameters (only one type of expected types)
        const std::vector<data::Type> &validTypes =
            expectedTypes.at(0);    // assumes the vector is correctly initialized

        for (const data::Value &valueToValidate: toValidate)    // iterate over every parameter value
            if (std::find(validTypes.begin(), validTypes.end(), valueToValidate.type()) ==
                validTypes.end()) {    // check it has correct type
                logger.err("ERROR") << " at node " << name;
                logger.err("Expected one of ") << stringifyVectorType(validTypes) << " parameter types, got "
                                               << valueToValidate.type().toString() << ".";
                throw InvalidException("BadParameterType");
            }
    }
}

// Register a component declaration node
void registerComponentDeclaration(const Node &component, component::Index &componentIndex, const std::string &fileName)
{
    DEBUG_FUNCTION();
    pivot::ecs::component::Description r = {
        .name = component.value, .type = data::BasicType::Boolean, .createContainer = arrayFunctor};
    // Either it is a one line component
    if (component.children.at(0).type ==
        NodeType::Symbol) {    // we know children is not empty from parser::ast_from_file(const std::string &filename)
        const Node &componentTypeNode = component.children.at(1);

        if (!gVariableTypes.contains(componentTypeNode.value)) {    // Not a known pivotscript type
            logger.err("ERROR") << " at line " << componentTypeNode.line_nb << " char " << componentTypeNode.char_nb
                                << ": '" << componentTypeNode.value << "'";
            throw UnknownTypeException("This is not a PivotScript type.");
        }

        // Type of component is data::BasicType
        r.type = gVariableTypes.at(componentTypeNode.value);
    } else {    // or a multi-line component
        // Type of component is data::RecordType
        r.type = data::RecordType{};

        for (size_t i = 0; i < component.children.size() - 1; i += 2) {    // Loop over every property of the component
            const Node &propertyTypeNode = component.children.at(i);
            const Node &propertyNameNode = component.children.at(i + 1);
            if (!gVariableTypes.contains(propertyTypeNode.value)) {    // Not a known pivotscript type
                logger.err("ERROR") << " at line " << propertyTypeNode.line_nb << " char " << propertyTypeNode.char_nb
                                    << ": '" << propertyTypeNode.value << "'";
                throw UnknownTypeException("This is not a PivotScript type.");
            }
            // Push the property in the record type of the description
            std::get<data::RecordType>(r.type)[propertyNameNode.value] = gVariableTypes.at(propertyTypeNode.value);
        }
    }
    r.provenance = Provenance::externalRessource(fileName);
    r.defaultValue = r.type.defaultValue();
    componentIndex.registerComponent(r);
}
void registerEventDeclaration(const Node &event, event::Index &eventIndex, const std::string &filename)
{
    pivot::ecs::event::Description r = {
        .name = event.value,
    };
    for (const Node &eventParameter: event.children) {
        switch (eventParameter.type) {
            case NodeType::EventEntityParameter:
                // TODO : handle event description with ecs
                break;
            case NodeType::EventPayloadType:
                if (!gVariableTypes.contains(eventParameter.value)) {    // Not a known pivotscript type
                    logger.err("ERROR") << " at line " << eventParameter.line_nb << " char " << eventParameter.char_nb
                                        << ": '" << eventParameter.value << "'";
                    throw UnknownTypeException("This is not a PivotScript type.");
                }
                r.payload = gVariableTypes.at(eventParameter.value);
                break;
            case NodeType::EventPayloadName: r.payloadName = eventParameter.value; break;
        }
    }
    r.provenance = Provenance::externalRessource(filename);
    eventIndex.registerEvent(r);
}

// Register a system declaration node
systems::Description registerSystemDeclaration(const Node &system, const std::string &fileName)
{
    DEBUG_FUNCTION();
    pivot::ecs::systems::Description sysDesc = {.name = system.value};
    size_t nbChildren = system.children.size();
    if (nbChildren < 6) {    // ??minimum system declaration node??
        std::cerr
            << "registerSystemDeclaration(const Node &system): Invalid system, not enough children for declaration"
            << std::endl;
        sysDesc.name = "Error 1";
        return sysDesc;
    }
    pivot::ecs::event::Description evtDesc;
    size_t cursor = 0;
    expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "(", true);
    while (cursor < nbChildren && system.children.at(cursor).value != ")") {
        consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EntityParameterName);
        expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "<", true);
        while (cursor < nbChildren &&
               system.children.at(cursor).value != ">") {    // consume all that entity's components
            consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EntityParameterComponent);
            if (cursor < nbChildren && system.children.at(cursor).value == ">")    // no more ',', end of loop
                break;
            expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
        }
        expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ">", true);
        if (cursor < nbChildren && system.children.at(cursor).value == ")")    // no more ',', end of loop
            break;
        expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
    }
    expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ")", true);
    if (cursor == nbChildren || system.children.at(cursor).value != "event") {    // Default Tick event
        evtDesc.name = "Tick";
        evtDesc.payload = data::BasicType::Number;
        evtDesc.payloadName = "deltaTime";
        evtDesc.provenance = Provenance();
    } else {    // Custom event
        expectNodeTypeValue(system.children, cursor, NodeType::EventKeyword, "event", true);
        consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventName);
        expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "(", true);
        while (cursor < nbChildren - 1 && system.children.at(cursor).value != ")") {    // consume all event parameters
            if (system.children.at(cursor + 1).value == "<") {                          // Entity parameter
                sysDesc.eventComponents.push_back({});
                consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventEntityName);
                expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "<", true);
                while (cursor < nbChildren &&
                       system.children.at(cursor).value != ">") {    // consume all event entity components
                    consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventEntityComponent);
                    if (cursor < nbChildren && system.children.at(cursor).value == ">")    // no more ',', end of loop
                        break;
                    expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
                }
                expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ">", true);
            } else {    // payload
                consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventPayloadType);
                consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventPayloadName);
            }
            if (cursor < nbChildren && system.children.at(cursor).value == ")")    // no more ',', end of loop
                break;
            expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
        }
    }
    sysDesc.provenance = Provenance::externalRessource(fileName);
    evtDesc.provenance = Provenance::externalRessource(fileName);
    sysDesc.eventListener = evtDesc;
    // systemIndex.registerSystem(sysDesc);
    return sysDesc;
}

// Consume a node for system description
void consumeNode(const std::vector<Node> &children, size_t &childIndex, systems::Description &sysDesc,
                 event::Description &evtDesc, NodeType expectedType)
{
    DEBUG_FUNCTION();
    if (childIndex >= children.size()) {    // TODO : for sure this -1 crashes, fix
        logger.err("ERROR") << " at line " << children.at(childIndex - 1).line_nb << " char "
                            << children.at(childIndex - 1).char_nb << ": '" << children.at(childIndex - 1).value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedEOFException("");
    }
    const script::Node &node = children.at(childIndex);
    if (node.type != expectedType) {
        logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedNodeTypeException("");
    }
    switch (expectedType) {
        case NodeType::EntityParameterName:
            // TODO : keep record of entity parameter names for execution later
            sysDesc.entityName = node.value;
            break;
        case NodeType::EntityParameterComponent: sysDesc.systemComponents.push_back(node.value); break;
        case NodeType::EventName: evtDesc.name = node.value; break;
        case NodeType::EventEntityName: evtDesc.entities.push_back(node.value); break;
        case NodeType::EventEntityComponent: sysDesc.eventComponents.back().push_back(node.value); break;
        case NodeType::EventPayloadName:
            // TODO : put payload name back in description
            evtDesc.payloadName = node.value;
            break;
        case NodeType::EventPayloadType:
            if (!gVariableTypes.contains(node.value)) {    // Not a known pivotscript type
                logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value
                                    << "'";
                throw UnknownTypeException("This is not a PivotScript type.");
            }
            evtDesc.payload = gVariableTypes.at(node.value);
            break;
        default:
            logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value
                                << "'";
            logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
            throw UnexpectedNodeTypeException("");
    }
    childIndex++;
}

data::Value valueOf(const Node &var, const Stack &stack)
{    // get the data::Value version of a variable
    PROFILE_FUNCTION();
    if (var.type == NodeType::LiteralNumberVariable)
        return data::Value(std::stod(var.value));
    else if (var.type == NodeType::DoubleQuotedStringVariable)
        return data::Value(var.value);
    else if (var.type == NodeType::Boolean) {
        if (var.value == "True")
            return data::Value(true);
        else if (var.value == "False")
            return data::Value(false);
        else {
            logger.err("ERROR") << " with variable " << var.value;
            throw std::invalid_argument("Unsupported Feature: A Boolean is either 'True' or 'False'.");
        }
    } else if (var.type == NodeType::ExistingVariable) {    // Named variable
        return stack.find(var.value);                       // find it in the stack
    } else {
        logger.err("ERROR") << " with variable " << var.value;
        throw std::invalid_argument("Unsupported Feature: This type of variable is not supported yet.");
    }
}
// Only binary operators are supported (which take 2 operands exactly)
data::Value evaluateFactor(const data::Value &left, const data::Value &right, const std::string &op)
{    // Return the result of the operation op on left and right
    PROFILE_FUNCTION();
    if (!gOperatorCallbacks.contains(op)) {
        logger.err("ERROR") << " with operator " << op;
        throw InvalidException("Unsupported Feature: This type of operator is not supported yet.");
    }
    return gOperatorCallbacks.at(op)(left, right);
}

data::Value Interpreter::evaluateExpression(const Node &expr, const Stack &stack)
{    // evaluate a postfix expression
    // assume expr.type is NodeType::Expression
    PROFILE_FUNCTION();
    if (expr.children.size() == 1) {    // only one variable in the expression, no operators
        if (expr.children.at(0).type == NodeType::FunctionCall)
            return executeFunction(expr.children.at(0), stack);
        else
            return valueOf(expr.children.at(0), stack);
    }
    // Expression has operators, evaluate the postfix expression

    // vector to hold operands/operators during evaluation
    std::vector<ExpressionOp> ops;
    for (const Node &n: expr.children) {    // fill the vector
        if (n.type == NodeType::Operator)
            ops.push_back(ExpressionOp{.isOperator = true, .operatorStr = n.value});
        else if (n.type == NodeType::FunctionCall)
            ops.push_back(ExpressionOp{.isOperator = false, .operand = executeFunction(n, stack)});
        else
            ops.push_back(ExpressionOp{.isOperator = false, .operand = valueOf(n, stack)});
    }
    // evalute the postfix expression by evaluating first occurence of -- operand operand operator --
    while (ops.size() != 1) {                            // run until only one value is left
        for (size_t i = 0; i < ops.size() - 2; i++) {    // find first occurence of -- operand operand operator --
            if (!ops.at(i).isOperator && !ops.at(i + 1).isOperator && ops.at(i + 2).isOperator) {
                // put the result into the i index op
                ops.at(i) = ExpressionOp{
                    .isOperator = false,
                    .operand = evaluateFactor(ops.at(i).operand, ops.at(i + 1).operand, ops.at(i + 2).operatorStr)};
                // remove the other two indices
                ops.erase(std::next(ops.begin(), i + 1), std::next(ops.begin(), i + 3));
                break;
            }
        }
    }
    return ops.at(0).operand;
}

// Throw if the type or value of the targeted node isn't equal to the expected type and value
void expectNodeTypeValue(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType,
                         const std::string &expectedValue, bool consume)
{
    PROFILE_FUNCTION();
    if (childIndex == children.size()) {
        logger.err("ERROR") << " at line " << children.at(childIndex - 1).line_nb << " char "
                            << children.at(childIndex - 1).char_nb << ": '" << children.at(childIndex - 1).value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedEOFException("");
    }
    const Node &node = children.at(childIndex);
    if (node.type != expectedType) {
        logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedNodeTypeException("");
    }
    if (node.value != expectedValue) {
        logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value << "'";
        logger.err("Expected ") << expectedValue << " node value.";
        throw UnexpectedNodeValueException("");
    }
    if (consume) childIndex++;
}
// Throw if the type of the targeted node isn't equal to the expected type
void expectNodeType(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, bool consume)
{
    PROFILE_FUNCTION();
    if (childIndex == children.size()) {
        logger.err("ERROR") << " at line " << children.at(childIndex - 1).line_nb << " char "
                            << children.at(childIndex - 1).char_nb << ": '" << children.at(childIndex - 1).value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedEOFException("");
    }
    const Node &node = children.at(childIndex);
    if (node.type != expectedType) {
        logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value << "'";
        logger.err("Expected ") << magic_enum::enum_name(expectedType) << " node type.";
        throw UnexpectedNodeTypeException("");
    }
    if (consume) childIndex++;
}
// Throw if the value of the targeted node isn't equal to the expected value
void expectNodeValue(const std::vector<Node> &children, size_t &childIndex, const std::string &expectedValue,
                     bool consume)
{
    PROFILE_FUNCTION();
    if (childIndex == children.size()) {
        logger.err("ERROR") << " at line " << children.at(childIndex - 1).line_nb << " char "
                            << children.at(childIndex - 1).char_nb << ": '" << children.at(childIndex - 1).value << "'";
        logger.err("Expected ") << expectedValue << " node value.";
        throw UnexpectedNodeValueException("");
    }
    const Node &node = children.at(childIndex);
    if (node.value != expectedValue) {
        logger.err("ERROR") << " at line " << node.line_nb << " char " << node.char_nb << ": '" << node.value << "'";
        logger.err("Expected ") << expectedValue << " node value.";
        throw UnexpectedNodeValueException("");
    }
    if (consume) childIndex++;
}

// Stringify a vector of data::Type types
std::string stringifyVectorType(const std::vector<data::Type> &types)
{
    std::string r = "[ ";
    for (data::Type type: types) r += type.toString() + (type == types.at(types.size() - 1) ? " , " : "");
    r += " ]";
    return r;
}

// Callback to create a container for components
std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description)
{
    return std::make_unique<component::ScriptingComponentArray>(description);
}

}    // end of namespace pivot::ecs::script::interpreter
