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

std::ostream& operator<<(std::ostream& os, const juli::Operator& op) {
	switch (op) {
	case juli::PLUS:
		os << "+";
		break;
	case juli::MINUS:
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
