#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdio>

#include <debug/print.h>

namespace juli {

enum Operator {
	PLUS,
	MINUS,
	MUL,
	DIV,
	EQ,
	NEQ,
	LT,
	GT,
	LEQ,
	GEQ,
	LOR,
	LAND,
	UNKNOWN
};

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class TranslationUnit;

enum NodeType {
	DOUBLE_LITERAL,
	INTEGER_LITERAL,
	STRING_LITERAL,
	VARIABLE_REF,
	CAST,
	FUNCTION_CALL,
	ARRAY_ACCESS,
	BINARY_OPERATOR,
	EXPRESSION,
	VARIABLE_DECL,
	ASSIGNMENT,
	BLOCK,
	IF,
	WHILE,
	RETURN,
	FUNCTION_DEF
};

class Indentable {
public:
	virtual ~Indentable() {
	}

	void beginLine(std::ostream& os, int indent) const {
		os << std::string(indent * 2, ' ');
	}

	virtual void print(std::ostream& os, int indent, unsigned int flags) const = 0;

	static const unsigned int FLAG_TREE;

};

class Node : public Indentable {
protected:
	const NodeType nodeType;
public:
	Node(const NodeType nodeType) :
			nodeType(nodeType) {
	}

	virtual ~Node() {
	}

	const NodeType& getType() const {
		return nodeType;
	}
//
//	const llvm::Module& getModule() const {
//		return *(translationUnit->module);
//	}

};

}

std::ostream& operator<<(std::ostream& os, const juli::Indentable* object);

std::ostream& operator<<(std::ostream& os, const juli::Indentable& object);

#endif
