#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
//#include <llvm/Value.h>
#include <debug/print.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

enum Operator {
	PLUS,
	UNKNOWN
};

class Node {
public:
    virtual ~Node() {}
    //virtual llvm::Value* codeGen(CodeGenContext& context) { }

//	template <typename T>
//    void printLine(std::ostream& os, int indent, T& value) const {
//    	beginLine(os, indent);
//		os << value << std::endl;
//    }

	void beginLine(std::ostream& os, int indent) const {
		os << std::string(indent*2, ' ');
	}

    virtual void print(std::ostream& os, int indent) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Node* object);

std::ostream& operator<<(std::ostream& os, const Node& object);

class NExpression : public Node {
};

class NStatement : public Node {
};

template <typename T>
class NLiteral : public NExpression {
protected:

public:
	T value;

	NLiteral(T value) : value(value) { }

	virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << value;
	}
};

class NDoubleLiteral : public NLiteral<double> {
public:
	NDoubleLiteral(double value) : NLiteral<double>(value) {}
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

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name)
    {}

    virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << name;
	}

    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }

    NMethodCall(const NIdentifier& id) : id(id) { }

    virtual void print(std::ostream& os, int indent) const {
    	beginLine(os, indent);
    	os << id << "(" << arguments << ");";
	}

    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
    NExpression* lhs;
    Operator op;
    NExpression* rhs;
    NBinaryOperator(NExpression* lhs, Operator op, NExpression* rhs) :
        lhs(lhs), op(op), rhs(rhs)  { }

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

		os << "(" << lhs << " " << opStr << " "<< rhs << ")";
	}

	//virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NStatement {
public:
    NIdentifier* lhs;
    NExpression* rhs;
    NAssignment(NIdentifier* lhs, NExpression* rhs) :
        lhs(lhs), rhs(rhs) { }
	//virtual llvm::Value* codeGen(CodeGenContext& context);

    virtual void print(std::ostream& os, int indent) const {
    	beginLine(os, indent);

		os << lhs << " = " << rhs;
    }
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);

    void addStatement(NStatement* statement) {
    	statements.push_back(statement);
    }

    virtual void print(std::ostream& os, int indent) const {
    	for (std::vector<NStatement*>::const_iterator i = statements.begin(); i != statements.end(); ++i) {
			beginLine(os, indent);
			os << **i << std::endl;
		}
    }
};

class NExpressionStatement : public NStatement {
public:
    NExpression* expression;
    NExpressionStatement(NExpression* expression) :
        expression(expression) { }
	//virtual llvm::Value* codeGen(CodeGenContext& context);

    virtual void print(std::ostream& os, int indent) const {
    	beginLine(os, indent);
    	os << expression;
    }
};

class NVariableDeclaration : public NStatement {
public:
    const NIdentifier& type;
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const NIdentifier& type, NIdentifier& id) :
        type(type), id(id) { }
    NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
	//virtual llvm::Value* codeGen(CodeGenContext& context);

    virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << " = " << assignmentExpr << ";";
    }
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id,
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { }
	//virtual llvm::Value* codeGen(CodeGenContext& context);

    virtual void print(std::ostream& os, int indent) const {
		beginLine(os, indent);
		os << type << " " << id << "(" << arguments << ")" << block;
    }
};

#endif
