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
	declareFunction(f);
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
	for (std::map<std::string, Type*>::const_iterator i = typeTable.begin(); i != typeTable.end(); ++i) {
		std::cout << i->first << ": " << i->second << std::endl;
	}
}
