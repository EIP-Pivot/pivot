#ifndef __SCRIPTING__TYPES__
#define __SCRIPTING__TYPES__

#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"
#include "pivot/ecs/Core/Component/description.hxx"
#include "pivot/ecs/Core/Component/index.hxx"
#include "pivot/ecs/Core/Component/ScriptingComponentArray.hxx"

// TODO : replace these by the actual components and descriptions

// struct Property {
// 	std::string name;
// 	std::string type;
// };

// struct ComponentDescription {
// 	std::string name;
// 	std::vector<Property> properties;
// };

// struct SystemDescription {
// 	std::string name;
// 	std::vector<std::string> inputComponents;
// };

using ComponentDescription = pivot::ecs::component::Description;
using SystemDescription = pivot::ecs::systems::Description;


struct LoadResult {
	std::vector<ComponentDescription> components;
	std::vector<SystemDescription> systems;
	std::string output;
};

enum IndentType {
	TAB,
	SPACE,
	NOINDENT
};

struct Indent {
	IndentType type;
	size_t size;
};

enum State {
	START,
	COMPONENT_DECL,
	SYSTEM_DECL,
	PROPERTY_DECL,
	INSTRUCTION,
	INVALID,
};

enum InstructionType {
	ASSIGN,
	FUNCTION,
	BLOCK_OPERATOR
};

std::string StateToString(State s);


#endif