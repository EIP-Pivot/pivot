#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"

using namespace pivot::ecs;
using namespace pivot::ecs::script;

static std::map<std::string, data::BasicType> variableTypes {
	{"Vector3", data::BasicType::Vec3},
	{"Number", data::BasicType::Number},
	{"Boolean", data::BasicType::Boolean},
	{"Color", data::BasicType::Number},
	{"String", data::BasicType::String}
};


// static const std::vector<std::string> variableTypes {
// 	"Vector3",
// 	"Number",
// 	"Boolean",
// 	"String",
// 	"Color"
// };

static const std::vector<std::string> blockOperators {
	"if",
	"while",
	"for"
};


ScriptEngine::ScriptEngine()
:	_phComponent ( {"", data::RecordType(), arrayFunctor})
{
	_currentLine = 0;
	_fileIndent = Indent{NOINDENT, 0};
	_fileIndentSet = false;
	_currentState = START;
	_currentIndent = 0;
	// _phComponent.createContainer = arrayFunctor;
	_totalCompute = 0;
	_totalReplaceall = 0;
	_totalCompile = 0;
	_symbol_table.add_infinity();
	_expression.register_symbol_table(_symbol_table);
}

ScriptEngine::~ScriptEngine() {
}

LoadResult ScriptEngine::loadFile(const std::string &fileName, bool verbose = false) {
	LoadResult result;

	softReset();
try {
	if (!populateLinesFromFile(fileName))
		return result;
	for (std::string line : _lines) {
		_currentLine++;
		cleanLine(line);
		// using _line from now on
		if (lineIsEmpty(_line))
			continue ;
		State lineState = getLineState();
		size_t lineIndent = getLineIndent();
		if (!expectsState(_currentState, lineState))
			throw UnexpectedStateException("ERROR", _line.data(), _currentLine, _currentState, lineState);
		if (badIndent(_line, lineState))
			throw BadIndentException("ERROR", _line.data(), _currentLine, "Bad indent");
		cleanWhitespace();
		if (!handleState(lineState, result))
			throw UnexpectedStateException("ERROR", _line.data(), _currentLine, _currentState, lineState);
		_currentState = lineState;
		_currentIndent = lineIndent;
	}
	if (_currentState == INSTRUCTION)
		registerSystem(result);
	if (_currentState == PROPERTY_DECL)
		registerComponent(result);
	_systems = result.systems;
	// _components = result.components;
}
catch (BadIndentException e) {
	result.output = std::string("\n") + e.what() + ":\t" + fileName + "\n\tline " + std::to_string(e.get_line_nb()) + ":\t" + e.get_line() + "\nIndentError: " + e.get_info() + "\n";
	if (verbose)
		std::cout << result.output << std::flush;
}
catch (InvalidSyntaxException e) {
	result.output = std::string("\n") + e.what() + ":\t" + fileName + "\n\tline " + std::to_string(e.get_line_nb()) + ":\t" + e.get_line() + "\nSyntaxError: " + e.get_info() + "\n";
	if (verbose)
		std::cout << result.output << std::flush;
}
catch (UnknownExpressionException e) {
	result.output = std::string("\n") + e.what() + ":\t" + fileName + "\n\tline " + std::to_string(e.get_line_nb()) + ":\t" + e.get_line() + "\nSyntaxError: " + e.get_info() + "\n";
	if (verbose)
		std::cout << result.output << std::flush;
}
catch (UnexpectedStateException e) {
	result.output = std::string("\n") + e.what() + ":\t" + fileName + "\n\tline " + std::to_string(e.get_line_nb()) + ":\t" + e.get_line() + "\nLogicError: ";
	if (e.get_new_state() == INVALID)
		result.output += "Unknown expression.\n";
	else
		result.output += "State " + StateToString(e.get_prev_state()) + " did not expect new state " + StateToString(e.get_new_state());
	if (verbose)
		std::cout << result.output << std::flush;
}
catch (std::exception e) {
	result.output = std::string("\n!Unhandled Exception: ") + e.what() + "\n";
	if (verbose)
		std::cout << result.output << std::flush;
}
	if (result.output.empty())
		result.output = fileName + " succesfully parsed.\n";
	return result;
}

// void ScriptEngine::executeSystem(const pivot::ecs::systems::Description &toExec, std::vector<std::vector<std::pair<ComponentDescription, std::any>>> &entities, const pivot::ecs::event::Event &event) {
// try {
// 	size_t entityId = 0;
// 	for (SystemDescription &d : _systems) {
// 		if (toExec.name == d.name) {
// 			std::cout << "System to execute:\t" << d.name << "\n\n";
// 			std::cout << "Entities to execute on:\n";
// 			for (std::vector<std::pair<ComponentDescription, std::any>> &entity : entities) {
// 				std::cout << "\tEntity " << entityId << "\n";
// 				for (size_t i = 0; i < entity.size(); i++) {
// 					std::cout << "\t\tComponent " << entity[i].first.name << "\t";
// 					if (entity[i].first.name == "Position") {
// 						ComponentPosition p = std::any_cast<ComponentPosition>(entity[i].second);
// 						std::cout << p.pos_x << " " << p.pos_y << " " << p.pos_z;
// 					} else if (entity[i].first.name == "Velocity") {
// 						ComponentVelocity v = std::any_cast<ComponentVelocity>(entity[i].second);
// 						std::cout << v.vel_x << " " << v.vel_y << " " << v.vel_z;
// 					}
// 					std::cout << std::endl;
// 				}
// 				executeSystem(toExec.name, entity, entityId);
// 				entityId += 1;
// 			}
// 			break;
// 		}
// 	}
// }
// catch (UnhandledException e) {
// 	std::cout << e.what() << std::endl;
// }
// }

