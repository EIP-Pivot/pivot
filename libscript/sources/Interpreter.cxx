#include "pivot/script/Interpreter.hxx"
#include "pivot/script/Exceptions.hxx"
#include "pivot/script/Builtins.hxx"
#include "Logger.hpp"
#include <unordered_map>


namespace pivot::ecs::script::interpreter {

// TODO: consider using magicenum
const std::map<NodeType, std::string> gNodeTypeStrings = {
	{	NodeType::File,	"File"	},
	{	NodeType::ComponentDeclaration,	"ComponentDeclaration"	},
	{	NodeType::SystemDeclaration,	"SystemDeclaration"	},
	{	NodeType::ComponentName,	"ComponentName"	},
	{	NodeType::SystemName,	"SystemName"	},
	{	NodeType::PropertyType,	"PropertyType"	},
	{	NodeType::PropertyName,	"PropertyName"	},
	{	NodeType::EventKeyword,	"EventKeyword"	},
	{	NodeType::EventName,	"EventName"	},
	{	NodeType::EventPayloadType,	"EventPayloadType"	},
	{	NodeType::EventPayloadName,	"EventPayloadName"	},
	{	NodeType::EntityParameterName,	"EntityParameterName"	},
	{	NodeType::EntityParameterComponent,	"EntityParameterComponent"	},
	{	NodeType::Symbol,	"Symbol"	}
};
const std::map<std::string, data::BasicType> gVariableTypes {
	{"Vector3", data::BasicType::Vec3},
	{"Number", data::BasicType::Number},
	{"Boolean", data::BasicType::Boolean},
	{"Color", data::BasicType::Number},
	{"String", data::BasicType::String}
};

// Map built-in function strings, to their callback and parameters (number and types)
const std::unordered_map<
	std::string, std::pair<
		std::function<
			data::Value(const std::vector<data::Value> &)
		>, std::pair<size_t, std::vector<data::Type>>
	>
> gBuiltinsCallbacks = {
	{	"isPressed", {interpreter::builtins::builtin_isPressed, { 1, {data::BasicType::String }}}},
	{	"print", {interpreter::builtins::builtin_print, { 1, {data::BasicType::String }}}}
};


// Public functions ( can be called anywhere )

// This will go through a file's tree and register all component/system declarations into the global index
std::vector<systems::Description> registerDeclarations(const Node &file, component::Index &componentIndex) {
	std::vector<systems::Description> result;
	if (file.type != NodeType::File) {
		// std::cerr << std::format("registerDeclarations(const Node &file): can't interpret node {} (not a file)", gNodeTypeStrings.at(file.type)) << std::endl; // format not available in c++20 gcc yet
		logger.warn("registerDeclarations(const Node &file): can't interpret node ") << gNodeTypeStrings.at(file.type) << " (not a file)";
		return result;
	}
try { // handle exceptions from register functions
	// Loop through every declaration in the file
	for (const Node &node : file.children) {
		if (node.type == NodeType::ComponentDeclaration) { // register component
			registerComponentDeclaration(node, componentIndex, file.value);
		} else if (node.type == NodeType::SystemDeclaration) { // store system declaration for return
			systems::Description r = registerSystemDeclaration(node, file.value);
			if (r.name == "Error 1") {
				// std::cerr << std::format("registerDeclarations(const Node &file): failed to interpret sub node {}", gNodeTypeStrings.at(node.type)) << std::endl; // format not available in c++20 gcc yet
				logger.err("registerDeclarations(const Node &file): failed to register declarations for sub node ") << gNodeTypeStrings.at(node.type);
				return result;
			}
			result.push_back(r);
		} else { // ??
			// std::cerr << std::format("registerDeclarations(const Node &file): can't interpret sub node {} (not a component nor a system)", gNodeTypeStrings.at(node.type)) << std::endl; // format not available in c++20 gcc yet
			logger.err("registerDeclarations(const Node &file): can't register declarations for sub node ") << gNodeTypeStrings.at(node.type) << " (not a component nor a system)";
			return result;
		}
	}
} catch (TokenException e) {
	// std::cerr << std::format("\nInterpreter {} {}:\t{}\n\tline {} char {}:\t{}\n{}",
	// 	e.what(), e.get_exctype(), file.value, e.get_line_nb(), e.get_char_nb(), e.get_token(), e.get_info()) << std::endl; // format not available in c++20 gcc yet
	logger.err("\nInterpreter ") << e.what() << " " << e.get_exctype() << ":\t" << file.value <<
		"\n\tline " << e.get_line_nb() << " char " << e.get_char_nb() << ":\t" << e.get_token() <<
		"\n" << e.get_info();
} catch(std::exception e) {
	// std::cerr << std::format("Unhandled exception in file {}: {}", file.value, e.what()) << std::endl; // format not available in c++20 gcc yet
	logger.err("Unhandled exception in file ") << file.value << ": " << e.what();
}
	return result;
}

// This will execute a SystemEntryPoint node by executing all of its statements
void executeSystem(const Node &systemEntry, component::ArrayCombination::ComponentCombination &entity, const event::EventWithComponent &trigger, Stack &stack) {
	std::cout << "Executing block " << systemEntry.value << std::endl;
	for (const Node &statement : systemEntry.children) { // execute all statements
		if (statement.value == "functionCall") {
			if (statement.children.size() != 2) // should be [Variable, FunctionParams]
				throw InvalidException("InvalidFunctionStatement", statement.value, "Expected callee and params children node.");
			const Node &callee = statement.children.at(0);
			if (callee.children.size() != 1 || !gBuiltinsCallbacks.contains(callee.value)) // Only support builtin functions for now
				throw InvalidException("UnknownFunction", callee.value, "Pivotscript only supports built-in functions for now.");
			std::vector<data::Value> parameters;
			for (const Node &param : statement.children.at(1).children) { // get all the parameters for the callback
				if (param.type == NodeType::ExistingVariable) { // Named variable
					const Variable &v = stack.find(param.value); // find it in the stack 
					if (!v.hasValue)
						throw InvalidException("InvalidVariable", v.name, "Variable has no value (not been initialized, or has members).");
					parameters.push_back(v.value); // and add it to the parameters
				} else if (param.type == NodeType::LiteralNumberVariable) // double
					parameters.push_back(data::Value(std::stod(param.value))); // if type is LiteralNumber, script::parser caught std::stod exceptions
				else if (param.type == NodeType::DoubleQuotedStringVariable) // "string"
					parameters.push_back(data::Value(param.value));
				else
					throw InvalidException("UnsupportedFeature", param.value, "This type of variable is not supported yet.");
			}
			// validate the parameters and call the callback for the built-in function
			validateParams(parameters, gBuiltinsCallbacks.at(callee.value).second.first, gBuiltinsCallbacks.at(callee.value).second.second, callee.value); // pair is <size_t numberOfParams, vector<data::Type> types>
			gBuiltinsCallbacks.at(callee.value).first(parameters);

		} else if (statement.value == "if") {
		} else if (statement.value == "while") {
		} else if (statement.value == "assign") {
		} else { // unsupported yet
			throw InvalidException("InvalidStatement", statement.value, "Unsupported statement.");
		}
	}
}


// Private functions (never called elsewhere than this file and tests)

// Validate the parameters for a builtin
void validateParams(const std::vector<data::Value> &toValidate, size_t expectedSize, const std::vector<data::Type> &expectedTypes, const std::string &name) {
	// check expected size
	if (toValidate.size() != expectedSize)
		throw InvalidException("BadNumberOfParameters", name,
			"Wrong number of parameters " + std::to_string(toValidate.size()) + " (expected " + std::to_string(expectedSize) + ")");
	// check expected types (assume expectedTypes is of right size)
	for (size_t i = 0; i < expectedSize; i++)
		if (toValidate.at(i).type() != expectedTypes.at(i))
			throw InvalidException("BadParameter", name,
				"Bad parameter type " + toValidate.at(i).type().toString() + " (expected " + expectedTypes.at(i).toString() + ")");
}

// Register a component declaration node
void registerComponentDeclaration(const Node &component, component::Index &componentIndex, const std::string &fileName) {
	pivot::ecs::component::Description r = {
		.name = component.value,
		.type = data::BasicType::Boolean,
		.createContainer = arrayFunctor
	};
	// Either it is a one line component
	if (component.children.at(0).type == NodeType::Symbol) { // we know children is not empty from parser::ast_from_file(const std::string &filename)
		const Node &componentTypeNode = component.children.at(1);

		if (!gVariableTypes.contains(componentTypeNode.value)) // Not a known pivotscript type
			throw TokenException("ERROR", componentTypeNode.value, componentTypeNode.line_nb, componentTypeNode.char_nb, "UnknownType", "This is not a PivotScript type.");

		// Type of component is data::BasicType
		r.type = gVariableTypes.at(componentTypeNode.value);
	} else { // or a multi-line component
		// Type of component is data::RecordType
		r.type = data::RecordType{};

		for (size_t i = 0; i < component.children.size() - 1; i += 2) { // Loop over every property of the component
			const Node &propertyTypeNode = component.children.at(i);
			const Node &propertyNameNode = component.children.at(i + 1);
			if (!gVariableTypes.contains(propertyTypeNode.value)) // Not a known pivotscript type
				throw TokenException("ERROR", propertyTypeNode.value, propertyTypeNode.line_nb, propertyTypeNode.char_nb, "UnknownType", "This is not a PivotScript type.");
			// Push the property in the record type of the description
			std::get<data::RecordType>(r.type)[propertyNameNode.value] = gVariableTypes.at(propertyTypeNode.value);
		}
	}
	r.provenance = Provenance::externalRessource(fileName);
	componentIndex.registerComponent(r);
}
// Register a system declaration node
systems::Description registerSystemDeclaration(const Node &system, const std::string &fileName) {
	pivot::ecs::systems::Description sysDesc = {
		.name = system.value
	};
	size_t nbChildren = system.children.size();
	if (nbChildren < 6) { // ??minimum system declaration node??
		std::cerr << "registerSystemDeclaration(const Node &system): Invalid system, not enough children for declaration" << std::endl;
		sysDesc.name = "Error 1";
		return sysDesc;
	}
	pivot::ecs::event::Description evtDesc;
	size_t cursor = 0;
	expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "(", true);
	while (cursor < nbChildren && system.children.at(cursor).value != ")") {
		consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EntityParameterName);
		expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "<", true);
		while (cursor < nbChildren && system.children.at(cursor).value != ">") { // consume all that entity's components
			consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EntityParameterComponent);
			if (cursor < nbChildren && system.children.at(cursor).value == ">") // no more ',', end of loop
				break;
			expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
		}
		expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ">", true);
		if (cursor < nbChildren && system.children.at(cursor).value == ")") // no more ',', end of loop
			break;
		expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
	}
	expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ")", true);
	if (cursor == nbChildren || system.children.at(cursor).value != "event") { // Default Tick event
		evtDesc.name = "Tick";
		evtDesc.payload = data::BasicType::Number;
		// evtDesc.payloadName = "deltaTime"; // TODO: payload name
		evtDesc.provenance = Provenance();
	} else { // Custom event
		expectNodeTypeValue(system.children, cursor, NodeType::EventKeyword, "event", true);
		consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventName);
		expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "(", true);
		while (cursor < nbChildren - 1 && system.children.at(cursor).value != ")") { // consume all event parameters
			if (system.children.at(cursor + 1).value == "<") { // Entity parameter
				sysDesc.eventComponents.push_back({});
				consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventEntityName);
				expectNodeTypeValue(system.children, cursor, NodeType::Symbol, "<", true);
				while (cursor < nbChildren && system.children.at(cursor).value != ">") { // consume all event entity components
					consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventEntityComponent);
					if (cursor < nbChildren && system.children.at(cursor).value == ">") // no more ',', end of loop
						break;
					expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ",", true);
				}
				expectNodeTypeValue(system.children, cursor, NodeType::Symbol, ">", true);
			} else { // payload
				consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventPayloadType);
				consumeNode(system.children, cursor, sysDesc, evtDesc, NodeType::EventPayloadName);
			}
			if (cursor < nbChildren && system.children.at(cursor).value == ")") // no more ',', end of loop
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
void consumeNode(const std::vector<Node> &children, size_t &childIndex, systems::Description &sysDesc, event::Description &evtDesc, NodeType expectedType) {
	if (childIndex >= children.size())
		throw TokenException("ERROR", children.at(childIndex - 1).value, children.at(childIndex - 1).line_nb, children.at(childIndex - 1).char_nb, "Unexpected_EndOfFile", "Expected token " + gNodeTypeStrings.at(expectedType));
	const script::Node &node = children.at(childIndex);
	if (node.type != expectedType)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeType", "Expected type " + gNodeTypeStrings.at(expectedType));
	switch (expectedType)
	{
	case NodeType::EntityParameterName:
		// TODO : keep record of entity parameter names for execution later
		break;
	case NodeType::EntityParameterComponent :
		sysDesc.systemComponents.push_back(node.value);
		break;
	case NodeType::EventName:
		evtDesc.name = node.value;
		break;
	case NodeType::EventEntityName:
		evtDesc.entities.push_back(node.value);
		break;
	case NodeType::EventEntityComponent:
		sysDesc.eventComponents.back().push_back(node.value);
		break;
	case NodeType::EventPayloadName:
		// TODO : ask maxence to put payload name back in description
		break;
	case NodeType::EventPayloadType:
		if (!gVariableTypes.contains(node.value)) // Not a known pivotscript type
			throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnknownType", "This is not a PivotScript type.");
		evtDesc.payload = gVariableTypes.at(node.value);
		break;
	default:
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeType", "Expected type " + gNodeTypeStrings.at(expectedType));
	}
	childIndex++;
}
// Throw if the type or value of the targeted node isn't equal to the expected type and value
void expectNodeTypeValue(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, const std::string &expectedValue, bool consume) {
	if (childIndex == children.size())
		throw TokenException("ERROR", children.at(childIndex - 1).value, children.at(childIndex - 1).line_nb, children.at(childIndex - 1).char_nb, "Unexpected_EndOfFile", "Expected token of type " + gNodeTypeStrings.at(expectedType));
	const Node &node = children.at(childIndex);
	if (node.type != expectedType)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeType", "Expected type " + gNodeTypeStrings.at(expectedType));
	if (node.value != expectedValue)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeValue", "Expected value " + expectedValue);
	if (consume)
		childIndex++;
}
// Throw if the type of the targeted node isn't equal to the expected type
void expectNodeType(const std::vector<Node> &children, size_t &childIndex, NodeType expectedType, bool consume) {
	if (childIndex == children.size())
		throw TokenException("ERROR", children.at(childIndex - 1).value, children.at(childIndex - 1).line_nb, children.at(childIndex - 1).char_nb, "Unexpected_EndOfFile", "Expected token of type " + gNodeTypeStrings.at(expectedType));
	const Node &node = children.at(childIndex);
	if (node.type != expectedType)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeType", "Expected type " + gNodeTypeStrings.at(expectedType));
	if (consume)
		childIndex++;
}
// Throw if the value of the targeted node isn't equal to the expected value
void expectNodeValue(const std::vector<Node> &children, size_t &childIndex, const std::string &expectedValue, bool consume) {
	if (childIndex == children.size())
		throw TokenException("ERROR", children.at(childIndex - 1).value, children.at(childIndex - 1).line_nb, children.at(childIndex - 1).char_nb, "UnexpectedNodeValue", "Expected value " + expectedValue);
	const Node &node = children.at(childIndex);
	if (node.value != expectedValue)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeValue", "Expected value " + expectedValue);
	if (consume)
		childIndex++;
}

// Callback to create a container for components
std::unique_ptr<pivot::ecs::component::IComponentArray> arrayFunctor(pivot::ecs::component::Description description) {
	return std::make_unique<component::ScriptingComponentArray>(description);
}

} // end of namespace pivot::ecs::script::interpreter
