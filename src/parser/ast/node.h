#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdio>
#include <map>

#include <llvm/DerivedTypes.h>

#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetSelect.h"

#include <debug/print.h>

using namespace llvm;

static IRBuilder<> builder(getGlobalContext());
static std::map<std::string, Value*> namedValues;

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

enum Operator {
	PLUS, UNKNOWN
};

class TranslationUnit {
public:
	Module* module;

	TranslationUnit(const std::string& name) {
		module = new Module(name, getGlobalContext());
	}
};

class Node {
protected:
	const TranslationUnit* translationUnit;
public:
	Node(const TranslationUnit* module) :
			translationUnit(module) {
	}

	virtual ~Node() {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context) { }

//	template <typename T>
//    void printLine(std::ostream& os, int indent, T& value) const {
//    	beginLine(os, indent);
//		os << value << std::endl;
//    }

	void beginLine(std::ostream& os, int indent) const {
		os << std::string(indent * 2, ' ');
	}

	virtual void print(std::ostream& os, int indent) const = 0;

	const TranslationUnit& getTranslationUnit() const  {
		return *translationUnit;
	}


};

Node* Error(const char *Str);
Node* ErrorP(const char *Str);
Node* ErrorF(const char *Str);
Value *errorValue(const char *Str);

std::ostream& operator<<(std::ostream& os, const Node* object);

std::ostream& operator<<(std::ostream& os, const Node& object);

class NExpression: public Node {
public:
	NExpression(const TranslationUnit* module) :
			Node(module) {
	}

	virtual Value* generateCode() const = 0;
};

class NStatement: public Node {
public:
	NStatement(const TranslationUnit* module) :
			Node(module) {
	}

	virtual void generateCode() const = 0;
};

template<typename T>
class NLiteral: public NExpression {
protected:

public:
	T value;

	NLiteral(const TranslationUnit* module, T value) :
			NExpression(module), value(value) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << value;
	}
};

class NDoubleLiteral: public NLiteral<double> {
public:
	NDoubleLiteral(const TranslationUnit* module, double value) :
			NLiteral<double>(module, value) {
	}

	virtual Value* generateCode() const;
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
	NIdentifier(const TranslationUnit* module, const std::string& name) :
			NExpression(module), name(name) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

	virtual Value* generateCode() const;
};

class NMethodCall: public NExpression {
public:
	const NIdentifier* id;
	ExpressionList arguments;
	NMethodCall(const TranslationUnit* module, const NIdentifier* id,
			ExpressionList& arguments) :
			NExpression(module), id(id), arguments(arguments) {
	}

	NMethodCall(const TranslationUnit* module, const NIdentifier* id) :
			NExpression(module), id(id) {
	}

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << id << "(" << arguments << ");";
	}

	virtual Value* generateCode() const;
};

class NBinaryOperator: public NExpression {
public:
	NExpression* lhs;
	Operator op;
	NExpression* rhs;
	NBinaryOperator(const TranslationUnit* module, NExpression* lhs,
			Operator op, NExpression* rhs) :
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

	virtual Value* generateCode() const;
};

class NAssignment: public NStatement {
public:
	NIdentifier* lhs;
	NExpression* rhs;
	NAssignment(const TranslationUnit* module, NIdentifier* lhs,
			NExpression* rhs) :
			NStatement(module), lhs(lhs), rhs(rhs) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);

		os << lhs << " = " << rhs;
	}

	virtual void generateCode() const;
};

class NBlock: public NStatement {
public:
	StatementList statements;
	NBlock(const TranslationUnit* module) :
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

	virtual void generateCode() const;
};

class NExpressionStatement: public NStatement {
public:
	NExpression* expression;

	NExpressionStatement(const TranslationUnit* module, NExpression* expression) :
			NStatement(module), expression(expression) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << expression;
	}

	virtual void generateCode() const;
};

class NVariableDeclaration: public NStatement {
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	NVariableDeclaration(const TranslationUnit* module, const NIdentifier& type,
			NIdentifier& id) :
			NStatement(module), type(type), id(id) {
	}
	NVariableDeclaration(const TranslationUnit* module, const NIdentifier& type,
			NIdentifier& id, NExpression *assignmentExpr) :
			NStatement(module), type(type), id(id), assignmentExpr(
					assignmentExpr) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << " = " << assignmentExpr << ";";
	}

	virtual void generateCode() const;
};

class NFunctionDeclaration: public NStatement {
public:
	const NIdentifier* type;
	const NIdentifier* id;
	VariableList arguments;
	NBlock* block;
	NFunctionDeclaration(const TranslationUnit* module, const NIdentifier* type,
			const NIdentifier* id, const VariableList arguments, NBlock* block) :
			NStatement(module), type(type), id(id), arguments(arguments), block(
					block) {
	}
	//virtual llvm::Value* codeGen(CodeGenContext& context);

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << "(" << arguments << ")" << block;
	}

	virtual void generateCode() const;
};

#endif
