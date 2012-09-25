#include "types.h"

using namespace juli;

const PrimitiveType juli::PrimitiveType::VOID_TYPE(VOID);
const PrimitiveType juli::PrimitiveType::BOOLEAN_TYPE(BOOLEAN);
const PrimitiveType juli::PrimitiveType::INT8_TYPE(INT8);
const PrimitiveType juli::PrimitiveType::INT32_TYPE(INT32);
const PrimitiveType juli::PrimitiveType::FLOAT64_TYPE(FLOAT64);

juli::Type::Type(TypeCategory category) :
		category(category) {
}

TypeCategory juli::Type::getCategory() const {
	return category;
}

juli::PrimitiveType::PrimitiveType(Primitive primitive) :
		Type(PRIMITIVE), primitive(primitive) {
}

juli::PrimitiveType::~PrimitiveType() {
}

Primitive juli::PrimitiveType::getPrimitive() const {
	return primitive;
}

bool juli::PrimitiveType::operator==(const Type& t) const {
	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(&t);
	return (pt != 0) ? primitive == pt->primitive : false;
}

void juli::PrimitiveType::print(std::ostream& os) const {
	switch (primitive) {
	case VOID:
		os << "void";
		break;
	case BOOLEAN:
		os << "boolean";
		break;
	case INT8:
		os << "char";
		break;
	case INT32:
		os << "int";
		break;
	case FLOAT64:
		os << "double";
		break;
	}
}

const Type* juli::PrimitiveType::getCommonType(const Type* t) const {
	if (*this == *t)
		return this;

	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		if (primitive < INT8 || pt->primitive < INT8)
			return 0;

		if (primitive < pt->primitive)
			return this;
		else
			return pt;
	} else {
		return 0;
	}
}

bool juli::PrimitiveType::isAssignableTo(const Type* t) const {
	if (*this == *t)
		return true;

	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		return !(primitive < INT8 || pt->primitive < INT8)
				&& (primitive < pt->primitive);
	} else {
		return false;
	}
}

bool juli::PrimitiveType::canCastTo(const Type* t) const {
	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		return (primitive != VOID && pt->primitive != VOID);
	} else {
		return false;
	}
}

juli::ArrayType::ArrayType(const Type* elementType) :
		Type(ARRAY), elementType(elementType) {
}

juli::ArrayType::~ArrayType() {
}

const Type* juli::ArrayType::getElementType() const {
	return elementType;
}

bool juli::ArrayType::operator==(const Type& t) const {
	const ArrayType* pt = dynamic_cast<const ArrayType*>(&t);
	if (!pt)
		return false;
	return (*elementType == *pt->elementType);
}

void juli::ArrayType::print(std::ostream& os) const {
	os << elementType << "[]";
}

const Type* juli::ArrayType::getCommonType(const Type* t) const {
	if (*this == *t)
		return this;
	else
		return 0;
}

bool juli::ArrayType::isAssignableTo(const Type* t) const {
	return (*this == *t);
}

bool juli::ArrayType::canCastTo(const Type* t) const {
	return isAssignableTo(t);
}
