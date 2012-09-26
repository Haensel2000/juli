#include "error.h"

juli::CompilerError::CompilerError(const CompilerError& ce) :
		msgstream(ce.getMessage()), node(ce.node) {
}

void juli::CompilerError::operator=(const CompilerError& ce) {
	msgstream.str(ce.getMessage());
	node = ce.node;
}

juli::CompilerError::CompilerError(const Indentable* node) :
		node(node) {
}

const std::string juli::CompilerError::getMessage() const {
	return msgstream.str();
}

std::stringstream& juli::CompilerError::getStream() {
	return msgstream;
}

const std::string& juli::CompilerError::getFile() const {
	return node->filename;
}

unsigned int juli::CompilerError::getLine() const {
	return node->line;
}

unsigned int juli::CompilerError::getStart() const {
	return node->start;
}

unsigned int juli::CompilerError::getEnd() const {
	return node->end;
}

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce) {
	os << ce.getFile() << ":" << ce.getLine() << "," << ce.getStart() << ","
			<< ce.getEnd() << " - " << ce.getMessage() << std::endl;
	return os;
}
