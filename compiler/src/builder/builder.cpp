#include "builder.h"

using namespace juli;

juli::SourceImportLoader::SourceImportLoader(Parser& parser, Importer& parent) :
		parser(parser), parent(parent) {
}

TypeInfo* juli::SourceImportLoader::importTypes(const std::string& module) {
	Declarator declarator(parent, false);
	try {
		return declarator.declare(parser.parse(module + ".jl"));
	} catch (CompilerError& e) {
		throw e;
	} catch (...) {
	}
	return 0;
}

juli::Importer::Importer() {
}

void juli::Importer::add(ImportLoader* loader) {
	loaders.push_back(loader);
}

juli::Importer::~Importer() {
	for (std::vector<ImportLoader*>::iterator i = loaders.begin(); i != loaders.end(); ++i) {
		delete *i;
	}

	for (std::map<std::string, TypeInfo*>::iterator i = cache.begin(); i != cache.end(); ++i) {
		delete i->second;
	}
}

TypeInfo& juli::Importer::getTypes(const std::string& module) {
	TypeInfo* & ti = cache[module];
	if (!ti) {
		std::vector<ImportLoader*>::iterator loaderIt = loaders.begin();
		while (loaderIt != loaders.end() && !(ti = (*loaderIt)->importTypes(module))) {
			++loaderIt;
		}
		if (!ti) {
			ImportError err;
			err.getStream() << "Could not load module " << module;
			throw err;
		}
	}
	std::cerr << "Importing " << module;
	ti->dump();
	return *ti;
}