// void ScriptEngine::executeSystem(const std::string &systemName, std::vector<std::pair<ComponentDescription, std::any>> &entity, size_t entityId) {
// 	std::cout << "Executing " << systemName << " on Entity " << entityId << std::endl;
// 	size_t toExec = indexOf(systemName);
// 	if (toExec == SIZE_MAX)
// 		throw UnhandledException("UNKNOWN ERROR: Couldn't find system in\tScriptEngine::executeSystem\tScriptEngin.cpp");
// 	for (auto &[description, component] : entity) {
// 		if (description.name == "Position") {
// 			ComponentPosition data = std::any_cast<ComponentPosition>(component);
// 			Variable v {
// 				.name = "Position",
// 				.type = "Struct",
// 				.value = nullptr
// 			};
// 			// for (Property p : description.properties) {
// 			// 	Variable vv {
// 			// 		.name = p.name,
// 			// 		.type = p.type,
// 			// 		.value = getField(component, description.name, p.name)
// 			// 	};
// 			// 	v.fields.push_back(vv);
// 			// }
// 			_variables.push_back(v);
// 		}
// 		else if (description.name == "Velocity") {
// 			ComponentVelocity data = std::any_cast<ComponentVelocity>(component);
// 			Variable v {
// 				.name = "Velocity",
// 				.type = "Struct",
// 				.value = nullptr
// 			};
// 			// for (Property p : description.properties) {
// 			// 	Variable vv {
// 			// 		.name = p.name,
// 			// 		.type = p.type,
// 			// 		.value = getField(component, description.name, p.name)
// 			// 	};
// 			// 	v.fields.push_back(vv);
// 			// }
// 			_variables.push_back(v);
// 		}
// 		else
// 			throw UnhandledException("UNKNOWN ERROR: Invalid description.name in\tScriptEngine::executeSystem\tScriptEngin.cpp");
// 	}
// 	printStack();
// 	for (std::string instruction : _systemsInstructions[toExec]) {
// 		std::cout << "Instruction:\t'" << instruction << "' to execute." << std::endl;
// 		cleanInstruction(instruction);
// 		InstructionType iType = getInstructionType(instruction);
// 		handleInstruction(instruction, iType);
// 	}
// }

void ScriptEngine::executeSystemNew(const pivot::ecs::systems::Description &toExec, pivot::ecs::systems::Description::systemArgs &entities, const pivot::ecs::event::Event &event) {
	try {
		size_t toExecIndex = indexOf(toExec.name); // Correspondance in between vectors for instructions
		if (toExecIndex == SIZE_MAX) // if for whatever reason the system is not registered in this engine, throw
			throw UnhandledException("UNKNOWN ERROR: Couldn't find system in\tScriptEngine::executeSystemNew\tScriptEngin.cpp");
		// std::cout << "Executing system\t" << toExec.name << std::endl;
		for (auto entity: entities) // input is ALL entities, loop over each
			executeSystemOnEntity(toExec, entity, event);
	}
	catch (InvalidTypeException e) { 
		std::cerr << "\nInvalidTypeError: " << e.what() << std::endl;
	}
	catch (UnhandledException e) { 
		std::cerr << "\n!Unhandled Exception: " << e.what() << std::endl;
	}
	catch (pivot::ecs::component::ComponentRef::MissingComponent e) { // can be thrown by ComponentCombination[].get()
		std::cerr << "MissingComponentException in ScriptEngine::executeSystemNew :\n\t" <<
			e.component.name << " was not found in entity " << e.entity << std::endl;
	}
	catch (UnknownVariableException e) { 
		std::cerr << "\nUnknown Variable: " << e.what() << std::endl;
	}
	catch (std::exception e) { // ?
		std::cerr << "std::exception: " << e.what() << std::endl;
	}
}

