#include "typeinfo.h"

#include <parser/ast/ast.h>

#include <stdexcept>

using namespace juli;

std::map<std::string, Type*> juli::TypeInfo::implicitTypes;

juli::TypeInfo::TypeInfo(std::vector<Error>& errors, bool implicit) : CompilerComponent(errors) {
	if (implicitTypes.empty()) {
		implicitTypes["double"] = new PrimitiveType(FLOAT64);
		implicitTypes["void"] = new PrimitiveType(VOID);
		implicitTypes["int"] = new PrimitiveType(INT32);
		implicitTypes["char"] = new PrimitiveType(INT8);
		implicitTypes["boolean"] = new PrimitiveType(BOOLEAN);
	}

	// initialize primitive types:
	mergeMaps(typeTable, implicitTypes);
	if (implicit) {
		// implicit declarations:
		std::vector<std::string> comparison;
		comparison.push_back(">");
		comparison.push_back("<");
		comparison.push_back(">=");
		comparison.push_back("<=");
		comparison.push_back("==");
		comparison.push_back("!=");
		declareImplicitOperator(comparison, &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::INT8_TYPE, 2);
		declareImplicitOperator(comparison, &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::INT32_TYPE, 2);
		declareImplicitOperator(comparison, &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::FLOAT64_TYPE, 2);

		std::vector<std::string> equality;
		equality.push_back("==");
		equality.push_back("!=");
		declareImplicitOperator(equality, &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::BOOLEAN_TYPE, 2);
		declareImplicitOperator(equality, &PrimitiveType::BOOLEAN_TYPE, &ReferenceType::REFERENCE_TYPE, 2);

		std::vector<std::string> arithmetic;
		arithmetic.push_back("+");
		arithmetic.push_back("-");
		arithmetic.push_back("/");
		arithmetic.push_back("*");
		declareImplicitOperator(arithmetic, &PrimitiveType::INT8_TYPE, &PrimitiveType::INT8_TYPE, 2);
		declareImplicitOperator(arithmetic, &PrimitiveType::INT32_TYPE, &PrimitiveType::INT32_TYPE, 2);
		declareImplicitOperator(arithmetic, &PrimitiveType::FLOAT64_TYPE, &PrimitiveType::FLOAT64_TYPE, 2);

		declareImplicitOperator("%", &PrimitiveType::INT8_TYPE, &PrimitiveType::INT8_TYPE, 2);
		declareImplicitOperator("%", &PrimitiveType::INT32_TYPE, &PrimitiveType::INT32_TYPE, 2);
		declareImplicitOperator("%", &PrimitiveType::FLOAT64_TYPE, &PrimitiveType::FLOAT64_TYPE, 2);

		std::vector<std::string> logical;
		logical.push_back("and");
		logical.push_back("or");
		declareImplicitOperator(logical, &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::BOOLEAN_TYPE, 2);

		declareImplicitOperator("not", &PrimitiveType::BOOLEAN_TYPE, &PrimitiveType::BOOLEAN_TYPE, 1);

		declareImplicitOperator("-", &PrimitiveType::INT8_TYPE, &PrimitiveType::INT8_TYPE, 1);
		declareImplicitOperator("-", &PrimitiveType::INT32_TYPE, &PrimitiveType::INT32_TYPE, 1);
		declareImplicitOperator("-", &PrimitiveType::FLOAT64_TYPE, &PrimitiveType::FLOAT64_TYPE, 1);

		declareImplicitOperator("~", &PrimitiveType::INT8_TYPE, &PrimitiveType::INT8_TYPE, 1);
		declareImplicitOperator("~", &PrimitiveType::INT32_TYPE, &PrimitiveType::INT32_TYPE, 1);
		declareImplicitOperator("~", &PrimitiveType::FLOAT64_TYPE, &PrimitiveType::FLOAT64_TYPE, 1);
	}
}

void juli::TypeInfo::declareImplicitOperator(const std::vector<std::string> names, const Type* returnType,
		const Type* type, unsigned int arity) {
	for (std::vector<std::string>::const_iterator i = names.begin(); i != names.end(); ++i) {
		declareImplicitOperator(*i, returnType, type, arity);
	}
}

