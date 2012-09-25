/*
 * ast.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef AST_H_
#define AST_H_

#include <sstream>

#include <parser/ast/node.h>
#include <parser/ast/types.h>
#include <analysis/error.h>

#include <analysis/type/typeinfo.h>

namespace juli {

class NType: public Indentable {
public:
	virtual ~NType() {
	}

	virtual const juli::Type* resolve(const TypeInfo& types) const
			throw (CompilerError) = 0;
};

class NBasicType: public NType {
public:
	const std::string name;

	virtual ~NBasicType() {
	}

	NBasicType(const std::string& name);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

	virtual const juli::Type* resolve(const TypeInfo& types) const
			throw (CompilerError);

};

class NArrayType: public NType {
public:
	NType* elementType;

	virtual ~NArrayType() {
	}

	NArrayType(NType* elementType);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

	virtual const juli::Type* resolve(const TypeInfo& types) const
			throw (CompilerError);
};

class NExpression: public Node {
public:
	const Type* expressionType;

	NExpression(NodeType nodeType, const Type* expressionType = 0);

	const Type* getExpressionType() const;

	void printType(std::ostream& os) const;
};

class NStatement: public Node {
public:
	NStatement(NodeType nodeType);
};

template<typename T>
class NLiteral: public NExpression {
public:
	T value;

	NLiteral(NodeType nodeType, T value, const Type* type) :
			NExpression(nodeType, type), value(value) {
	}

	virtual void print(std::ostream& os, int indent, unsigned int flags) const {
		beginLine(os, indent);

		if (flags & FLAG_TREE) {
			os << "Literal: " << value;
			printType(os);
		} else {
			os << value;
		}
	}
};

class NStringLiteral: public NLiteral<std::string> {
protected:
	std::string origValue;
public:
	NStringLiteral(std::string value);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NIdentifier: public NExpression {
public:
	std::string name;

	NIdentifier(const std::string& name);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NFunctionCall: public NExpression {
public:
	const std::string id;
	ExpressionList arguments;

	NFunctionCall(const std::string& id, ExpressionList& arguments);

	NFunctionCall(const std::string& id);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NArrayAccess: public NExpression {
public:
	NExpression* ref;
	NExpression* index;

	NArrayAccess(NExpression* ref, NExpression* index);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NBinaryOperator: public NExpression {
public:
	NExpression* lhs;
	Operator op;
	NExpression* rhs;

	NBinaryOperator(NExpression* lhs, Operator op, NExpression* rhs);

	std::string opStr() const;

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NAssignment: public NStatement {
public:
	const std::string lhs;
	NExpression* rhs;

	NAssignment(const std::string& lhs, NExpression* rhs);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NBlock: public NStatement {
public:
	StatementList statements;

	NBlock();

	void addStatement(NStatement* statement);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NExpressionStatement: public NStatement {
public:
	NExpression* expression;

	NExpressionStatement(NExpression* expression);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NVariableDeclaration: public NStatement {
public:
	const std::string name;
	NType* type;
	NExpression* assignmentExpr;

	NVariableDeclaration(NType* type, const std::string& name,
			NExpression *assignmentExpr = 0);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NFunctionSignature: public Indentable {
public:
	const std::string name;
	const NType* type;
	VariableList arguments;
	bool varArgs;

	NFunctionSignature(const NType* type, const std::string& name,
			const VariableList arguments, bool varArgs = false);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

class NFunctionDefinition: public NStatement {
public:
	NFunctionSignature * signature;
	NBlock* body;

	NFunctionDefinition(NFunctionSignature * signature, NBlock* body);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NReturnStatement: public NStatement {
public:
	NExpression* expression;

	NReturnStatement(NExpression* expression);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NIfClause {
public:

	NExpression* condition;
	NBlock* body;

	NIfClause(NExpression* condition, NBlock* body);

};

class NIfStatement: public NStatement {
public:
	std::vector<NIfClause*> clauses;

	NIfStatement(std::vector<NIfClause*> clauses);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

}

#endif /* AST_H_ */
