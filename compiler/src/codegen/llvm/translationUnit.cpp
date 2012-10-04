#include "translationUnit.h"

#include <stdexcept>
#include <iostream>

using namespace juli;

juli::TranslationUnit::TranslationUnit(const std::string& name,
		const TypeInfo& types) :
		types(types) {
	module = new llvm::Module(name, llvm::getGlobalContext());
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

//const Type* juli::TranslationUnit::getVariableType(
//		const std::string& name) const throw (CompilerError) {
//	try {
//		return symbolTable.at(name);
//	} catch (std::out_of_range& e) {
//		CompilerError err;
//		err.getStream() << "Undeclared variable '" << name << "'";
//		reportError(err);
//		throw err;
//	}
//	return 0;
//}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const Type* t) const
		throw (CompilerError) {
	llvm::LLVMContext& c = getContext();

	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		switch (pt->getPrimitive()) {
		case BOOLEAN:
			return llvm::Type::getInt1Ty(c);
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
		if (*at->getElementType() == PrimitiveType::INT8_TYPE && at->getDimension() == 1) { // char array
			return llvm::PointerType::get(resolveLLVMType(at->getElementType()),
					0);
		} else {
			std::stringstream s;
			s << "type__" << at->mangle();
			llvm::Type* t = module->getTypeByName(s.str());
			if (!t) {
				if (at->getStaticSize() >= 0 && at->getDimension() == 1) {
					return llvm::PointerType::get(
							llvm::ArrayType::get(
									resolveLLVMType(at->getElementType()),
									at->getStaticSize()), 0);
				} else {
					// create type:
					std::vector<llvm::Type*> fields;
					fields.push_back(
							llvm::PointerType::get(
									resolveLLVMType(at->getElementType()), 0));
					if (at->getDimension() > 1) {
						fields.push_back(
								llvm::ArrayType::get(llvm::Type::getInt32Ty(c),
										at->getDimension()));
					} else {
						fields.push_back(llvm::Type::getInt32Ty(c));
					}
					t = llvm::StructType::create(fields, s.str());
				}
			}
			return llvm::PointerType::get(t, 0);
		}

	}
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const NType* nt) const
		throw (CompilerError) {
	return resolveLLVMType(nt->resolve(types));
}