void ScriptEngine::executeSystemOnEntity(const pivot::ecs::systems::Description &toExec, pivot::ecs::component::ArrayCombination::ComponentCombination &entity, const pivot::ecs::event::Event &event) {
	// std::chrono::high_resolution_clock clock;
	// Push all input components to stack
	// auto bchmarkInitStart = clock.now();
	size_t componentIndex = 0;
	VariableNew inputEntity = {
		.type = data::RecordType(),
		.value = data::Record(),
		.name = _systemParameters.at(toExec.name).name,
		.isEntity = true
	};
	for (std::string componentString : toExec.systemComponents) {
		// if (!_components.contains(componentString))
		// 	throw UnhandledException(("Component " + componentString + " not found when executing system " + toExec.name).c_str());
		std::get<data::Record>(inputEntity.value).insert_or_assign(componentString, std::get<data::Record>(entity[componentIndex].get()));
		componentIndex++;
	}
	_stackNew.pushVar(inputEntity);
	_stackNew.pushVar(VariableNew {
		.type = event.description.payload,
		.value = event.payload,
		.name = event.description.payloadName,
		.isEntity = false
	});
	// std::cout << "Time to execute ExecuteSystemOnEntity Inititialization:\t" << std::chrono::duration(clock.now() - bchmarkInitStart).count() / 1000000 << " ms" << std::endl;

	// auto bchmarkExecStart = clock.now();
	for (std::string &instruction : _systemsInstructions[indexOf(toExec.name)]) {
		// std::cout << "Instruction:\t'" << instruction << "' to execute." << std::endl;
		cleanInstruction(instruction);
		InstructionType iType = getInstructionType(instruction);
		handleInstruction(instruction, iType);
	}
	// std::cout << "Time to execute ExecuteSystemOnEntity Execution:\t" << std::chrono::duration(clock.now() - bchmarkExecStart).count() / 1000000 << " ms" << std::endl;

	// auto bchmarkReassignStart = clock.now();
	componentIndex = 0;
		for (auto &[key, var] : _stackNew.getVars()) {
			if (key == _systemParameters.at(toExec.name).name) {
				for (auto &[componentName, componentValue] : std::get<data::Record>(var.value)) {
					entity[componentIndex].set(componentValue);
					componentIndex++;
				}
				break;
			}
		}
	_stack.clear();
	_stackNew.clear();
	// std::cout << "Time to execute ExecuteSystemOnEntity Reassign:\t" << std::chrono::duration(clock.now() - bchmarkReassignStart).count() / 1000000 << " ms" << std::endl;
	// std::cout << std::get<std::string>(std::get<data::Record>(entity[0].get()).at("pos_x")) << std::endl;
	// std::cout << std::get<double>(std::get<data::Record>(entity[1].get()).at("vel_x")) << std::endl;
}

void ScriptEngine::handleInstruction(const std::string &instruction, InstructionType iType) {
	// std::chrono::high_resolution_clock clock;
	// auto bchmarkhandleInstrucStart = clock.now();
	switch (iType)
	{
	case ASSIGN:
	{
		size_t equalPos = instruction.find('=');
		if (equalPos == std::string::npos)
			throw UnhandledException("UNKNOWN ERROR: No equal sign in ASSIGN instruction");
		std::string left = instruction.substr(0, equalPos);
		std::string right = instruction.substr(equalPos + 1, instruction.size() - equalPos - 1);
		// std::cout << "Left [" << left << "] Right [" << right << "]" << std::endl;
		if (!isSolvableNew(left))
			throw UnknownVariableException(left.c_str());
		// std::cout << "Is left solvable: " << (isSolvableNew(left) ? "yes" : "no") << std::endl;
		for (std::string &token : split(right, "+-*/%^()")) {
			if (isLiteralNumber(token))
				continue ;
			if (!isSolvableNew(token))
				throw UnknownVariableException(token.c_str());
			data::Value &solvedToken = solve(token);
			try {
				if (std::get<data::BasicType>(solvedToken.type()) != data::BasicType::Number)
					throw UnknownVariableException(solvedToken.type().toString().c_str());
			} catch (std::bad_variant_access e) {
				throw UnknownVariableException("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now ( not Record )");
			} catch (UnknownVariableException e) {
				throw UnknownVariableException((std::string("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now ( not ") + e.what() + ")").c_str());
			}
		}
		// std::chrono::high_resolution_clock clock;
		// auto start = clock.now();
		data::Value result;
		_totalCompute += timeThis([&]() {
			result = computeExpression(right); // *-*
		});
		// data::Value result = computeExpression(right); // *-*
		// std::cout << "Time to execute computeExpression:\t" << std::chrono::duration(clock.now() - start).count() / 1000000 << " ms" << std::endl;
		// std::cout << right << " = " << std::get<double>(result) << std::endl;
		_stackNew.updateVar(left, result);
		break;
						// std::visit(overloaded {
						// 	[&](data::BasicType arg) {
						// 			if (std::get<data::BasicType>(solvedToken.type()) != data::BasicType::Number)
						// 				throw UnknownVariableException("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now"); },
						// 	[&](data::Record arg) { throw UnknownVariableException("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now"); },
						// }, solvedToken);
					// std::visit([&](auto&& arg) {
					// 	using T = std::decay_t<decltype(arg)>;
					// 	if constexpr (std::is_same_v<T, data::BasicType &>) {
					// 		if (std::get<data::BasicType>(solvedToken.type()) != data::BasicType::Number)
					// 			throw UnknownVariableException("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now");
					// 	}
					// 	else if constexpr (std::is_same_v<T, data::RecordType &>)
					// 		throw UnknownVariableException("ScriptEngine::handleInstruction() : can only support BasicType::Number assign for now");
						// else 
						// 	throw UnhandledException("ScriptEngine::handleInstruction() : std::visit is not exhaustive !");
					// }, solvedToken);

			// std::cout << "Right token " << token << (isSolvableNew(token) ? " is solvable" : " is not solvable") << " and has type " << solve(token).type().toString() << std::endl;
		
	}
	case FUNCTION:
	{
		// Not yet implemented
		break;
	}
	case BLOCK_OPERATOR:
	{
		// Not yet implemented
		break;
	}
	default:
	{
		throw UnhandledException("UNKNOWN ERROR: Unknown instruction type");
	}
	}
	// std::cout << "Time to execute handleInstruction:\t" << std::chrono::duration(clock.now() - bchmarkhandleInstrucStart).count() / 1000000 << " ms" << std::endl;
}


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
data::Value ScriptEngine::computeExpression(const std::string &toSolve) { // assumes every token is solvable
	std::string mut = toSolve;
	std::vector<std::string> tokens = split(toSolve, "+-*/%^()");

	_totalReplaceall += timeThis([&]() {
		for (std::string &token : tokens) {
			if (isLiteralNumber(token))
				continue ;
			data::Value &val = solve(token);
			std::visit(overloaded {
				[&](int arg) { replaceAll(mut, token, std::to_string(arg)); },
				[&](double arg) { replaceAll(mut, token, std::to_string(arg)); },
				[&](bool arg) { replaceAll(mut, token, (arg) ? "true" : "false"); },
				[&](const std::string &arg) { replaceAll(mut, token, arg); },
				[&](glm::vec3 arg) { replaceAll(mut, token, std::to_string(arg.x) + ":" + std::to_string(arg.y) + ":" + std::to_string(arg.z)); },
				[&](data::Record arg) { replaceAll(mut, token, "record"); }
			}, val);
		}
	});

	_totalCompile += timeThis([&]() {
		if (!_parser.compile(mut, _expression)) // *-*
			throw UnhandledException(("Failed to compile expression: '" + mut + "' which originated from '" + toSolve + "'").c_str());
	});
	return data::Value(_expression.value());	
}

