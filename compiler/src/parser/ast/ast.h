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

#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

namespace juli {

class NExpression: public Node {
public:
	NExpression(NodeType nodeType) :
			Node(nodeType) {
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const = 0;
};

class NStatement: public Node {
public:
	NStatement(NodeType nodeType) :
			Node(nodeType) {
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const = 0;
};

template<typename T>
class NLiteral: public NExpression {
protected:

public:
	T value;

	NLiteral(NodeType nodeType, T value) :
			NExpression(nodeType), value(value) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << value;
	}
};

class NStringLiteral: public NLiteral<std::string> {
protected:
	std::string origValue;
public:
	NStringLiteral(std::string value) :
			NLiteral<std::string>(STRING_LITERAL, value) {

		std::stringstream sstream;
		unsigned char escCount = 0;
		for (std::string::iterator i = value.begin(); i != value.end(); ++i) {
			if (*i == '\\') {
				escCount++;
			} else if (escCount == 1) {
				switch (*i) {
				case 'n':
					sstream << "\n";
					break;
				default:
					sstream << "\\" << *i;
					break;
				}
				escCount = 0;
			} else if (escCount > 1) {
				for (int j = 0; j < escCount; ++j)
					sstream << "\\";
				sstream << *i;
			} else {
				sstream << *i;
			}
		}

		origValue = value;
		this->value = sstream.str();

	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << "\"" << origValue << "\"";
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NIdentifier: public NExpression {
public:
	std::string name;

	NIdentifier(const std::string& name) :
			NExpression(VARIABLE_REF), name(name) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NType: public Indentable {
public:
	virtual ~NType() {
	}
};

class NBasicType: public NType {
public:
	const std::string name;

	virtual ~NBasicType() {
	}

	NBasicType(const std::string& name) :
			name(name) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

};

class NArrayType: public NType {
public:
	NType* elementType;

	virtual ~NArrayType() {
	}

	NArrayType(NType* elementType) :
			elementType(elementType) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << elementType << "[]";
	}
};

class NFunctionCall: public NExpression {
public:
	const std::string id;
	ExpressionList arguments;
	NFunctionCall(const std::string& id, ExpressionList& arguments) :
			NExpression(FUNCTION_CALL), id(id), arguments(arguments) {
	}

	NFunctionCall(const std::string& id) :
			NExpression(FUNCTION_CALL), id(id) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << id << "(" << arguments << ")";
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NDoubleLiteral: public NLiteral<double> {
public:
	NDoubleLiteral(double value) :
			NLiteral<double>(DOUBLE_LITERAL, value) {
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NBinaryOperator: public NExpression {
public:
	NExpression* lhs;
	Operator op;
	NExpression* rhs;
	NBinaryOperator(NExpression* lhs, Operator op, NExpression* rhs) :
			NExpression(BINARY_OPERATOR), lhs(lhs), op(op), rhs(rhs) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);

		std::string opStr;
		switch (op) {
		case PLUS:
			opStr = "+";
			break;
		default:
			opStr = "?";
			break;
		}

		os << "(" << lhs << " " << opStr << " " << rhs << ")";
	}

	//virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NAssignment: public NStatement {
public:
	const std::string lhs;
	NExpression* rhs;
	NAssignment(const std::string& lhs, NExpression* rhs) :
			NStatement(ASSIGNMENT), lhs(lhs), rhs(rhs) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);

		os << lhs << " = " << rhs;
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NBlock: public NStatement {
public:
	StatementList statements;
	NBlock() :
			NStatement(BLOCK) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	void addStatement(NStatement* statement) {
		statements.push_back(statement);
	}

	virtual void print(std::ostream& os, int indent) const {
		for (std::vector<NStatement*>::const_iterator i = statements.begin();
				i != statements.end(); ++i) {
			(*i)->print(os, indent + 2);
			beginLine(os, indent);
			os << ";" << std::endl;
			//os << **i << std::endl;
		}
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NExpressionStatement: public NStatement {
public:
	NExpression* expression;

	NExpressionStatement(NExpression* expression) :
			NStatement(EXPRESSION), expression(expression) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << expression;
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NVariableDeclaration: public NStatement {
public:
	const std::string name;
	NType* type;
	NExpression* assignmentExpr;

	NVariableDeclaration(NType* type, const std::string& name,
			NExpression *assignmentExpr = 0) :
			NStatement(VARIABLE_DECL), name(name), type(type), assignmentExpr(
					assignmentExpr) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << name;
		if (assignmentExpr) {
			os << " = " << assignmentExpr;
		}
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NFunctionDeclaration: public NStatement {
public:
	const std::string name;
	const NType* type;
	VariableList arguments;
	bool varArgs;

	NFunctionDeclaration(const NType* type, const std::string& name,
			const VariableList arguments, bool varArgs = false) :
			NStatement(FUNCTION_DECL), name(name), type(type), arguments(
					arguments), varArgs(varArgs) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << name << "(" << arguments;
		if (varArgs)
			os << ", ...";
		os << ")";
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;

};

class NFunctionDefinition: public NStatement {
public:
	NFunctionDeclaration* declaration;
	NBlock* body;

	NFunctionDefinition(NFunctionDeclaration* decl, NBlock* body) :
			NStatement(FUNCTION_DEF), declaration(decl), body(body) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << declaration << std::endl << body;
	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NReturnStatement: public NStatement {
public:
	NExpression* expression;

	NReturnStatement(NExpression* expression) :
			NStatement(RETURN), expression(expression) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << "return ";
		if (expression)
			os << expression;

	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NIfClause {
public:

	NExpression* condition;
	NBlock* body;

	NIfClause(NExpression* condition, NBlock* body) :
			condition(condition), body(body) {
	}

};

class NIfStatement: public NStatement {
public:
	std::vector<NIfClause*> clauses;

	NIfStatement(std::vector<NIfClause*> clauses) :
			NStatement(IF), clauses(clauses) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		std::vector<NIfClause*>::const_iterator first = clauses.begin();
		os << "if " << (*first)->condition << std::endl;
		os << (*first)->body;
		for (std::vector<NIfClause*>::const_iterator i = ++first;
				i != clauses.end(); ++i) {
			if ((*i)->condition)
				os << "else if " << (*i)->condition << std::endl;
			else
				os << "else" << std::endl;
			os << (*i)->body;
		}

	}

	//virtual void generateCode(llvm::IRBuilder<>& builder) const;

};

}

#endif /* AST_H_ */
