/*
 * types.h
 *
 *  Created on: Sep 14, 2012
 *      Author: hannes
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <debug/print.h>

namespace juli {

enum Primitive {
	INT8 = 0, INT32 = 1, FLOAT64 = 2, VOID = -1
};

class Type: public cpputils::debug::Printable {

public:
	virtual ~Type() {
	}

	virtual const Type* getCommonType(const Type* t) const = 0;

	virtual bool operator==(const Type& t) const = 0;

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

	virtual bool operator==(const Type& t) const {
		const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(&t);
		return (pt != 0) ? primitive == pt->primitive : false;
	}

	virtual void print(std::ostream& os) const {
		switch (primitive) {
		case VOID:
			os << "void";
			break;
		case INT8:
			os << "int8";
			break;
		case INT32:
			os << "int32";
			break;
		case FLOAT64:
			os << "float64";
			break;
		}
	}

	virtual const Type* getCommonType(const Type* t) const;
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

	virtual bool operator==(const Type& t) const {
		const ArrayType* pt = dynamic_cast<const ArrayType*>(&t);
		return (*elementType == *pt->elementType);
	}

	virtual void print(std::ostream& os) const {
		os << elementType << "[]";
	}

	virtual const Type* getCommonType(const Type* t) const;
};

}

#endif /* TYPES_H_ */
