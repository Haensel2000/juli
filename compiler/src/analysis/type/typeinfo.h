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

	static std::map<std::string, Type*> implicitTypes;

	Functions functions;
	std::map<std::string, Type*> typeTable;

	std::map<std::string, const NClassDefinition*> unresolvedTypes;

	template<class K, class V>
	void mergeMaps(std::map<K, V>& first, const std::map<K, V>& second) {
		typedef typename std::map<K, V>::const_iterator ConstIt;
		typedef typename std::map<K, V>::const_iterator It;

		std::cerr << "Merging: " << std::endl;
		std::cerr << first << std::endl;
		std::cerr << second << std::endl;

		for (ConstIt i = second.begin(); i != second.end(); ++i) {
			std::pair<It, bool> res = first.insert(*i);
			if (!res.second) {
				if (implicitTypes.find(i->first) == implicitTypes.end()) {
					ImportError err;
					err.getStream() << "Multiply defined type " << i->first;
					throw err;
				}
			}
		}

	}

	void declareImplicitOperator(const std::vector<std::string> names, const Type* returnType, const Type* type,
			unsigned int arity);

	void declareImplicitOperator(const std::string& name, const Type* returnType, const Type* type, unsigned int arity);

	void declareImplicitOperator(const std::string& name, const Type* type, unsigned int arity);
public:

	TypeInfo(bool implicit = true);

	void defineFunction(const NFunctionDefinition* f);
	void defineClass(const NClassDefinition* def);

	void declareClass(const NClassDefinition* def);

	void resolveClasses();

	void declareFunction(Function* f);

	Function* resolveFunction(const std::string& name, std::vector<const Type*>& argTypes,
			const Indentable* astNode) const throw (CompilerError);

	const Functions& getFunctions() const;

	const std::vector<Type*> getTypes() const;

	const Type* getType(const std::string& name, const Indentable* astNode) const throw (CompilerError);

	void merge(const TypeInfo& other);

	void dump() const;
};

}

#endif /* TYPEINFO_H_ */
