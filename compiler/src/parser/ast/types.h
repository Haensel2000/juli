/*
 * types.h
 *
 *  Created on: Sep 14, 2012
 *      Author: hannes
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <debug/print.h>
#include <parser/ast/node.h>

namespace juli {

enum Primitive {
	INT8 = 0, INT32 = 1, FLOAT64 = 2, BOOLEAN = -1, VOID = -2
};

enum TypeCategory {
	PRIMITIVE,
	ARRAY
};

class Type: public cpputils::debug::Printable {
private:
	TypeCategory category;
public:
	Type(TypeCategory category);

	virtual ~Type() {
	}

	virtual const Type* getCommonType(const Type* t) const = 0;

	virtual bool isAssignableTo(const Type* t) const = 0;

	virtual bool canCastTo(const Type* t) const = 0;

	virtual bool operator==(const Type& t) const = 0;

	virtual const Type* supportsBinaryOperator(Operator op, const Type* t) const = 0;

	TypeCategory getCategory() const;
};

class PrimitiveType: public Type {
private:
	Primitive primitive;
public:

	static const PrimitiveType VOID_TYPE;
	static const PrimitiveType BOOLEAN_TYPE;
	static const PrimitiveType INT8_TYPE;
	static const PrimitiveType INT32_TYPE;
	static const PrimitiveType FLOAT64_TYPE;

	PrimitiveType(Primitive primitive);

	virtual ~PrimitiveType();

	Primitive getPrimitive() const;

	bool isInteger() const;

	bool isSignedInteger() const;

	bool isUnsignedInteger() const;

	bool isFloatingPoint() const;

	virtual bool operator==(const Type& t) const;

	virtual void print(std::ostream& os) const;

	virtual const Type* getCommonType(const Type* t) const;

	virtual bool isAssignableTo(const Type* t) const;

	virtual bool canCastTo(const Type* t) const;

	virtual const Type* supportsBinaryOperator(Operator op, const Type* t) const;
};

class ArrayType: public Type {
private:
	const Type* elementType;
public:
	ArrayType(const Type* elementType);

	virtual ~ArrayType();

	const Type* getElementType() const;

	virtual bool operator==(const Type& t) const;

	virtual void print(std::ostream& os) const;

	virtual const Type* getCommonType(const Type* t) const;

	virtual bool isAssignableTo(const Type* t) const;

	virtual bool canCastTo(const Type* t) const;

	virtual const Type* supportsBinaryOperator(Operator op, const Type* t) const;
};

}

#endif /* TYPES_H_ */