bool ScriptEngine::isLiteralNumber(const std::string &expr) {
	try {
		std::stod(expr);
		return true;
	} catch (std::invalid_argument e) {
		return false;
	} catch (std::out_of_range e) {
		throw UnhandledException(("Overflow error in ScriptEngine::isLiteralNumber : expr '" + expr + "' leads to std::out_of_range").c_str());
	}
	return false;
}

data::Value &ScriptEngine::solve(const std::string &toSolve) { // assumes isSolvable has been called
	std::string mut = toSolve;
	size_t dotIndex = mut.find('.');
	std::string token = "";
	data::Record vars = _stackNew.getAsRecord();
	data::Record *cursor = &vars;
	
	while (dotIndex != std::string::npos) {
		token = mut.substr(0, dotIndex);
		if (!cursor->contains(token))
			throw UnknownVariableException((token + " in " + mut).c_str());
		try {
			cursor = &(std::get<data::Record>(cursor->at(token)));
		} catch (std::bad_variant_access e) {
			throw UnknownVariableException((token + " is not a data::Record").c_str());
		}
		mut = mut.substr(dotIndex + 1);
		dotIndex = mut.find('.');
	}
	token = mut.substr(0, dotIndex);
	if (!cursor->contains(token))
		throw UnknownVariableException((token + " in " + mut).c_str());
	return _stackNew.getVarValue(toSolve);
}

bool ScriptEngine::isSolvableNew(const std::string &toSolve) {
	if (isLiteralNumber(toSolve))
		return true;
	std::string mut = toSolve;
	size_t dotIndex = mut.find('.');
	std::string token = "";
	data::Record vars = _stackNew.getAsRecord();
	data::Record *cursor = &vars;
	
	while (dotIndex != std::string::npos) {
		std::string token = mut.substr(0, dotIndex);
		if (!cursor->contains(token))
			throw UnknownVariableException((token + " in variable " + mut).c_str());
		try {
			cursor = &(std::get<data::Record>(cursor->at(token)));
		} catch (std::bad_variant_access e) {
			std::cout << "Token " << token << "of cursor " << cursor << " is not a data::Record" << std::endl;
			return false;
		}
		mut = mut.substr(dotIndex + 1);
		dotIndex = mut.find('.');
	}
	token = mut.substr(0, dotIndex);
	if (!cursor->contains(token))
		throw UnknownVariableException((token + " in variable " + mut).c_str());
	return true;
}

bool ScriptEngine::isSolvable(const std::string &toSolve) {
	std::string mut = toSolve;
	std::vector<Variable> toCheck = _variables;
	size_t index = mut.find('.');
	bool found = false;

	while (index != std::string::npos) {
		std::string token = mut.substr(0, index);
		for (size_t i = 0; i < toCheck.size(); i++) {
			if (toCheck[i].name.compare(token) == 0) {
				found = true;
				toCheck = std::vector(toCheck[i].fields);
				break;
			}
		}
		if (!found) {
			std::cout << "DID NOT FIND " << token << " in toCheck" << std::endl;
			return false;
		}
		mut = mut.substr(index + 1);
		index = mut.find('.');
	}
	std::string token = mut.substr(0, index);
	for (size_t i = 0; i < toCheck.size(); i++) {
		if (toCheck[i].name.compare(token) == 0) {
			found = true;
			toCheck = std::vector(toCheck[i].fields);
			break;
		}
	}
	if (!found) {
		std::cout << "DID NOT FIND " << token << " in toCheck" << std::endl;
		return false;
	}
	return true;
}

