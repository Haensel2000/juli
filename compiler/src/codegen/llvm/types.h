/*
 * types.h
 *
 *  Created on: Sep 14, 2012
 *      Author: hannes
 */

#ifndef TYPES_H_
#define TYPES_H_

namespace juli {

#include <llvm/Type.h>

class Type {

public:
	virtual ~Type() {
	}

	virtual llvm::Type* getLLVMType() const = 0;
};

class PrimitiveType: public Type {
private:
	llvm::Type* llvmType;
public:
	PrimitiveType(llvm::Type* llvmType) :
			llvmType(llvmType) {
	}

	virtual ~PrimitiveType() {
	}

	virtual llvm::Type* getLLVMType() const {
		return llvmType;
	}
};

class ArrayType: public Type {
private:
	llvm::Type* llvmType;
	const Type& elementType;
public:
	ArrayType(const Type& elementType) :
			elementType(elementType), llvmType(llvm::PointerType::get(elementType.getLLVMType(), 0)) {
	}

	virtual ~ArrayType() {
	}

	virtual llvm::Type* getLLVMType() const {
		return llvmType;
	}

	const Type& getElementType() const {
		return elementType;
	}
};

}

#endif /* TYPES_H_ */
