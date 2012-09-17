#include "error.h"

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce) {
	os << ce.getFile() << ":" << ce.getLine() << "," << ce.getColumn() << " - "
			<< ce.getMessage() << std::endl;
	return os;
}
