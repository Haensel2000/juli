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

namespace juli {

class TypeInfo;
class Function;

//class NQualifiedName : public Indentable {
//public:
//	std::vector<std::string> path;
//	std::vector<unsigned int> access;
//
//	NQualifiedName(std::vector<std::string>& path);
//
//	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
//
//	operator std::string();
//
//	operator const std::string() const;
//};

class NIdentifier : public Indentable {
public:
	std::string name;

	NIdentifier(const std::string& name);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

	operator std::string();

	operator const std::string() const;
};

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

	NBasicType(NIdentifier* id);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

	virtual const juli::Type* resolve(const TypeInfo& types) const
			throw (CompilerError);

};

class NArrayType: public NType {
public:
	NType* elementType;
	int dimension;

	virtual ~NArrayType() {
	}

	NArrayType(NType* elementType, int dimension = 1);

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

class NAddressable : public NExpression {
public:
	bool address;

	NAddressable(NodeType nodeType);

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

class NVariableRef: public NAddressable {
public:
	std::string name;

	NVariableRef(NIdentifier* id);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NQualifiedAccess : public NAddressable {
public:
	NExpression* ref;
	NIdentifier* name;
	int index;

	NQualifiedAccess(NExpression* ref, NIdentifier* name);

	NQualifiedAccess(NExpression* ref, NVariableRef* name);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NCast : public NExpression {
public:
	NExpression* expression;
	NType* target;

	NCast(NExpression* expression, NType* target);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NFunctionCall: public NExpression {
public:
	NIdentifier* name;
	ExpressionList arguments;

	Function* function;

	NFunctionCall(NIdentifier* name, ExpressionList& arguments);

	NFunctionCall(NIdentifier* name);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NArrayAccess: public NAddressable {
public:
	NExpression* ref;
	ExpressionList indices;

	NArrayAccess(NExpression* ref, ExpressionList& indices);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NUnaryOperator : public NExpression {
public:
	NExpression* expression;
	Operator op;

	NUnaryOperator(NExpression* expression, Operator op);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NBinaryOperator: public NExpression {
public:
	NExpression* lhs;
	Operator op;
	NExpression* rhs;

	NBinaryOperator(NExpression* lhs, Operator op, NExpression* rhs);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

class NAllocateArray : public NExpression {
public:
	NBasicType* type;
	std::vector<NExpression*> sizes;

	NAllocateArray(NArrayAccess* aacc);

	NAllocateArray(NBasicType* type, std::vector<NExpression*>& sizes);

	const Type* getType(const TypeInfo& typeInfo) const;

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

class NAllocateObject : public NExpression {
public:
	NBasicType* type;

	NAllocateObject(NBasicType* type);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

class NAssignment: public NStatement {
public:
	NExpression* lhs;
	NExpression* rhs;

	NAssignment(NExpression* lhs, NExpression* rhs);

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
	NIdentifier* name;
	NType* type;
	NExpression* assignmentExpr;

	NVariableDeclaration(NType* type, NIdentifier* name,
			NExpression *assignmentExpr = 0);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

extern const unsigned int MODIFIER_C;

class NFunctionSignature: public Indentable {
public:
	const std::string name;
	const NType* type;
	VariableList arguments;
	bool varArgs;
	unsigned int modifiers;

	NFunctionSignature(const NType* type, const std::string& name,
			const VariableList arguments, bool varArgs = false, unsigned int modifiers = 0);

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

class NIfClause : public Indentable {
public:
	bool first;

	NExpression* condition;
	NBlock* body;

	NIfClause(NExpression* condition, NBlock* body, bool first = false);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

class NIfStatement: public NStatement {
public:
	std::vector<NIfClause*> clauses;

	NIfStatement(std::vector<NIfClause*> clauses);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;

};

class NWhileStatement : public NStatement {
public:
	NExpression* condition;
	NBlock* body;

	NWhileStatement(NExpression* condition, NBlock* body);

	virtual void print(std::ostream& os, int indent, unsigned int flags) const;
};

}

#endif /* AST_H_ */
