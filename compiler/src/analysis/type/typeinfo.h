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

namespace juli {

class NFunctionDefinition;

class TypeInfo {
private:
	std::map<std::string, const NFunctionDefinition*> functions;
	std::map<std::string, Type*> typeTable;
public:

	TypeInfo();

	void declareFunction(const NFunctionDefinition* f);

	const NFunctionDefinition* getFunction(const std::string& name) const
			throw (CompilerError);

	const Type* getType(const std::string& name, const Indentable* astNode) const throw (CompilerError);

};

}


#endif /* TYPEINFO_H_ */
