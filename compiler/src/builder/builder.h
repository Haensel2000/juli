/*
 * builder.h
 *
 *  Created on: Oct 8, 2012
 *      Author: hannes
 */

#ifndef BUILDER_H_
#define BUILDER_H_

#include <analysis/type/typeinfo.h>
#include <analysis/type/declare.h>
#include <parser/parser.h>

namespace juli {

class Declarator;

class ImportLoader {
private:
public:
	virtual ~ImportLoader() {
	}

	virtual TypeInfo* importTypes(const std::string& module) = 0;
};

class Importer {
private:
	std::map<std::string, TypeInfo*> cache;
	std::vector<ImportLoader*> loaders;
public:
	Importer();
	~Importer();

	void add(ImportLoader* loader);

	TypeInfo& getTypes(const std::string& module);
};

class SourceImportLoader : public ImportLoader {
private:
	Parser& parser;
	Importer& parent;
public:
	SourceImportLoader(Parser& parser, Importer& parent);

	virtual TypeInfo* importTypes(const std::string& module);
};

}

#endif /* BUILDER_H_ */
