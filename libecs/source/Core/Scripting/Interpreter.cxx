#include "pivot/ecs/Core/Scripting/Interpreter.hxx"
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

// Public functions ( can be called anywhere )

// This will go through a file's tree and register all component/system declarations into the global index
std::vector<systems::Description> registerDeclarations(const Node &file, component::Index &componentIndex) {
	std::vector<systems::Description> result;
	if (file.type != NodeType::File) {
		std::cerr << std::format("registerDeclarations(const Node &file): can't interpret node {} (not a file)", gNodeTypeStrings.at(file.type)) << std::endl;
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
				std::cerr << std::format("registerDeclarations(const Node &file): failed to interpret sub node {}", gNodeTypeStrings.at(node.type)) << std::endl;
				return result;
			}
			result.push_back(r);
		} else { // ??
			std::cerr << std::format("registerDeclarations(const Node &file): can't interpret sub node {} (not a component nor a system)", gNodeTypeStrings.at(node.type)) << std::endl;
			return result;
		}
	}
} catch (TokenException e) {
	std::cerr << std::format("\nInterpreter {} {}:\t{}\n\tline {} char {}:\t{}\n{}",
		e.what(), e.get_exctype(), file.value, e.get_line_nb(), e.get_char_nb(), e.get_token(), e.get_info()) << std::endl;
} catch(std::exception e) {
	std::cerr << std::format("Unhandled exception in file {}: {}", file.value, e.what()) << std::endl;
}
	return result;
}


// Private functions (never called elsewhere than this file and tests)

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
		throw TokenException("ERROR", children.at(childIndex - 1).value, children.at(childIndex - 1).line_nb, children.at(childIndex - 1).char_nb, "Unexpected_EndOfFile", "Expected token " + gNodeTypeStrings.at(expectedType));
	const Node &node = children.at(childIndex);
	if (node.type != expectedType)
		throw TokenException("ERROR", node.value, node.line_nb, node.char_nb, "UnexpectedNodeType", "Expected type " + gNodeTypeStrings.at(expectedType));
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