std::any ScriptEngine::getField(const std::any &component, const std::string &componentName, const std::string &fieldName) {
	if (componentName == "Position") {
		ComponentPosition comp = std::any_cast<ComponentPosition>(component);
		if (fieldName == "pos_x")
			return comp.pos_x;
		else if (fieldName == "pos_y")
			return comp.pos_y;
		else if (fieldName == "pos_z")
			return comp.pos_z;
		else
			throw UnhandledException("UNKNOWN ERROR: Bad field name in ScriptEngine::getPositionField");
	}
	else if (componentName == "Velocity") {
		ComponentVelocity comp = std::any_cast<ComponentVelocity>(component);
		if (fieldName == "vel_x")
			return comp.vel_x;
		else if (fieldName == "vel_y")
			return comp.vel_y;
		else if (fieldName == "vel_z")
			return comp.vel_z;
		else
			throw UnhandledException("UNKNOWN ERROR: Bad field name in ScriptEngine::getPositionField");
	} else
		throw UnhandledException("UNKNOWN ERROR: Bad component name in ScriptEngine::getPositionField");
}

void ScriptEngine::cleanInstruction(std::string &instruction) {
	while (instruction.find_first_of(" \t") != std::string::npos)
		instruction.erase(instruction.find_first_of(" \t"), 1);
}

void ScriptEngine::totalReset() {
	softReset();
	_systems.clear();
	_systemsInstructions.clear();
	// _components.clear();
	_variables.clear();
}

void ScriptEngine::softReset() {
	_lines.clear();
	_line.clear();
	_currentLine = 0;
	_fileIndent = Indent{NOINDENT, 0};
	_fileIndentSet = false;
	_currentIndent = 0;
	_currentState = START;
}


void ScriptEngine::printStack() {
	static int step = 0;
	std::cout << "-----------------------------------\t\t\t\t\t\nPRINTING STACK step " << step << "\n\n" << std::endl;
	for (VariableNew v : _stack) {
		std::cout << "\t" << v.name << " : " << v.type.toString();
		try {
			data::Record rec = std::get<data::Record>(v.value);
			for (auto &[key, value] : rec) {
				std::cout << "\n\t\t" << key << " : " << value.type().toString() << " = ";
				printValue(value, value.type());
				std::cout << "\n";
			}
		} catch (std::bad_variant_access e) {
			std::cout << " = ";
			printValue(v.value, v.type);
		}
	}
	std::cout << "\t\t\t\t\t\nFINISHED PRINTING STACK\n-----------------------------------\n\n" << std::endl;
}

void ScriptEngine::printVec3(const glm::vec3 &vec) {
	std::cout << std::format("{ {},{},{} }", vec.x, vec.y, vec.z);
}

void ScriptEngine::printValue(const data::Value &value, const data::Type &type) {
	try {
		std::get<data::Record>(value);
		std::cout << "Record";
	} catch (std::bad_variant_access e) {
		data::BasicType t = std::get<data::BasicType>(type);
		if (t == data::BasicType::Boolean)
			std::cout << std::get<bool>(value);
		else if (t == data::BasicType::String)
			std::cout << std::get<std::string>(value);
		else if (t == data::BasicType::Vec3)
			printVec3(std::get<glm::vec3>(value));
		else if (t == data::BasicType::Number)
			std::cout << std::get<double>(value);
		else
			std::cout << "UnsupportedType";
	}

}


InstructionType ScriptEngine::getInstructionType(const std::string &instruction) {
	for (std::string blockOp : blockOperators)
		if (instruction.find(blockOp) == 0) {
			// std::cout << "Found block operator " << blockOp << std::endl;
			return BLOCK_OPERATOR;
		}
	if (instruction.find('=') != std::string::npos) {
		// std::cout << "Found equal operator =" << std::endl;
		return ASSIGN;
	}
	return FUNCTION;
}


size_t ScriptEngine::indexOf(const std::string &systemName) {
	for (size_t i = 0; i < _systems.size(); i++)
		if (_systems[i].name == systemName)
			return i;
	return SIZE_MAX;
}


bool ScriptEngine::handleState(State lineState, LoadResult &result) {
	switch (lineState)
	{
	case START:
		return handleStart();
	case COMPONENT_DECL:
		return handleComponentDecl(result);
	case SYSTEM_DECL:
		return handleSystemDecl(result);
	case PROPERTY_DECL:
		return handlePropertyDecl();
	case INSTRUCTION:
		return handleInstructionDecl();
	default:
		throw UnexpectedStateException("ERROR", _line.data(), _currentLine, _currentState, lineState);
	}
}

