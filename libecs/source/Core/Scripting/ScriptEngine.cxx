#include "pivot/ecs/Core/Scripting/ScriptEngine.hxx"

using namespace pivot::ecs;

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
		if (!expectsState(_currentState, lineState))
			throw UnexpectedStateException("ERROR", _line.data(), _currentLine, _currentState, lineState);
		if (badIndent(_line, lineState))
			throw BadIndentException("ERROR", _line.data(), _currentLine, "Bad indent");
		if (!handleState(lineState, result))
			throw UnexpectedStateException("ERROR", _line.data(), _currentLine, _currentState, lineState);
		_currentState = lineState;
		_currentIndent = getLineIndent();
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
catch (UnexpectedStateException e) {
	result.output = std::string("\n") + e.what() + ":\t" + fileName + "\n\tline " + std::to_string(e.get_line_nb()) + ":\t" + e.get_line() + "\nLogicError: ";
	if (e.get_new_state() == INVALID)
		result.output += "Unknown expression.\n";
	else
		result.output += "State " + StateToString(e.get_prev_state()) + " did not expect new state " + StateToString(e.get_new_state());
	if (verbose)
		std::cout << result.output << std::flush;
}
	if (result.output.empty())
		result.output = fileName + " succesfully parsed.\n";
	return result;
}

void ScriptEngine::executeSystem(const pivot::ecs::systems::Description &toExec, std::vector<std::vector<std::pair<ComponentDescription, std::any>>> &entities, const pivot::ecs::event::Event &event) {
try {
	size_t entityId = 0;
	for (SystemDescription &d : _systems) {
		if (toExec.name == d.name) {
			std::cout << "System to execute:\t" << d.name << "\n\n";
			std::cout << "Entities to execute on:\n";
			for (std::vector<std::pair<ComponentDescription, std::any>> &entity : entities) {
				std::cout << "\tEntity " << entityId << "\n";
				for (size_t i = 0; i < entity.size(); i++) {
					std::cout << "\t\tComponent " << entity[i].first.name << "\t";
					if (entity[i].first.name == "Position") {
						ComponentPosition p = std::any_cast<ComponentPosition>(entity[i].second);
						std::cout << p.pos_x << " " << p.pos_y << " " << p.pos_z;
					} else if (entity[i].first.name == "Velocity") {
						ComponentVelocity v = std::any_cast<ComponentVelocity>(entity[i].second);
						std::cout << v.vel_x << " " << v.vel_y << " " << v.vel_z;
					}
					std::cout << std::endl;
				}
				executeSystem(toExec.name, entity, entityId);
				entityId += 1;
			}
			break;
		}
	}
}
catch (UnhandledException e) {
	std::cout << e.what() << std::endl;
}
}

void executeSystemNew(const pivot::ecs::systems::Description &toExec, pivot::ecs::systems::Description::systemArgs &components, const pivot::ecs::event::Event &event) {

}

void ScriptEngine::executeSystem(const std::string &systemName, std::vector<std::pair<ComponentDescription, std::any>> &entity, size_t entityId) {
	std::cout << "Executing " << systemName << " on Entity " << entityId << std::endl;
	size_t toExec = indexOf(systemName);
	if (toExec == SIZE_MAX)
		throw UnhandledException("UNKNOWN ERROR: Couldn't find system in\tScriptEngine::executeSystem\tScriptEngin.cpp");
	for (auto &[description, component] : entity) {
		if (description.name == "Position") {
			ComponentPosition data = std::any_cast<ComponentPosition>(component);
			Variable v {
				.name = "Position",
				.type = "Struct",
				.value = nullptr
			};
			// for (Property p : description.properties) {
			// 	Variable vv {
			// 		.name = p.name,
			// 		.type = p.type,
			// 		.value = getField(component, description.name, p.name)
			// 	};
			// 	v.fields.push_back(vv);
			// }
			_variables.push_back(v);
		}
		else if (description.name == "Velocity") {
			ComponentVelocity data = std::any_cast<ComponentVelocity>(component);
			Variable v {
				.name = "Velocity",
				.type = "Struct",
				.value = nullptr
			};
			// for (Property p : description.properties) {
			// 	Variable vv {
			// 		.name = p.name,
			// 		.type = p.type,
			// 		.value = getField(component, description.name, p.name)
			// 	};
			// 	v.fields.push_back(vv);
			// }
			_variables.push_back(v);
		}
		else
			throw UnhandledException("UNKNOWN ERROR: Invalid description.name in\tScriptEngine::executeSystem\tScriptEngin.cpp");
	}
	printStack();
	for (std::string instruction : _systemsInstructions[toExec]) {
		std::cout << "Instruction:\t'" << instruction << "' to execute." << std::endl;
		cleanInstruction(instruction);
		InstructionType iType = getInstructionType(instruction);
		handleInstruction(instruction, iType);
	}
}

