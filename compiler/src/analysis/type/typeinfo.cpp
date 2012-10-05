#include "typeinfo.h"

#include <parser/ast/ast.h>

#include <stdexcept>

using namespace juli;

juli::TypeInfo::TypeInfo() {
	// initialize primitive types:
	typeTable["double"] = new PrimitiveType(FLOAT64);
	typeTable["void"] = new PrimitiveType(VOID);
	typeTable["int"] = new PrimitiveType(INT32);
	typeTable["char"] = new PrimitiveType(INT8);
	typeTable["boolean"] = new PrimitiveType(BOOLEAN);
}

void juli::TypeInfo::defineFunction(const NFunctionDefinition* def) {
	Function* f = new Function(def, *this);
	declareFunction(f);
}

void juli::TypeInfo::defineClass(const NClassDefinition* def) {
	std::vector<Field> fields;
	int index = 0;
	for (FieldList::const_iterator i = def->fields.begin();
			i != def->fields.end(); ++i, ++index) {
		fields.push_back(
				Field((*i)->name->name, index, (*i)->type->resolve(*this)));
	}

	Type* type = typeTable[def->name->name];
	static_cast<ClassType*>(type)->addFields(fields);
}

void juli::TypeInfo::declareClass(const NClassDefinition* def) {
	std::vector<Field> fields;
	ClassType* type = new ClassType(def->name->name, fields);

	std::map<std::string, Type*>::iterator i = typeTable.find(def->name->name);
	if (i != typeTable.end()) {
		CompilerError err(def);
		err.getStream() << "Redefinition of type " << def->name->name;
		throw err;
	}

	typeTable[def->name->name] = type;
	unresolvedTypes[def->name->name] = def;
}

void juli::TypeInfo::resolveClasses() {
	for (std::map<std::string, const NClassDefinition*>::iterator i =
			unresolvedTypes.begin(); i != unresolvedTypes.end(); ++i) {
		defineClass(i->second);
	}
}

void juli::TypeInfo::declareFunction(Function* f) {
	functions.addFunction(f);
}

Function* juli::TypeInfo::resolveFunction(const std::string& name,
		std::vector<const Type*>& argTypes, const Indentable* astNode) const
				throw (CompilerError) {
	std::vector<Function*> matches;
	try {
		matches = functions.resolve(name, argTypes);
	} catch (std::out_of_range& e) {
	}
	if (matches.empty()) {
		CompilerError err(astNode);
		err.getStream() << "Undeclared function: " << name << " " << argTypes;
		throw err;
	} else if (matches.size() > 1) {
		CompilerError err(astNode);
		err.getStream() << "Ambiguous Function Call: " << name << "("
				<< argTypes << ")" << std::endl << "Candidates are: "
				<< std::endl;
		for (std::vector<Function*>::iterator i = matches.begin();
				i != matches.end(); ++i) {
			err.getStream() << *i << std::endl;
		}

		throw err;
	}
	return *matches.begin();
}

const Functions& juli::TypeInfo::getFunctions() const {
	return functions;
}

const std::vector<Type*> juli::TypeInfo::getTypes() const {
	std::vector<Type*> result;
	for (std::map<std::string, Type*>::const_iterator i = typeTable.begin();
			i != typeTable.end(); ++i) {
		result.push_back(i->second);
	}
	return result;
}

const Type* juli::TypeInfo::getType(const std::string& name,
		const Indentable* astNode) const throw (CompilerError) {
	try {
		return typeTable.at(name);
	} catch (std::out_of_range& e) {
		CompilerError err(astNode);
		err.getStream() << "Unknown type '" << name << "'";
		throw err;
	}
	return 0;
}

void juli::TypeInfo::dump() const {
	std::cout << "FUNCTIONS: " << std::endl;
	functions.dump();
	std::cout << "TYPES: " << std::endl;
	for (std::map<std::string, Type*>::const_iterator i = typeTable.begin();
			i != typeTable.end(); ++i) {
		std::cout << i->first << ": " << i->second << std::endl;
	}
}