void juli::TypeInfo::declareImplicitOperator(const std::string& name, const Type* returnType, const Type* type,
		unsigned int arity) {
	std::vector<FormalParameter> binaryArgs;
	for (unsigned int i = 0; i < arity; ++i) {
		std::stringstream s;
		s << "x" << i;
		binaryArgs.push_back(FormalParameter(type, s.str()));
	}

	Function* f = Function::get(name, returnType, binaryArgs, false, 0, errors);
    if (f)
        declareFunction(f);

}

void juli::TypeInfo::declareImplicitOperator(const std::string& name, const Type* type, unsigned int arity) {
	declareImplicitOperator(name, type, type, arity);
}

void juli::TypeInfo::defineFunction(const NFunctionDefinition* def, bool importing) {
	Function* f = Function::get(def, *this, importing, errors);
    if (f)
        declareFunction(f);
}

void juli::TypeInfo::defineClass(const NClassDefinition* def) {
	std::vector<Field> fields;
	int index = 0;
	for (FieldList::const_iterator i = def->fields.begin(); i != def->fields.end(); ++i, ++index) {
		fields.push_back(Field((*i)->name->name, index, (*i)->type->resolve(*this)));
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
		errors.push_back(err);
        return;
	}

	typeTable[def->name->name] = type;
	unresolvedTypes[def->name->name] = def;
}

void juli::TypeInfo::resolveClasses() {
	for (std::map<std::string, const NClassDefinition*>::iterator i = unresolvedTypes.begin();
			i != unresolvedTypes.end(); ++i) {
		defineClass(i->second);
	}
}

void juli::TypeInfo::declareFunction(Function* f) {
	functions.addFunction(f);
}

Function* juli::TypeInfo::resolveFunction(const std::string& name, std::vector<const Type*>& argTypes,
		const Indentable* astNode) const throw (CompilerError) {
	std::vector<Function*> matches;
    matches = functions.resolve(name, argTypes);
	if (matches.empty()) {
		CompilerError err(astNode);
		err.getStream() << "Undeclared function: " << name << " " << argTypes;
		errors.push_back(err);
        return 0;
	} else if (matches.size() > 1) {
		CompilerError err(astNode);
		err.getStream() << "Ambiguous Function Call: " << name << "(" << argTypes << ")" << std::endl
				<< "Candidates are: " << std::endl;
		for (std::vector<Function*>::iterator i = matches.begin(); i != matches.end(); ++i) {
			err.getStream() << *i << std::endl;
		}

		errors.push_back(err);
        return 0;
	}
	return *matches.begin();
}

const Functions& juli::TypeInfo::getFunctions() const {
	return functions;
}

const std::vector<Type*> juli::TypeInfo::getTypes() const {
	std::vector<Type*> result;
	for (std::map<std::string, Type*>::const_iterator i = typeTable.begin(); i != typeTable.end(); ++i) {
		result.push_back(i->second);
	}
	return result;
}

const Type* juli::TypeInfo::getType(const std::string& name, const Indentable* astNode) const throw (CompilerError) {
    std::map<std::string, Type*>::const_iterator typeIterator = typeTable.find(name);
    if (typeIterator != typeTable.end()) {
        return typeTable.at(name);
    } else {
		CompilerError err(astNode);
		err.getStream() << "Unknown type '" << name << "'";
		errors.push_back(err);
        return 0;
	}
}

void juli::TypeInfo::merge(const TypeInfo& other) {
	functions.merge(other.functions);
	mergeMaps(typeTable, other.typeTable);
	mergeMaps(unresolvedTypes, other.unresolvedTypes);
}

void juli::TypeInfo::dump() const {
	std::cout << "FUNCTIONS: " << std::endl;
	functions.dump();
	std::cout << "TYPES: " << std::endl;
	for (std::map<std::string, Type*>::const_iterator i = typeTable.begin(); i != typeTable.end(); ++i) {
		std::cout << i->first << ": " << i->second << std::endl;
	}
}
