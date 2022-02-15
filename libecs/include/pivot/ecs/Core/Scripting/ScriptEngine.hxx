#ifndef __SCRIPT__ENGINE__
#define __SCRIPT__ENGINE__
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <any>

#include "pivot/ecs/Core/Scripting/Exceptions.hxx"

// TODO : remove this
// This silences iterator warnings from CombinationArray


struct ComponentPosition {
	double pos_x;
	double pos_y;
	double pos_z;
};
struct ComponentVelocity {
	double vel_x;
	double vel_y;
	double vel_z;
};

struct Variable {
	std::string name;
	std::string type;
	std::any value;
	std::vector<Variable> fields;
};

class ScriptEngine {

public:
	ScriptEngine();
	~ScriptEngine();
	LoadResult loadFile(const std::string &fileName, bool verbose);
	void executeSystem(const pivot::ecs::systems::Description &toExec, std::vector<std::vector<std::pair<ComponentDescription, std::any>>> &entities, const pivot::ecs::event::Event &event);

	void executeSystemNew(const pivot::ecs::systems::Description &toExec, pivot::ecs::systems::Description::systemArgs &components, const pivot::ecs::event::Event &event);

	void totalReset(); // This is to reset the entire file (for tests notably)
	void softReset(); // This is to reset the data needed to read files but not the already registered data

protected:

	void executeSystem(const std::string &systemName, std::vector<std::pair<ComponentDescription, std::any>> &entity, size_t entityId);
	size_t indexOf(const std::string &systemName);
	bool populateLinesFromFile(const std::string &fileName);
	void cleanLine(const std::string &line);
	bool lineIsEmpty(const std::string &line);
	Indent getIndent(const std::string &line);
	bool badIndent(const std::string &line, State lineState);
	bool validIndentSize(size_t indentSize, State lineState);
	State getLineState();
	bool expectsState(State previous, State next);
	size_t getLineIndent();

	bool handleState(State lineState, LoadResult &result);
	bool handleStart();
	bool handleComponentDecl(LoadResult &result);
	bool handleSystemDecl(LoadResult &result);
	bool handlePropertyDecl();
	bool handleInstructionDecl();

	void registerComponent(LoadResult &result, bool verbose = true);
	void registerSystem(LoadResult &result, bool verbose = true);
	void cleanInstruction(std::string &instruction);
	InstructionType getInstructionType(const std::string &instruction);
	void handleInstruction(const std::string &instruction, InstructionType iType);
	bool isSolvable(const std::string &toSolve);
	
	std::any getField(const std::any &component, const std::string &componentName, const std::string &fieldName);
	void printStack();
	void printValue(const std::any &value, const std::string &type);


	std::vector<std::string> _lines;
	std::string _line;
	size_t _currentLine;
	Indent _fileIndent;
	bool _fileIndentSet;
	size_t _currentIndent;
	State _currentState;
	ComponentDescription _phComponent;
	SystemDescription _phSystem;
	std::vector<std::string> _phInstructions;

	std::vector<SystemDescription> _systems;
	std::vector<std::vector<std::string>> _systemsInstructions;
	// std::vector<ComponentDescription> _components;

	std::vector<Variable> _variables;
};

std::vector<std::string> split(const std::string& str, const std::string& delim);
std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description);

/*

system onColisionWithObject(Player<Holding,Score,Tag>) event Collision(Object<Holding, Follow, Transform>, Other<Hole>, deltaTime)

system s(a<A,B,C>) event E(b<A,B>, c<D>, data)


systemDescription
	name:	s
	systemComponents:	[ A, B, C ]
	eventListener:
		name:	E
		entities:	[b, c]
		payload:	data::Type::Number
	eventComponents: [ [A,B], [D] ]
	system:		myCallback

*/


#endif