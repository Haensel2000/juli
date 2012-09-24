#include "types.h"

using namespace juli;

const PrimitiveType juli::PrimitiveType::VOID_TYPE(VOID);
const PrimitiveType juli::PrimitiveType::INT8_TYPE(INT8);
const PrimitiveType juli::PrimitiveType::INT32_TYPE(INT32);
const PrimitiveType juli::PrimitiveType::FLOAT64_TYPE(FLOAT64);

const Type* juli::PrimitiveType::getCommonType(const Type* t) const {
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

const Type* juli::ArrayType::getCommonType(const Type* t) const {
	if (*this == *t)
		return this;
	else
		return 0;
}
