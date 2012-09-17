/*
 * ast.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef AST_H_
#define AST_H_

#include <parser/ast/node.h>

#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

namespace juli {

class NExpression: public Node {
public:
	NExpression(TranslationUnit* module) :
			Node(module) {
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const = 0;
};

class NStatement: public Node {
public:
	NStatement(TranslationUnit* module) :
			Node(module) {
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const = 0;
};

template<typename T>
class NLiteral: public NExpression {
protected:

public:
	T value;

	NLiteral(TranslationUnit* module, T value) :
			NExpression(module), value(value) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << value;
	}
};

class NDoubleLiteral: public NLiteral<double> {
public:
	NDoubleLiteral(TranslationUnit* module, double value) :
			NLiteral<double>(module, value) {
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

//class NInteger : public NExpression {
//public:
//    long long value;
//    NInteger(long long value) : value(value) { }
//    //virtual llvm::Value* codeGen(CodeGenContext& context);
//
//
//};
//
//class NDouble : public NExpression {
//public:
//    double value;
//    NDouble(double value) : value(value) { }
//    //virtual llvm::Value* codeGen(CodeGenContext& context);
//};

class NIdentifier: public NExpression {
public:
	std::string name;
	NIdentifier(TranslationUnit* module, const std::string& name) :
			NExpression(module), name(name) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

	virtual llvm::Value* generateCode(llvm::IRBuilder<>& builder) const;
};

class NMethodCall: public NExpression {
public:
	const NIdentifier* id;
	ExpressionList arguments;
	NMethodCall(TranslationUnit* module, const NIdentifier* id,
			ExpressionList& arguments) :
			NExpression(module), id(id), arguments(arguments) {
	}

	NMethodCall(TranslationUnit* module, const NIdentifier* id) :
			NExpression(module), id(id) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << id << "(" << arguments << ");";
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
			NExpression(module), lhs(lhs), op(op), rhs(rhs) {
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
			NStatement(module), lhs(lhs), rhs(rhs) {
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
			NStatement(module) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	void addStatement(NStatement* statement) {
		statements.push_back(statement);
	}

	virtual void print(std::ostream& os, int indent) const {
		for (std::vector<NStatement*>::const_iterator i = statements.begin();
				i != statements.end(); ++i) {
			beginLine(os, indent);
			os << **i << std::endl;
		}
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NExpressionStatement: public NStatement {
public:
	NExpression* expression;

	NExpressionStatement(TranslationUnit* module, NExpression* expression) :
			NStatement(module), expression(expression) {
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
	NExpression *assignmentExpr;
	NVariableDeclaration(TranslationUnit* module, NIdentifier* type,
			NIdentifier* id) :
			NStatement(module), type(type), id(id) {
	}
	NVariableDeclaration(TranslationUnit* module, NIdentifier* type,
			NIdentifier* id, NExpression *assignmentExpr) :
			NStatement(module), type(type), id(id), assignmentExpr(
					assignmentExpr) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << " = " << assignmentExpr << ";";
	}

	llvm::Type* getLLVMType() const;

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NFunctionDeclaration : public Node {
protected:
	llvm::FunctionType* createFunctionType() const;
public:
	const NIdentifier* type;
	const NIdentifier* id;
	VariableList arguments;

	NFunctionDeclaration(TranslationUnit* module, const NIdentifier* type, const NIdentifier* id,
			const VariableList arguments) : Node(module),
			type(type), id(id), arguments(arguments) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << "(" << arguments << ")";
	}

	llvm::Function* createFunction() const;

};

class NFunctionDefinition: public NStatement {
public:
	NFunctionDeclaration* declaration;
	NBlock* block;

	NFunctionDefinition(TranslationUnit* module, NFunctionDeclaration* decl,
			NBlock* block) :
			NStatement(module), declaration(decl), block(block) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << declaration << block;
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

class NReturnStatement: public NStatement {
private:
	NExpression* expression;
public:
	NReturnStatement(TranslationUnit* module, NExpression* expression) :
			NStatement(module), expression(expression) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << "return " << expression;
	}

	virtual void generateCode(llvm::IRBuilder<>& builder) const;
};

}

#endif /* AST_H_ */
