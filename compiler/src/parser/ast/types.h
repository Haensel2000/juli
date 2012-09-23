/*
 * types.h
 *
 *  Created on: Sep 14, 2012
 *      Author: hannes
 */

#ifndef TYPES_H_
#define TYPES_H_

namespace juli {

enum Primitive {
	INT8,
	INT32,
	FLOAT64,
	VOID
};

class Type {

public:
	virtual ~Type() {
	}

	//virtual llvm::Type* getLLVMType() const = 0;
};

class PrimitiveType: public Type {
private:
	Primitive primitive;
public:

	static const PrimitiveType VOID_TYPE;
	static const PrimitiveType INT8_TYPE;
	static const PrimitiveType INT32_TYPE;
	static const PrimitiveType FLOAT64_TYPE;

	PrimitiveType(Primitive primitive) :
		primitive(primitive) {
	}

	virtual ~PrimitiveType() {
	}

	Primitive getPrimitive() const {
		return primitive;
	}
};

class ArrayType: public Type {
private:
	const Type* elementType;
public:
	ArrayType(const Type* elementType) :
			elementType(elementType) {
	}

	virtual ~ArrayType() {
	}

	const Type* getElementType() const {
		return elementType;
	}
};

}

#endif /* TYPES_H_ */
