#include "node.h"
#include <llvm/GlobalVariable.h>
#include <cassert>

using namespace juli;

juli::Marker::Marker(unsigned int line, unsigned int column) :
		line(line), column(column) {
}

juli::Marker::~Marker() {
}

void juli::Marker::print(std::ostream& os) const {
	os << line << ":" << column;
}

juli::Indentable::Indentable() :
		filename("<unknown>"), start(0, 0), end(0, 0) {
}

juli::Indentable::~Indentable() {
}

void juli::Indentable::setSourceLocation(const std::string& filename,
		const Marker& start, const Marker& end) {
	this->filename = filename;
	this->start = start;
	this->end = end;
}

void juli::Indentable::print(std::ostream& os) const {
	print(os, 0, 0);
}

void juli::Indentable::beginLine(std::ostream& os, int indent) const {
	os << std::string(indent * 2, ' ');
}

void juli::Indentable::printLocation(std::ostream& os) const {
	os << "  (" << filename << "  " << start << " - " << end << ")"
			<< std::endl;
}

juli::Node::Node(NodeType nodeType) :
		nodeType(nodeType) {
}

juli::Node::~Node() {
}

const NodeType& juli::Node::getType() const {
	return nodeType;
}

char juli::escape(const char c) {
	switch (c) {
	case 'n':
		return '\n';
	default:
		return c;
	}
}

std::ostream& operator<<(std::ostream& os, const juli::Indentable* object) {
	assert(object != 0);
	object->print(os, 0, 0);
	return os;
}

std::ostream& operator<<(std::ostream& os, const juli::Indentable& object) {
	object.print(os, 0, 0);
	return os;
}

std::ostream& operator<<(std::ostream& os, const juli::Operator& op) {
	switch (op) {
	case juli::PLUS:
		os << "+";
		break;
	case juli::SUB:
		os << "-";
		break;
	case juli::MUL:
		os << "*";
		break;
	case juli::DIV:
		os << "/";
		break;
	case juli::MOD:
		os << "%";
		break;
	case juli::EQ:
		os << "==";
		break;
	case juli::NEQ:
		os << "!=";
		break;
	case juli::LT:
		os << "<";
		break;
	case juli::GT:
		os << ">";
		break;
	case juli::LEQ:
		os << "<=";
		break;
	case juli::GEQ:
		os << ">=";
		break;
	case juli::LOR:
		os << "or";
		break;
	case juli::LAND:
		os << "and";
		break;
	case juli::MINUS:
		os << "-";
		break;
	case juli::NOT:
		os << "not";
		break;
	case juli::TILDE:
		os << "~";
		break;
	case juli::HASH:
		os << "#";
		break;
	case juli::UNKNOWN:
		os << "?";
		break;
	default:
		os << "?";
		break;
	}
	return os;
}

const unsigned int juli::Indentable::FLAG_TREE = 1;
