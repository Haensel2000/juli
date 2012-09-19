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
	NExpression(TranslationUnit* module, NodeType nodeType) :
			Node(module, nodeType) {
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const = 0;
};

class NStatement: public Node {
public:
	NStatement(TranslationUnit* module, NodeType nodeType) :
			Node(module, nodeType) {
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const = 0;
};

template<typename T>
class NLiteral: public NExpression {
protected:

public:
	T value;

	NLiteral(TranslationUnit* module, NodeType nodeType, T value) :
			NExpression(module, nodeType), value(value) {
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
	NStringLiteral(TranslationUnit* module, std::string value) :
			NLiteral<std::string>(module, STRING_LITERAL, value) {

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

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NIdentifier: public NExpression {
public:
	std::string name;

	NIdentifier(TranslationUnit* module, const std::string& name) :
			NExpression(module, VARIABLE_REF), name(name) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NType {
public:
	NType() {
	}
};

class NBasicType: public NType {
public:
	NIdentifier* name;

	NBasicType(NIdentifier* name) : name(name) {
	}
};

class NArrayType: public NType {
public:
	NBasicType* elementType;

	NArrayType(NBasicType* elementType) : elementType(elementType) {
	}
};

class NFunctionCall: public NExpression {
public:
	const NIdentifier* id;
	ExpressionList arguments;
	NFunctionCall(TranslationUnit* module, const NIdentifier* id,
			ExpressionList& arguments) :
			NExpression(module, FUNCTION_CALL), id(id), arguments(arguments) {
	}

	NFunctionCall(TranslationUnit* module, const NIdentifier* id) :
			NExpression(module, FUNCTION_CALL), id(id) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << id << "(" << arguments << ")";
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NDoubleLiteral: public NLiteral<double> {
public:
	NDoubleLiteral(TranslationUnit* module, double value) :
			NLiteral<double>(module, DOUBLE_LITERAL, value) {
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NBinaryOperator: public NExpression {
public:
	NExpression* lhs;
	Operator op;
	NExpression* rhs;
	NBinaryOperator(TranslationUnit* module, NExpression* lhs, Operator op,
			NExpression* rhs) :
			NExpression(module, BINARY_OPERATOR), lhs(lhs), op(op), rhs(rhs) {
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

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NAssignment: public NStatement {
public:
	NIdentifier* lhs;
	NExpression* rhs;
	NAssignment(TranslationUnit* module, NIdentifier* lhs, NExpression* rhs) :
			NStatement(module, ASSIGNMENT), lhs(lhs), rhs(rhs) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);

		os << lhs << " = " << rhs;
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NBlock: public NStatement {
public:
	StatementList statements;
	NBlock(TranslationUnit* module) :
			NStatement(module, BLOCK) {
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

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NExpressionStatement: public NStatement {
public:
	NExpression* expression;

	NExpressionStatement(TranslationUnit* module, NExpression* expression) :
			NStatement(module, EXPRESSION), expression(expression) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << expression;
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NVariableDeclaration: public NStatement {
public:
	NIdentifier* type;
	NIdentifier* id;
	NExpression* assignmentExpr;

	NVariableDeclaration(TranslationUnit* module, NIdentifier* type,
			NIdentifier* id, NExpression *assignmentExpr = 0) :
			NStatement(module, VARIABLE_DECL), type(type), id(id), assignmentExpr(
					assignmentExpr) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id;
		if (assignmentExpr) {
			os << " = " << assignmentExpr;
		}
	}

	llvm::Type* getLLVMType() const;

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NFunctionDeclaration: public NStatement {
protected:
	llvm::FunctionType* createFunctionType() const;
public:
	const NIdentifier* type;
	const NIdentifier* id;
	VariableList arguments;
	bool varArgs;

	NFunctionDeclaration(TranslationUnit* module, const NIdentifier* type,
			const NIdentifier* id, const VariableList arguments, bool varArgs =
					false) :
			NStatement(module, FUNCTION_DECL), type(type), id(id), arguments(arguments), varArgs(
					varArgs) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << "(" << arguments;
		if (varArgs)
			os << ", ...";
		os << ")";
	}

	llvm::Function* createFunction() const;

	virtual void generateCode(llvm::IRBuilder<>& builder) const;

};

class NFunctionDefinition: public NStatement {
public:
	NFunctionDeclaration* declaration;
	NBlock* block;

	NFunctionDefinition(TranslationUnit* module, NFunctionDeclaration* decl,
			NBlock* block) :
			NStatement(module, FUNCTION_DEF), declaration(decl), block(block) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << declaration << std::endl << block;
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NReturnStatement: public NStatement {
private:
	NExpression* expression;
public:
	NReturnStatement(TranslationUnit* module, NExpression* expression) :
			NStatement(module, RETURN), expression(expression) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << "return ";
		if (expression)
			os << expression;

	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
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
private:
	std::vector<NIfClause*> clauses;
public:

	NIfStatement(TranslationUnit* module, std::vector<NIfClause*> clauses) :
			NStatement(module, IF), clauses(clauses) {
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

	virtual void generateCode(llvm::IRBuilder<>& builder) const;

};

}

#endif /* AST_H_ */
