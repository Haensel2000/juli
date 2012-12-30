#include "translationUnit.h"

#include <stdexcept>
#include <iostream>

#include <llvm/Support/Casting.h>

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

std::string juli::TranslationUnit::getLLVMTypeName(const Type* t) const {
	std::stringstream s;
	s << "type__" << t->mangle();
	return s.str();
}

llvm::Type* juli::TranslationUnit::getType(const Type* t) const {
	return module->getTypeByName(getLLVMTypeName(t));
}

llvm::Type* juli::TranslationUnit::createClassType(const ClassType* t) const {
	llvm::StructType* result = llvm::StructType::create(module->getContext(),
			getLLVMTypeName(t));
	std::vector<llvm::Type*> structFields;
	std::vector<Field> fields = t->getFields();
	for (std::vector<Field>::const_iterator i = fields.begin();
			i != fields.end(); ++i) {
		structFields.push_back(resolveLLVMType(i->type));
	}
	result->setBody(structFields);
	return result;
}

llvm::Type* juli::TranslationUnit::getPointerIntType() const {
	llvm::LLVMContext& c = getContext();
	switch (module->getPointerSize()) {
	case llvm::Module::Pointer64:
		return llvm::Type::getInt64Ty(c);
	case llvm::Module::Pointer32:
		return llvm::Type::getInt32Ty(c);
	default:
		return llvm::Type::getInt32Ty(c);
	}
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const Type* t) const
		throw (CompilerError) {
	llvm::LLVMContext& c = getContext();

    if (t->getKind() == Type::K_ReferenceType) {
		return getPointerIntType();
	}

    const PrimitiveType* pt = llvm::dyn_cast<const PrimitiveType>(t);
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
		case NIL:
			return getPointerIntType();
		}
	}

    const ArrayType* at = llvm::dyn_cast<const ArrayType>(t);
	if (at) {
		if (*at->getElementType() == PrimitiveType::INT8_TYPE
				&& at->getDimension() == 1) { // char array
			return llvm::PointerType::get(resolveLLVMType(at->getElementType()),
					0);
		} else {
			llvm::Type* t = getType(at);
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
					t = llvm::StructType::create(fields, getLLVMTypeName(at));
				}
			}
			return llvm::PointerType::get(t, 0);
		}

	}

    const ClassType* ct = llvm::dyn_cast<const ClassType>(t);
	if (ct) {
		llvm::Type* llvmType = getType(ct);
		if (!llvmType) {
			llvmType = createClassType(ct);
		}
		return llvm::PointerType::get(llvmType, 0);
	}

	return 0;
}

llvm::Type* juli::TranslationUnit::resolveLLVMType(const NType* nt) const
		throw (CompilerError) {
	return resolveLLVMType(nt->resolve(types));
}
