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

void juli::TypeInfo::declareFunction(const NFunctionDefinition* f) {
	functions[f->signature->name] = f;
}

const NFunctionDefinition* juli::TypeInfo::getFunction(
		const std::string& name) const throw (CompilerError) {
	try {
		return functions.at(name);
	} catch (std::out_of_range& e) {
		return 0;
	}
	return 0;
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
