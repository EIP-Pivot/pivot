#pragma once

#include "pivot/script/DataStructures.hxx"
#include "pivot/script/Builtins.hxx"
#include "pivot/script/Stack.hxx"

// To register global system index descriptions
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"
#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/ScriptingComponentArray.hxx"


// #include "pivot/graphics/Window.hxx"
// #include "../../../pivot/libgraphics/include/pivot/graphics/Window.hxx"


#include <unordered_map>

namespace pivot::ecs::script::interpreter {

// Parse a file syntax tree to register the component and system declarations
std::vector<systems::Description> registerDeclarations(const Node &file, component::Index &componentIndex);

// Execute a SystemEntryPoint node by executing all of its statements
void executeSystem(const Node &systemEntry, const systems::Description &desc, component::ArrayCombination::ComponentCombination &entity, const event::EventWithComponent &trigger, Stack &stack);


// Private functions

// Validate the parameters for a builtin
void validateParams(const std::vector<data::Value> &toValidate, size_t expectedSize, const std::vector<data::Type> &expectedTypes, const std::string &name);

// Register description from a declaration
void registerComponentDeclaration(const Node &component, component::Index &componentIndex, const std::string &fileName);
systems::Description registerSystemDeclaration(const Node &system, const std::string &fileName);

// Consume a child node from a declaration
void consumeNode(const std::vector<Node> &children, size_t &childIndex, systems::Description &sysDesc, event::Description &evtDesc, NodeType expectedType);

// Node utils, and evaluation of expressions
data::Value valueOf(const Node &var, const Stack &stack); // get value of variable
data::Value evaluateFactor(const data::Value &left, const data::Value &right, const std::string &op); // Return the result of the operation op on left and right
data::Value evaluateExpression(const Node &expr, const Stack &stack); // evaluate a postfix expression

// Expect node to have a certain type, value or both (throw if not)
void expectNodeTypeValue(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, const std::string &expectedValue, bool consume);
void expectNodeType(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, bool consume);
void expectNodeValue(const std::vector<Node> &children, size_t &childIndex, const std::string &expectedValue, bool consume);

// createContainer
std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description);

} // end of namespace pivot::ecs::script::interpreter
