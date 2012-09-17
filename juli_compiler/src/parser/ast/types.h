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

}

#endif /* TYPES_H_ */
