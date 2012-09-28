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
}

void juli::TypeInfo::declareFunction(const NFunctionDefinition* def) {
	Function* f = new Function(def, *this);
	functions.addFunction(f);
}

std::vector<Function*> juli::TypeInfo::resolveFunction(const std::string& name, std::vector<const Type*>& argTypes) const throw (CompilerError) {
	try {
		return functions.resolve(name, argTypes);
	} catch (std::out_of_range& e) {
		return std::vector<Function*>();
	}
	return std::vector<Function*>();
}

const Functions& juli::TypeInfo::getFunctions() const {
	return functions;
}

const Type* juli::TypeInfo::getType(const std::string& name, const Indentable* astNode) const
		throw (CompilerError) {
	try {
		return typeTable.at(name);
	} catch (std::out_of_range& e) {
		CompilerError err(astNode);
		err.getStream() << "Unknown type '" << name << "'";
		throw err;
	}
	return 0;
}
