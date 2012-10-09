/*
 * functions.h
 *
 *  Created on: Sep 27, 2012
 *      Author: hannes
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <debug/print.h>

#include <parser/ast/types.h>
#include <parser/ast/ast.h>

#include <map>
#include <vector>
#include <set>
#include <string>

namespace juli {

class TypeInfo;

class FormalParameter: public cpputils::debug::Printable {
public:
	const Type* type;
	std::string name;

	FormalParameter(const FormalParameter& copy);

	FormalParameter(const Type* type, const std::string& name);

	virtual ~FormalParameter() {
	}

	virtual void print(std::ostream& os) const;

};

class Function: public cpputils::debug::Printable {
private:
	static std::map<std::string, Function*> functionPool;

	//static Function* get(Function* fNew);

	static std::vector<FormalParameter> transformParameterList(VariableList list, const TypeInfo& typeInfo);

	//Function(const NFunctionDefinition* functionDefinition, const TypeInfo& typeInfo);

	Function(const std::string& name, const Type* resultType, std::vector<FormalParameter>& argTypes, bool varArgs,
			unsigned int modifiers, NBlock* body = 0);



public:

	static Function* get(const NFunctionDefinition* functionDefinition, const TypeInfo& typeInfo, bool importing);

	static Function* get(const std::string& name, const Type* resultType, std::vector<FormalParameter>& argTypes,
			bool varArgs, unsigned int modifiers, NBlock* body = 0);

	const std::string name;
	const Type* resultType;
	std::vector<FormalParameter> formalArguments;
	bool varArgs;

	unsigned int modifiers;
	NBlock* body;

	unsigned int matches(std::vector<const Type*>& argTypes) const;

	const std::string mangle() const;

	bool operator==(const Function& f);

	virtual void print(std::ostream& os) const;

};

const std::string mangleFunction(const std::string& name, const Type* resultType, std::vector<FormalParameter> formalArguments, bool varArgs, unsigned int modifiers);

class Functions {
private:
	std::map<std::string, std::set<Function*> > data;

public:

	void addFunction(Function* function);

	std::vector<Function*> resolve(const std::string& name, std::vector<const Type*>& argTypes) const;

	void merge(const Functions& other);

	void dump() const;

};

}

#endif /* FUNCTIONS_H_ */
