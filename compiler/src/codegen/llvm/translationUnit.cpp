#include "translationUnit.h"

#include <stdexcept>
#include <iostream>

using namespace juli;

juli::TranslationUnit::TranslationUnit(const std::string& name) {
	module = new llvm::Module(name, llvm::getGlobalContext());

	// initialize primitive types:
	typeTable["double"] = new PrimitiveType(FLOAT64);
	typeTable["void"] = new PrimitiveType(VOID);
	typeTable["int"] = new PrimitiveType(INT32);
	typeTable["char"] = new PrimitiveType(INT8);
}

juli::TranslationUnit::~TranslationUnit() {
	delete module;
	for (StatementList::iterator i = statements.begin(); i != statements.end();
			++i) {
		delete *i;
	}
}

llvm::LLVMContext& juli::TranslationUnit::getContext() const {
	return module->getContext();
}

const Type* juli::TranslationUnit::getVariableType(
		const std::string& name) const throw (CompilerError) {
	try {
		return symbolTable.at(name);
	} catch (std::out_of_range& e) {
		CompilerError err;
		err.getStream() << "Undeclared variable '" << name << "'";
		reportError(err);
		throw err;
	}
	return 0;
}

const Type* juli::TranslationUnit::getType(const std::string& name) const
		throw (CompilerError) {
	try {
		return typeTable.at(name);
	} catch (std::out_of_range& e) {
		CompilerError err;
		err.getStream() << "Unknown type '" << name << "'";
		reportError(err);
		throw err;
	}
	return 0;
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const Type* t) const
		throw (CompilerError) {
	llvm::LLVMContext& c = getContext();

	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		switch (pt->getPrimitive()) {
		case INT8:
			return llvm::Type::getInt8Ty(c);
		case INT32:
			return llvm::Type::getInt32Ty(c);
		case FLOAT64:
			return llvm::Type::getDoubleTy(c);
		case VOID:
			return llvm::Type::getVoidTy(c);
		}
	}

	const ArrayType* at = dynamic_cast<const ArrayType*>(t);
	if (at) {
		return llvm::PointerType::get(resolveLLVMType(at->getElementType()), 0);
	}
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const NType* nt) const
		throw (CompilerError) {
	return resolveLLVMType(nt->resolve(*this));
}
