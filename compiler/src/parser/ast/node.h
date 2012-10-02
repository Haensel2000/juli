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
	SUB,
	MUL,
	DIV,
	MOD,
	EQ,
	NEQ,
	LT,
	GT,
	LEQ,
	GEQ,
	LOR,
	LAND,
	NOT,
	MINUS,
	TILDE,
	HASH,
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
	QUALIFIED_ACCESS,
	CAST,
	FUNCTION_CALL,
	ARRAY_ACCESS,
	UNARY_OPERATOR,
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

class Marker : public cpputils::debug::Printable {
public:
	unsigned int line;
	unsigned int column;

	Marker(unsigned int line, unsigned int column);

	virtual ~Marker();

	virtual void print(std::ostream& os) const;
};

class Indentable {
public:
	std::string filename;
	Marker start;
	Marker end;

	Indentable();

	virtual ~Indentable();

	void beginLine(std::ostream& os, int indent) const;

	void printLocation(std::ostream& os) const;

	void setSourceLocation(const std::string& filename,
			const Marker& start, const Marker& end);

	virtual void print(std::ostream& os, int indent,
			unsigned int flags) const = 0;

	static const unsigned int FLAG_TREE;

};

class Node: public Indentable {
protected:
	const NodeType nodeType;
public:

	Node(NodeType nodeType);

	virtual ~Node();

	const NodeType& getType() const;

};

}

std::ostream& operator<<(std::ostream& os, const juli::Indentable* object);

std::ostream& operator<<(std::ostream& os, const juli::Indentable& object);

std::ostream& operator<<(std::ostream& os, const juli::Operator& op);

#endif
