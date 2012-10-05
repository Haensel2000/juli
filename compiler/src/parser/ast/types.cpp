#include "types.h"

#include <stdexcept>

using namespace juli;

const PrimitiveType juli::PrimitiveType::NULL_TYPE(NIL);
const PrimitiveType juli::PrimitiveType::VOID_TYPE(VOID);
const PrimitiveType juli::PrimitiveType::BOOLEAN_TYPE(BOOLEAN);
const PrimitiveType juli::PrimitiveType::INT8_TYPE(INT8);
const PrimitiveType juli::PrimitiveType::INT32_TYPE(INT32);
const PrimitiveType juli::PrimitiveType::FLOAT64_TYPE(FLOAT64);

const Field juli::ArrayType::LENGTH("length", 1, &PrimitiveType::INT32_TYPE);

juli::Field::Field() :
		name(""), index(-1), type(0) {
}

juli::Field::Field(const Field& copy) :
		name(copy.name), index(copy.index), type(copy.type) {
}

juli::Field::Field(const std::string& name, unsigned int index,
		const Type* type) :
		name(name), index(index), type(type) {
}

juli::Field::~Field() {
}

void juli::Field::print(std::ostream& os) const {
	os << name << "@" << index << ": " << type;
}

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

bool juli::PrimitiveType::isInteger() const {
	return isSignedInteger() || isUnsignedInteger();
}

bool juli::PrimitiveType::isSignedInteger() const {
	switch (primitive) {
	case INT8:
	case INT32:
		return true;
	case FLOAT64:
	case BOOLEAN:
	case VOID:
	case NIL:
		return false;
	}
}

bool juli::PrimitiveType::isUnsignedInteger() const {
	switch (primitive) {
	case INT8:
	case INT32:
	case FLOAT64:
	case BOOLEAN:
	case VOID:
	case NIL:
		return false;
	}
}

bool juli::PrimitiveType::isFloatingPoint() const {
	switch (primitive) {
	case FLOAT64:
		return true;
	case INT8:
	case INT32:
	case BOOLEAN:
	case VOID:
	case NIL:
		return false;
	}
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
	case NIL:
		os << "null";
	}
}

