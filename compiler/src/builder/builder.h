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

#include <compiler_component.h>

namespace juli {

class Declarator;

class ImportLoader : public CompilerComponent {
private:
public:
    ImportLoader(std::vector<Error>& errors) : CompilerComponent(errors) {}
    
	virtual ~ImportLoader() {
	}

	virtual TypeInfo* importTypes(const std::string& module) = 0;
};

class Importer : CompilerComponent {
private:
	std::map<std::string, TypeInfo*> cache;
	std::vector<ImportLoader*> loaders;
public:
	Importer(std::vector<Error>& errors);
	~Importer();

	void add(ImportLoader* loader);

	TypeInfo& getTypes(const std::string& module);
};

class SourceImportLoader : public ImportLoader {
private:
	Parser& parser;
	Importer& parent;
public:
	SourceImportLoader(Parser& parser, Importer& parent, std::vector<Error>& errors);

	virtual TypeInfo* importTypes(const std::string& module);
};

}

#endif /* BUILDER_H_ */