bool ScriptEngine::handleStart() {
	return true;
}
bool ScriptEngine::handleComponentDecl(LoadResult &result) {
	int data = 0;
	if (_currentState == PROPERTY_DECL)
		registerComponent(result);
	if (_currentState == INSTRUCTION)
		registerSystem(result);
	std::vector<std::string> keyWords = split(_line, " \t");
	if (keyWords.size() != 2) // TODO: handle one line declaration of components
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid component declaration. Try declaring it like this : 'component Name'.");
	if (keyWords[0] != "component")
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Weird error.");
	_phComponent.name = keyWords[1];
	return true;
}
bool ScriptEngine::handleSystemDecl(LoadResult &result) {
	if (_currentState == PROPERTY_DECL)
		registerComponent(result);
	if (_currentState == INSTRUCTION)
		registerSystem(result);
	std::vector<std::string> keyWords = split(_line, " ");
	if (keyWords.size() < 2 || keyWords[0] != "system")
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Weird error.");
	auto systemComponentsA = _line.find('(');
	auto systemComponentsB = _line.find(')');
	auto eventComponentsA = _line.find('(', systemComponentsB + 1);
	auto eventComponentsB = _line.find(')', systemComponentsB + 1);
	if (systemComponentsA == std::string::npos || systemComponentsB == std::string::npos)
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "System declaration does not have any entity parameter. Provide at least one in the following form: 'system SystemName(entityName<Component1, Component2, ...>)' .");
	std::string expectedSystemComponents = _line.substr(systemComponentsA + 1, systemComponentsB - systemComponentsA - 1);
	std::string expectedEventComponents = _line.substr(eventComponentsA + 1, eventComponentsB - eventComponentsA - 1);
	std::vector<SystemParameter> systemParameters = getEntitiesFromSystem(expectedSystemComponents);
	event::Description eventDescription = {
		.name = "Tick",
        .payload = data::BasicType::Number
    };

	if (std::find(keyWords.begin(), keyWords.end(), "event") != keyWords.end() && std::find(keyWords.begin(), keyWords.end(), "event") + 1 != keyWords.end() && eventComponentsA != std::string::npos && eventComponentsB != std::string::npos && !expectedEventComponents.empty()) {
		std::vector<EventParameter> eventParameters = getEntitiesFromEvent(expectedEventComponents);
		std::string eventName = (std::find(keyWords.begin(), keyWords.end(), "event") + 1)[0];
		auto paren = eventName.find('(');
		if (paren != std::string::npos)
			eventName = eventName.substr(0, eventName.find('('));
		eventDescription.name = eventName;
		for (EventParameter &param : eventParameters) {
			if (param.type == "") {
				eventDescription.entities.push_back(param.name);
				_phSystem.eventComponents.push_back(param.components);
			} else {
				if (!variableTypes.contains(param.type))
					throw UnknownExpressionException("ERROR", _line.data(), _currentLine, "Unknown expression.");
				eventDescription.payload = variableTypes[param.type];
				eventDescription.payloadName = param.name;
			}
		}
	}
	auto endOfName = keyWords[1].find('(');
	if (endOfName != std::string::npos)
		_phSystem.name = keyWords[1].substr(0, endOfName);
	else
		_phSystem.name = keyWords[1];
	_phSystem.systemComponents = systemParameters[0].components; // TODO : make sure only one entity is input per system
	_phSystem.eventListener = eventDescription;
	// TODO : check if lamda or bind is more appropriate
	// _phSystem.system = [this](const pivot::ecs::systems::Description &d, pivot::ecs::systems::Description::systemArgs &a, const pivot::ecs::event::Event &e) {
	// 	this->executeSystemNew(d, a, e);
	// };
	_phSystem.system = std::bind(&ScriptEngine::executeSystemNew, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	_systemParameters.insert_or_assign(_phSystem.name, systemParameters[0]); // TODO : make sure only one entity is input per system
	return true;
}
bool ScriptEngine::handlePropertyDecl() {
	std::vector<std::string> keyWords = split(_line, " \t");
	if (keyWords.size() != 2) // TODO: handle one line declaration of components
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid property declaration. Try declaring it like this : 'propertyType propertyName' ex. 'Vector3 position'.");
	try {
		if (!variableTypes.contains(keyWords[0]))
			throw UnknownExpressionException("ERROR", _line.data(), _currentLine, "Unknown expression.");
		std::get<data::RecordType>(_phComponent.type)[keyWords[1]] = variableTypes[keyWords[0]];
	} catch (std::bad_variant_access e) {
		std::cerr << "Type of component is not RecordType. Should maybe use std::visit...\tScriptEngine::handlePropertyDecl()" << std::endl;
	}
	// _phProperty.type = keyWords[0];
	// _phProperty.name = keyWords[1];
	// _phComponent.properties.push_back(_phProperty);
	return true;
}
bool ScriptEngine::handleInstructionDecl() {
	_phInstructions.push_back(_line);
	return true;
}

