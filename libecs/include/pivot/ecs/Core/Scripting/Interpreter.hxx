#pragma once

#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1

#include "pivot/ecs/Core/Scripting/DataStructures.hxx"

// To register global system index descriptions
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"
#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/ScriptingComponentArray.hxx"

namespace pivot::ecs::script::interpreter {

// Parse a file syntax tree to register the component and system declarations
std::vector<systems::Description> registerDeclarations(const Node &file, component::Index &componentIndex);



// private functions
void registerComponentDeclaration(const Node &component, component::Index &componentIndex, const std::string &fileName);
systems::Description registerSystemDeclaration(const Node &system, const std::string &fileName);

void consumeNode(const std::vector<Node> &children, size_t &childIndex, systems::Description &sysDesc, event::Description &evtDesc, NodeType expectedType);
void expectNodeTypeValue(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, const std::string &expectedValue, bool consume);

std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description);

} // end of namespace pivot::ecs::script::interpreter
