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
	typeTable["cstr"] = new PrimitiveType(llvm::Type::getInt8PtrTy(c));
}

juli::TranslationUnit::~TranslationUnit() {
	delete module;
	for (StatementList::iterator i = statements.begin(); i != statements.end();
			++i) {
		delete *i;
	}
}

llvm::LLVMContext& juli::TranslationUnit::getContext() {
	return module->getContext();
}

const Type* juli::TranslationUnit::resolveBasicType(const NBasicType* t) const
		throw (CompilerError) {
	try {
		return typeTable.at(t->name);
	} catch (std::out_of_range& e) {
		CompilerError err;
		err.getStream() << "Unknown type '" << t->name << "'";
		reportError(err);
		throw err;
	}
	return 0;
}

const Type* juli::TranslationUnit::resolveArrayType(const NArrayType* t) const
		throw (CompilerError) {
	return new ArrayType(*resolveType(t->elementType));
}

const Type* juli::TranslationUnit::resolveType(const NType* t) const
		throw (CompilerError) {
	const NBasicType* bt = dynamic_cast<const NBasicType*>(t);
	if (bt)
		return resolveBasicType(bt);
	const NArrayType* at = dynamic_cast<const NArrayType*>(t);
	if (at)
		return resolveArrayType(at);

	return 0;
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const NType* t) const
		throw (CompilerError) {
	return resolveType(t)->getLLVMType();
}
