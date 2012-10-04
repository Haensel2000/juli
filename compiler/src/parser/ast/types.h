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

const std::string mangle(Primitive p);

enum TypeCategory {
	PRIMITIVE, ARRAY
};

class Type;

class Field: public cpputils::debug::Printable {
public:

	const std::string name;
	const unsigned int index;
	const Type* type;

	Field(const std::string& name, unsigned int index, const Type* type);

	virtual ~Field();

	virtual void print(std::ostream& os) const;

};

class Type: public cpputils::debug::Printable {
private:
	TypeCategory category;
public:
	Type(TypeCategory category);

	virtual ~Type() {
	}

	virtual bool isAssignableTo(const Type* t) const = 0;

	virtual bool canCastTo(const Type* t) const = 0;

	virtual const Field* getField(const std::string& name) const = 0;

	virtual bool operator==(const Type& t) const = 0;

	virtual const std::string mangle() const = 0;

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

	virtual bool isAssignableTo(const Type* t) const;

	virtual bool canCastTo(const Type* t) const;

	virtual const Field* getField(const std::string& name) const;

	virtual const std::string mangle() const;
};

class ArrayType: public Type {
private:
	const Type* elementType;
	int dimension;
	int staticSize;

	static const Field LENGTH;
public:

	static ArrayType* getMultiDimensionalArray(const Type* elementType, int dimension);

	ArrayType(const Type* elementType, int dimension = 1, int staticSize = -1);

	virtual ~ArrayType();

	const Type* getElementType() const;

	int getDimension() const;

	int getStaticSize() const;

	virtual bool operator==(const Type& t) const;

	virtual void print(std::ostream& os) const;

	virtual bool isAssignableTo(const Type* t) const;

	virtual bool canCastTo(const Type* t) const;

	virtual const Field* getField(const std::string& name) const;

	virtual const std::string mangle() const;
};

}

#endif /* TYPES_H_ */
