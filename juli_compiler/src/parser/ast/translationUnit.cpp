#include "translationUnit.h"

#include <stdexcept>
#include <iostream>

using namespace juli;

juli::TranslationUnit::TranslationUnit(const std::string& name) {
	module = new llvm::Module(name, llvm::getGlobalContext());

	llvm::LLVMContext& c = getContext();

	// initialize primitive types:
	typeTable["double"] = new PrimitiveType(llvm::Type::getDoubleTy(c));
	typeTable["void"] = new PrimitiveType(llvm::Type::getVoidTy(c));
	typeTable["int"] = new PrimitiveType(llvm::Type::getInt32Ty(c));
}

juli::TranslationUnit::~TranslationUnit() {
	delete module;
	for (StatementList::iterator i = statements.begin(); i != statements.end(); ++i) {
		delete *i;
	}
}

llvm::LLVMContext& juli::TranslationUnit::getContext() {
	return module->getContext();
}

void juli::TranslationUnit::generateCode() {
	llvm::IRBuilder<> builder(module->getContext());
	generateCode(builder);
}

void juli::TranslationUnit::generateCode(llvm::IRBuilder<> builder) {
	for (StatementList::iterator i = statements.begin(); i != statements.end(); ++i) {
		(*i)->generateCode(builder);
	}
}

void juli::TranslationUnit::addStatement(NStatement* statement) {
	this->statements.push_back(statement);
}

const StatementList juli::TranslationUnit::getStatements() {
	return statements;
}

const Type* juli::TranslationUnit::resolveType(const NIdentifier* id) const throw (CompilerError) {
	try {
		return typeTable.at(id->name);
	} catch (std::out_of_range& e) {
		CompilerError err;
		err.getStream() << "Unknown type '" << id->name << "'";
		reportError(err);
		//std::cerr << err << std::endl;
		throw err;
	}
}