void ScriptEngine::handleInstruction(const std::string &instruction, InstructionType iType) {
	switch (iType)
	{
	case ASSIGN:
	{
		size_t equalPos = instruction.find('=');
		if (equalPos == std::string::npos)
			throw UnhandledException("UNKNOWN ERROR: No equal sign in ASSIGN instruction");
		std::string left = instruction.substr(0, equalPos);
		std::string right = instruction.substr(equalPos + 1, instruction.size() - equalPos - 1);
		std::cout << "Left [" << left << "] Right [" << right << "]" << std::endl;
		std::cout << "Is left solvable: " << (isSolvable(left) ? "yes" : "no") << std::endl;
		break;
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
}

bool ScriptEngine::isSolvable(const std::string &toSolve) {
	std::string mut(toSolve);
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
	std::cout << "-----------------------------------\t\t\t\t\t\nPRINTING STACK\n\n" << std::endl;
	for (Variable v : _variables) {
		std::cout << "\t" << v.name << " : " << v.type;
		if (v.fields.size() == 0) {
			std::cout << " = ";
			printValue(v.value, v.type);
		} else {
			for (Variable field : v.fields) {
				std::cout << "\n\t\t" << field.name << " : " << field.type << " = ";
				printValue(field.value, field.type);
				std::cout << "\n";
			}

		}
	}
	std::cout << "\t\t\t\t\t\nFINISHED PRINTING STACK\n-----------------------------------\n\n" << std::endl;
}

void ScriptEngine::printValue(const std::any &value, const std::string &type) {
	if (type == "Struct")
		std::cout << "null";
	else if (type == "Number")
		std::cout << std::any_cast<double>(value);
	else if (type == "String")
		std::cout << std::any_cast<std::string>(value);
	// else if (type == "Vector3")
	// 	std::cout << std::any_cast<glm::vec3>(value);
}


InstructionType ScriptEngine::getInstructionType(const std::string &instruction) {
	for (std::string blockOp : blockOperators)
		if (instruction.find(blockOp) == 0) {
			std::cout << "Found block operator " << blockOp << std::endl;
			return BLOCK_OPERATOR;
		}
	if (instruction.find('=') != std::string::npos) {
		std::cout << "Found equal operator =" << std::endl;
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
	std::vector<std::string> keyWords = split(_line, " \t");
	// size_t firstInd = _line.find_first_of('{');
	// std::string componentsString = _line.substr(firstInd + 1, _line.find_last_of('}') - firstInd - 1);
	// if (componentsString.empty())
	// 	throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "System requires input components declared in brackets.\nSuch as\tsystem Name {component1 component2}");
	if (keyWords[0] != "system")
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Weird error.");
	_phSystem.name = keyWords[1];
	_phSystem.systemComponents = {""};
	event::Description eventDescription{
        .name = "Tick",
        .entities = {
            {}
        },
        .payload = data::BasicType::Number,
    };
	_phSystem.eventListener = eventDescription;
	_phSystem.eventComponents = { {"A", "B"}, {"C"}};
	return true;
}
bool ScriptEngine::handlePropertyDecl() {
	std::vector<std::string> keyWords = split(_line, " \t");
	if (keyWords.size() != 2) // TODO: handle one line declaration of components
		throw InvalidSyntaxException("ERROR", _line.data(), _currentLine, "Invalid property declaration. Try declaring it like this : 'propertyType propertyName' ex. 'Vector3 position'.");
	try {
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
	try {
		std::get<data::RecordType>(_phComponent.type).clear();
	} catch (std::bad_variant_access e) {
		std::cerr << "Type of component is not RecordType. Should maybe use std::visit...\tScriptEngine::handlePropertyDecl()" << std::endl;
	}
}

void ScriptEngine::registerSystem(LoadResult &result, bool verbose){
	if (verbose) {
		std::cout << "Registering system : '" << _phSystem.name << "'\t on event " << _phSystem.eventListener.name << std::endl;
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

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find_first_of(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + 1;
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}


std::string StateToString(State s) {
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
	auto cursor = _line.find_first_not_of(" \t") + 1;
	std::replace(_line.begin() + cursor, _line.end(), '\t', ' ');
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

std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description) {
	return std::make_unique<component::ScriptingComponentArray>(description);
}