bool juli::PrimitiveType::isAssignableTo(const Type* t) const {
	if (*this == *t)
		return true;

	const PrimitiveType* pt = dynamic_cast<const PrimitiveType*>(t);
	if (pt) {
		return !(primitive < INT8 || pt->primitive < INT8)
				&& (primitive < pt->primitive);
	} else if (primitive == NIL) {
		return true;
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

const Field* juli::PrimitiveType::getField(const std::string& name) const {
	return 0;
}

//http://theory.uwinnipeg.ca/localfiles/infofiles/gcc/gxxint_15.html
const std::string juli::PrimitiveType::mangle() const {
	switch (primitive) {
	case VOID:
		return "v";
	case BOOLEAN:
		return "b";
	case INT8:
		return "c";
	case INT32:
		return "i";
	case FLOAT64:
		return "d";
	case NIL:
		return "n";
	}
}

const ReferenceType juli::ReferenceType::REFERENCE_TYPE;

juli::ReferenceType::ReferenceType() :
		Type(REFERENCE) {
}

juli::ReferenceType::~ReferenceType() {
}

bool juli::ReferenceType::isAssignableTo(const Type* t) const {
	return false;
}

bool juli::ReferenceType::canCastTo(const Type* t) const {
	return false;
}

const Field* juli::ReferenceType::getField(const std::string& name) const {
	return 0;
}

bool juli::ReferenceType::operator==(const Type& t) const {
	return t.getCategory() == REFERENCE;
//	return (t.getCategory() == ARRAY || t.getCategory() == CLASS
//			|| t.getCategory() == REFERENCE || ((const PrimitiveType&)t).getPrimitive() == NIL);
}

const std::string juli::ReferenceType::mangle() const {
	return "R";
}

void juli::ReferenceType::print(std::ostream& os) const {
	os << "ref";
}

ArrayType* juli::ArrayType::getMultiDimensionalArray(const Type* elementType,
		int dimension) {
	return new ArrayType(elementType, dimension);
}

juli::ArrayType::ArrayType(const Type* elementType, int dimension,
		int staticSize) :
		Type(ARRAY), elementType(elementType), dimension(dimension), staticSize(
				staticSize) {
}

juli::ArrayType::~ArrayType() {
}

const Type* juli::ArrayType::getElementType() const {
	return elementType;
}

int juli::ArrayType::getDimension() const {
	return dimension;
}

int juli::ArrayType::getStaticSize() const {
	return staticSize;
}

bool juli::ArrayType::operator==(const Type& t) const {
//	if (t.getCategory() == REFERENCE)
//		return true;

	const ArrayType* pt = dynamic_cast<const ArrayType*>(&t);
	if (!pt)
		return false;
	return (*elementType == *pt->elementType && dimension == pt->dimension
			&& staticSize == pt->staticSize);
}

void juli::ArrayType::print(std::ostream& os) const {
	if (dimension == 1) {
		if (staticSize >= 0)
			os << elementType << "[" << staticSize << "]";
		else
			os << elementType << "[]";
	} else {
		os << elementType << "[";
		for (int i = 0; i < dimension - 1; ++i) {
			os << ",";
		}
		os << "]";
	}
}

bool juli::ArrayType::isAssignableTo(const Type* t) const {
	return (*this == *t);
}

bool juli::ArrayType::canCastTo(const Type* t) const {
	return isAssignableTo(t);
}

const Field* juli::ArrayType::getField(const std::string& name) const {
	if (name == "length") {
		if (dimension == 1)
			return &LENGTH;
		else
			return new Field("length", 1,
					new ArrayType(&PrimitiveType::INT32_TYPE, 1, dimension));
	} else {
		return 0;
	}
}

//http://theory.uwinnipeg.ca/localfiles/infofiles/gcc/gxxint_15.html
const std::string juli::ArrayType::mangle() const {
	std::stringstream s;
	s << "A";
	if (staticSize >= 0) {
		s << "S_" << staticSize << "_";
	} else if (dimension == 1) {
		s << "P";
	} else {
		s << "M_" << dimension << "_";
	}
	s << elementType->mangle();
	return s.str();
}

juli::ClassType::ClassType(const std::string& name,
		const std::vector<Field>& fields) :
		Type(CLASS), name(name), fields() {
	addFields(fields);
}

void juli::ClassType::addFields(const std::vector<Field>& fields) {
	for (std::vector<Field>::const_iterator i = fields.begin();
			i != fields.end(); ++i) {
		this->fields[i->name] = *i;
	}
}

juli::ClassType::~ClassType() {
}

bool juli::ClassType::isAssignableTo(const Type* t) const {
	return (*this == *t) || (t->getCategory() == REFERENCE);
}

bool juli::ClassType::canCastTo(const Type* t) const {
	return isAssignableTo(t);
}

const Field* juli::ClassType::getField(const std::string& name) const {
	try {
		return &fields.at(name);
	} catch (std::out_of_range& e) {
		return 0;
	}
	return 0;
}

bool juli::ClassType::operator==(const Type& t) const {
//	if (t.getCategory() == REFERENCE)
//			return true;

	const ClassType* ct = dynamic_cast<const ClassType*>(&t);
	if (!ct)
		return false;
	return (name == ct->name);
}

const std::string juli::ClassType::mangle() const {
	std::stringstream s;
	s << "C" << name;
	return s.str();
}

void juli::ClassType::print(std::ostream& os) const {
	os << name;
}

std::vector<Field> juli::ClassType::getFields() const {
	std::vector<Field> result;
	for (std::map<std::string, Field>::const_iterator i = fields.begin();
			i != fields.end(); ++i) {
		result.push_back(i->second);
	}
	return result;
}
