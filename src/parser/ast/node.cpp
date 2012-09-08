#include "node.h"
#include <cassert>

std::ostream& operator<<(std::ostream& os, const Node* object) {
	assert(object != 0);
	object->print(os, 0);
	return os;
}

std::ostream& operator<<(std::ostream& os, const Node& object) {
	object.print(os, 0);
	return os;
}
