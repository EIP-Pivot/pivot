#ifndef __SCRIPT__ENGINE__
#define __SCRIPT__ENGINE__
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include "pivot/ecs/Core/Scripting/Stack.hxx"

namespace pivot::ecs::script {


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
	pivot::ecs::data::Value value;
	std::vector<Variable> fields;
};

class ScriptEngine {
public:
	// class Stack {
	// public:
	// 	Stack() = default;
	// 	~Stack() = default;
	// protected:
	// 	std::vector<VariableNew> _stack;
	// };

	ScriptEngine();
	~ScriptEngine();
	LoadResult loadFile(const std::string &fileName, bool verbose);
	// void executeSystem(const pivot::ecs::systems::Description &toExec, std::vector<std::vector<std::pair<ComponentDescription, std::any>>> &entities, const pivot::ecs::event::Event &event);

	void executeSystemNew(const pivot::ecs::systems::Description &toExec, pivot::ecs::systems::Description::systemArgs &entities, const pivot::ecs::event::Event &event);

	void totalReset(); // This is to reset the entire file (for tests notably)
	void softReset(); // This is to reset the data needed to read files but not the already registered data

	long long _totalCompute;
	long long _totalReplaceall;
	long long _totalCompile;

protected:

	// void executeSystem(const std::string &systemName, std::vector<std::pair<ComponentDescription, std::any>> &entity, size_t entityId);
	void executeSystemOnEntity(const pivot::ecs::systems::Description &toExec, pivot::ecs::component::ArrayCombination::ComponentCombination &entity, const pivot::ecs::event::Event &event);
	size_t indexOf(const std::string &systemName);
	bool populateLinesFromFile(const std::string &fileName);
	void cleanLine(const std::string &line);
	void cleanWhitespace();
	bool lineIsEmpty(const std::string &line);
	Indent getIndent(const std::string &line);
	bool badIndent(const std::string &line, State lineState);
	bool validIndentSize(size_t indentSize, State lineState);
	State getLineState();
	bool expectsState(State previous, State next);
	size_t getLineIndent();
	std::vector<SystemParameter> getEntitiesFromSystem(const std::string &str);
	std::vector<EventParameter> getEntitiesFromEvent(const std::string &str);

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
	bool isSolvableNew(const std::string &toSolve);
	data::Value &solve(const std::string &toSolve);
	data::Value computeExpression(const std::string &toSolve);
	bool isLiteralNumber(const std::string &expr);
	std::any getField(const std::any &component, const std::string &componentName, const std::string &fieldName);
	void printStack();
	void printVec3(const glm::vec3 &vec);
	void printValue(const data::Value &value, const data::Type &type);
	long long timeThis(std::function<void()> toTime, const std::string &name = "", bool print = false);

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
	// std::map<std::string, ComponentDescription> _components;
	std::vector<std::vector<std::string>> _systemsInstructions;
	std::map<std::string, SystemParameter> _systemParameters;
	std::map<std::string, EventParameter> _eventParameters;
	// std::vector<ComponentDescription> _components;

	std::vector<Variable> _variables;
	std::vector<VariableNew> _stack;
	Stack _stackNew;

	exprtk::expression<double> _expression;
	exprtk::symbol_table<double> _symbol_table;
	exprtk::parser<double> _parser;


	std::chrono::high_resolution_clock _clock;
};

void replaceAll(std::string& str, const std::string& from, const std::string& to);
bool doubleSpacePredicate(char left, char right);
std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description);

} // end of namespace pivot::ecs::script

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