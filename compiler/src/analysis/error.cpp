#include "error.h"

using namespace juli;

const std::string juli::Error::getMessage() const {
	return msgstream.str();
}

std::stringstream& juli::Error::getStream() {
	return msgstream;
}

juli::Error::Error(const Error& ce) :
		msgstream(ce.getMessage()) {
}

void juli::Error::operator=(const Error& ce) {
	msgstream.str(ce.getMessage());
}

juli::CompilerError::CompilerError(const Indentable* node) :
		node(node) {
}

const std::string& juli::CompilerError::getFile() const {
	return node->filename;
}

const Marker juli::CompilerError::getStart() const {
	return node->start;
}

const Marker juli::CompilerError::getEnd() const {
	return node->end;
}

std::ostream& operator<<(std::ostream& os, const juli::Error& ce) {
	os << ce.getMessage() << std::endl;
	return os;
}

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce) {
	os << ce.getFile() << "  " << ce.getStart() << "," << ce.getEnd() << " - " << ce.getMessage() << std::endl;
	return os;
}
