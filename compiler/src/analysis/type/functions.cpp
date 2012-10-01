#include "functions.h"

#include <analysis/type/typeinfo.h>

#include <stdexcept>

using namespace juli;

juli::FormalParameter::FormalParameter(const FormalParameter& copy) :
		type(copy.type), name(copy.name) {
}

juli::FormalParameter::FormalParameter(const Type* type,
		const std::string& name) :
		type(type), name(name) {
}

void juli::FormalParameter::print(std::ostream& os) const {
	os << type << " " << name;
}

juli::Function::Function(const NFunctionDefinition* functionDefinition,
		const TypeInfo& typeInfo) :
		name(functionDefinition->signature->name), resultType(
				functionDefinition->signature->type->resolve(typeInfo)), varArgs(
				functionDefinition->signature->varArgs), modifiers(
				functionDefinition->signature->modifiers), body(
				functionDefinition->body) {

	VariableList args = functionDefinition->signature->arguments;
	for (VariableList::iterator i = args.begin(); i != args.end(); ++i) {
		formalArguments.push_back(
				FormalParameter((*i)->type->resolve(typeInfo),
						(*i)->name->name));
	}

}

juli::Function::Function(const std::string& name, const Type* resultType,
		std::vector<FormalParameter>& argTypes, bool varArgs, NBlock* body) :
		name(name), resultType(resultType), formalArguments(argTypes), varArgs(
				varArgs), body(body) {

}

unsigned int juli::Function::matches(std::vector<const Type*>& argTypes) const {

	if (argTypes.size() < formalArguments.size()
			|| (argTypes.size() > formalArguments.size() && !varArgs))
		return 0;

	int s = 0;

	std::vector<const Type*>::iterator i = argTypes.begin();
	std::vector<FormalParameter>::const_iterator fi = formalArguments.begin();

	int c = 1;
	while (i != argTypes.end()) {
		if (fi != formalArguments.end()) {
			if (**i == *fi->type) {
				s += 2;
			} else if ((*i)->isAssignableTo(fi->type)) {
				s += 1;
			} else {
				s = 0;
				return s;
			}
			++fi;
		}

		++c;
		++i;
	}

	return (s >= 0) ? s : 0;
}

//http://theory.uwinnipeg.ca/localfiles/infofiles/gcc/gxxint_15.html
const std::string juli::Function::mangle() const {
	if ((modifiers & MODIFIER_C) || name == "main") {
		return name;
	} else {
		std::stringstream s;
		s << name << "__";
		for (std::vector<FormalParameter>::const_iterator i =
				formalArguments.begin(); i != formalArguments.end(); ++i) {
			s << i->type->mangle();
		}
		return s.str();
	}
}

void juli::Function::print(std::ostream& os) const {
	os << resultType << " " << name << "(" << formalArguments << ")";
}

void juli::Functions::addFunction(Function* function) {
	data[function->name].push_back(function);
}

std::vector<Function*> juli::Functions::resolve(const std::string& name,
		std::vector<const Type*>& argTypes) const {
	std::vector<Function*> matches;

	try {
		const std::vector<Function*> candidates = data.at(name);
		unsigned int bestScore = 0;
		for (std::vector<Function*>::const_iterator i = candidates.begin();
				i != candidates.end(); ++i) {
			unsigned int s = (*i)->matches(argTypes);
			if (s > bestScore) {
				bestScore = s;
				matches.clear();
				matches.push_back(*i);
			} else if (s == bestScore && bestScore > 0) {
				matches.push_back(*i);
			}
		}
		return matches;
	} catch (std::out_of_range& e) {
		return matches;
	}

	return matches;
}

void juli::Functions::dump() const {
	for (std::map<std::string, std::vector<Function*> >::const_iterator i =
			data.begin(); i != data.end(); ++i) {
		for (std::vector<Function*>::const_iterator j = i->second.begin();
				j != i->second.end(); ++j) {
			std::cout << *j << std::endl;
		}
	}
}
