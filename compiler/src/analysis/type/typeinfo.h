/*
 * typeinfo.h
 *
 *  Created on: Sep 24, 2012
 *      Author: hannes
 */

#ifndef TYPEINFO_H_
#define TYPEINFO_H_

#include <map>
#include <string>

#include <parser/ast/types.h>
#include <analysis/error.h>
#include <analysis/type/functions.h>

namespace juli {

class Functions;

class TypeInfo {
private:
	Functions functions;
	std::map<std::string, Type*> typeTable;
public:

	TypeInfo();

	void declareFunction(const NFunctionDefinition* f);

	void declareFunction(Function* f);

	Function* resolveFunction(const std::string& name, std::vector<const Type*>& argTypes, const Indentable* astNode) const
			throw (CompilerError);

	const Functions& getFunctions() const;

	const Type* getType(const std::string& name, const Indentable* astNode) const throw (CompilerError);

	void dump() const;
};

}


#endif /* TYPEINFO_H_ */
