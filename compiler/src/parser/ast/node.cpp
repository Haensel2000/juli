#include "node.h"
#include <llvm/GlobalVariable.h>
#include <cassert>

std::ostream& operator<<(std::ostream& os, const juli::Indentable* object) {
	assert(object != 0);
	object->print(os, 0, 0);
	return os;
}

std::ostream& operator<<(std::ostream& os, const juli::Indentable& object) {
	object.print(os, 0, 0);
	return os;
}

const unsigned int juli::Indentable::FLAG_TREE = 1;