void ScriptEngine::registerComponent(LoadResult &result, bool verbose){
	if (verbose) {
		std::cout << "Registering component : '" << _phComponent.name << "'\n";
		try {
			for (auto [name, type] : std::get<data::RecordType>(_phComponent.type))
				std::cout << "\t" << type << " Name { \"" << name <<  "\" }" << std::endl;
		} catch (std::bad_variant_access e) {
			std::cerr << "Type of component is not RecordType. Should maybe use std::visit...\tScriptEngine::handlePropertyDecl()" << std::endl;
		}
	}
	component::GlobalIndex::getSingleton().registerComponent(_phComponent);
	result.components.push_back(_phComponent);
	// _components.insert_or_assign(_phComponent.name, _phComponent);
	try {
		std::get<data::RecordType>(_phComponent.type).clear();
	} catch (std::bad_variant_access e) {
		std::cerr << "Type of component is not RecordType. Should maybe use std::visit...\tScriptEngine::handlePropertyDecl()" << std::endl;
	}
}

void ScriptEngine::registerSystem(LoadResult &result, bool verbose){
	if (verbose) {
		std::cout << "Registering system : '" << _phSystem.name << "'\t on event '" << _phSystem.eventListener.name << "'" << std::endl;
		for (std::string component : _phSystem.systemComponents)
			std::cout << "\t" << component << std::endl;
	}
	systems::GlobalIndex::getSingleton().registerSystem(_phSystem);
	result.systems.push_back(_phSystem);
	_phSystem.systemComponents.clear();
	_phSystem.eventComponents.clear();
	_systemsInstructions.push_back(_phInstructions);
	_phInstructions.clear();
}


bool ScriptEngine::badIndent(const std::string &line, State lineState) {
	Indent t = getIndent(line);
	if (!_fileIndentSet && t.type != NOINDENT) {
		_fileIndent.type = t.type;
		_fileIndent.size = t.size;
		_fileIndentSet = true;
		// std::cout << "Detected indent of file: " << (t.type == TAB ? "TABS" : "SPACES") << std::endl;
		return false;
	}
	if (t.type != NOINDENT && _fileIndent.type != NOINDENT && t.type != _fileIndent.type)
		throw BadIndentException("ERROR", line.data(), _currentLine, "Inconsistent use of tabs and spaces as indent.");
	if (!validIndentSize(t.size, lineState))
		throw BadIndentException("ERROR", line.data(), _currentLine, "Wrong number of indents.");
	return false;
}

bool ScriptEngine::validIndentSize(size_t indentSize, State lineState) {
	if (lineState == START || lineState == COMPONENT_DECL || lineState == SYSTEM_DECL)
		return (indentSize == 0);
	if (lineState == PROPERTY_DECL)
		return (indentSize == _fileIndent.size);
	if (lineState == INSTRUCTION) {
		if (_currentState == SYSTEM_DECL)
			return (indentSize == (_currentIndent + 1) * _fileIndent.size);
		return (indentSize == _currentIndent * _fileIndent.size || indentSize == (_currentIndent+1) * _fileIndent.size || indentSize == (_currentIndent-1) * _fileIndent.size);
	}
	std::cerr << "Error at: ScriptEngine::validIndentSize in ScriptEngine.cxx" << std::endl;
	return false;
}

size_t ScriptEngine::getLineIndent() {
	size_t totalIndent = getIndent(_line).size;

	if (_fileIndent.size == 0)
		return totalIndent;
	double indents = (double)(totalIndent / _fileIndent.size);
	return (size_t)indents;
}

State ScriptEngine::getLineState() {
	std::vector<std::string> keyWords = split(_line, " \t");

	if (keyWords.size() == 0)
		return INVALID;
	if (keyWords[0] == "component")
		return COMPONENT_DECL;
	if (keyWords[0] == "system")
		return SYSTEM_DECL;
	// if (std::find(variableTypes.begin(), variableTypes.end(), keyWords[0]) != variableTypes.end())
	// 	return PROPERTY_DECL;
	if (variableTypes.contains(keyWords[0]))
		return PROPERTY_DECL;
	if (_line.find('=') != std::string::npos || _line.find('(') != std::string::npos)
		return INSTRUCTION;
	return INVALID;
}

std::vector<SystemParameter> ScriptEngine::getEntitiesFromSystem(const std::string &line) {
	std::vector<SystemParameter> r;
	std::string str = line;
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
	while (!str.empty()) {
		SystemParameter p;
		auto componentStart = str.find('<');
		auto componentEnd = str.find('>');
		if (componentStart != std::string::npos && componentEnd != std::string::npos) {
			p.name = str.substr(0, componentStart);
			p.components = split(str.substr(componentStart + 1, componentEnd - (componentStart + 1)), ",");
		} else
			throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid system entity parameter component declaration. Try declaring it like this : 'entityName<Component1, Component2, ...>' .");
		r.push_back(p);
		str = str.substr(componentEnd + 1, str.length() - (componentEnd + 1));
	}
	return r;
}

std::vector<EventParameter> ScriptEngine::getEntitiesFromEvent(const std::string &line) {
	std::vector<EventParameter> r;
	std::string str = line;
	// str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
	while (!str.empty()) {
		EventParameter p;
		size_t nextParameter;
		std::string paramStr;
		auto componentStart = str.find('<');
		auto componentEnd = str.find('>');
		if (componentStart != std::string::npos && componentEnd != std::string::npos) {
			nextParameter = str.find(',', componentEnd + 1);
			if (nextParameter == std::string::npos)
				nextParameter = str.length() - 1;
			paramStr = str.substr(0, nextParameter + 1);
			paramStr.erase(remove_if(paramStr.begin(), paramStr.end(), isspace), paramStr.end());
			componentStart = paramStr.find('<');
			componentEnd = paramStr.find('>');
			p.name = paramStr.substr(0, componentStart);
			p.components = split(paramStr.substr(componentStart + 1, componentEnd - 2), ",");
			p.type = "";
		} else if (componentStart == std::string::npos && componentEnd == std::string::npos) {
			nextParameter = str.find(',');
			if (nextParameter == std::string::npos)
				nextParameter = str.length() - 1;
			paramStr = str.substr(0, nextParameter + 1);
			auto spl = split(paramStr, " ");
			if (spl.size() != 2)
				throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid event payload declaration. Try declaring it like this : 'payloadType payloadName' ex: 'Number deltatime'.");
			p.name = spl[1];
			p.components = {};
			p.type = spl[0];
		} else
			throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid event entity parameter component declaration. Try declaring it like this : 'entityName<Component1, Component2, ...>' or 'paramType param' .");
		str = str.substr(nextParameter + 1, str.length() - nextParameter - 1);
		r.push_back(p);
	}
	return r;
}

bool ScriptEngine::expectsState(State previous, State next) {
	switch (previous)
	{
	case START:
		if (next == COMPONENT_DECL || next == SYSTEM_DECL)
			return true;
		return false;
	case COMPONENT_DECL:
		if (next == PROPERTY_DECL)
			return true;
		return false;
	case SYSTEM_DECL:
		if (next == INSTRUCTION || next == COMPONENT_DECL || next == SYSTEM_DECL)
			return true;
		return false;
	case PROPERTY_DECL:
		if (next == PROPERTY_DECL || next == COMPONENT_DECL || next == SYSTEM_DECL)
			return true;
		return false;
	case INSTRUCTION:
		if (next == INSTRUCTION || next == COMPONENT_DECL || next == SYSTEM_DECL)
			return true;
		return false;
	default:
		return false;
	}
}


std::string script::StateToString(State s) {
	switch (s) {
		case START:
			return "START";
		case COMPONENT_DECL:
			return "COMPONENT_DECL";
		case SYSTEM_DECL:
			return "SYSTEM_DECL";
		case PROPERTY_DECL:
			return "PROPERTY_DECL";
		case INVALID:
			return "INVALID";
		default:
			return "UNKNOWN";
	};
}

bool ScriptEngine::populateLinesFromFile(const std::string &fileName) {
	_lines.clear();
	std::ifstream fin(fileName);

	if (fin.is_open() == false) {
		std::cerr << "Can't open file:\t" << fileName << std::endl;
		return false;
	}
	std::string line;
	while( std::getline(fin, line) ) {
		_lines.push_back(line);
	}
	return true;
}

void ScriptEngine::cleanLine(const std::string &line) {
	auto result = line.find('#');

	if (result != std::string::npos)
		_line = line.substr(0, result);
	else
		_line = line;
}

void ScriptEngine::cleanWhitespace() {
	std::replace(_line.begin(), _line.end(), '\t', ' ');
	auto cursor = _line.find("  ");
	while (cursor != std::string::npos) { // TODO : fix this disgusting loop
		_line = _line.substr(0, cursor) + _line.substr(cursor + 1, _line.length() - (cursor + 1));
		cursor = _line.find("  ");
	}
	// std::cout << "_line : " << _line << std::endl;
}

bool ScriptEngine::lineIsEmpty(const std::string &line) {
	if (line.find_first_not_of(" \t\r") != std::string::npos)
		return false;
	return true;
	// for (char c : line)
	// 	if (c != '\t' && c != ' ' && c != '\r')
	// 		return false;
	// return true;
}

Indent ScriptEngine::getIndent(const std::string &line) {
	Indent r {
		NOINDENT,
		0
	};
	for (char c : line) {
		if (c == ' ') {
			if (r.type == TAB)
				throw BadIndentException("ERROR", line.data(), _currentLine, "Simultaneous use of spaces and tabs as indent.");
			r.type = SPACE;
			r.size += 1;
		} else if (c == '\t') {
			if (r.type == SPACE)
				throw BadIndentException("ERROR", line.data(), _currentLine, "Simultaneous use of spaces and tabs as indent.");
			r.type = TAB;
			r.size += 1;
		} else
			break ;
	}
	return r;
}

long long ScriptEngine::timeThis(std::function<void()> toTime, const std::string &name, bool print) {
	auto start = _clock.now();
	toTime();
	auto end = _clock.now();
	if (print)
		std::cout << "-timeThis-- " << name << "\t" << std::chrono::duration(end - start).count() / 1000 << "us" << std::endl;
	return std::chrono::duration(end - start).count();
}


void script::replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
bool script::doubleSpacePredicate(char left, char right) {
	return ((left == right) && (left == ' '));
}

std::unique_ptr<pivot::ecs::component::IComponentArray> script::arrayFunctor(pivot::ecs::component::Description description) {
	return std::make_unique<component::ScriptingComponentArray>(description);
}
