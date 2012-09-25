#include "typecheck.h"

#include <stdexcept>

#include <debug/print.h>

using namespace juli;

juli::TypeChecker::TypeChecker(const TypeInfo& typeInfo) : typeInfo(typeInfo) {
}

const Type* juli::TypeChecker::visit(Node* n) {
	return visitAST<TypeChecker, const Type*>(*this, n);
}

const Type* juli::TypeChecker::visitDoubleLiteral(NLiteral<double>* n) {
	return n->expressionType;
}

const Type* juli::TypeChecker::visitIntegerLiteral(NLiteral<uint64_t>* n) {
	return n->expressionType;
}

const Type* juli::TypeChecker::visitStringLiteral(NStringLiteral* n) {
	return n->expressionType;
}

const Type* juli::TypeChecker::visitVariableRef(NIdentifier* n) {
	try {
		n->expressionType = symbolTable.at(n->name);
		return n->expressionType;
	} catch (std::out_of_range& e) {
		CompilerError err;
		err.getStream() << "Undeclared variable '" << n->name << "'";
		throw err;
	}
	return 0;
}

const Type* juli::TypeChecker::visitBinaryOperator(NBinaryOperator* n) {
	const Type* lhs = visit(n->lhs);
	const Type* rhs = visit(n->rhs);

	n->expressionType = lhs->getCommonType(rhs);
	if (n->expressionType == 0) {
		CompilerError err;
		err.getStream() << "Incompatible types '" << lhs << "' and '" << rhs << "'";
		throw err;
	}

	return n->expressionType;
}

const Type* juli::TypeChecker::visitFunctionCall(NFunctionCall* n) {
	n->expressionType = typeInfo.getFunction(n->id)->signature->type->resolve(typeInfo);

	ExpressionList args = n->arguments;
	for (ExpressionList::iterator i = args.begin(); i != args.end(); ++i) {
		visit(*i);
	}

	return n->expressionType;
}

const Type* juli::TypeChecker::visitArrayAccess(NArrayAccess* n) {
	const ArrayType* t = dynamic_cast<const ArrayType*>(visit(n->ref));
	if (t == 0) {
		n->expressionType = 0;
		CompilerError err;
		err.getStream() << "Left hand side of array access must be of array type";
		throw err;
	}

	n->expressionType = t->getElementType();
	return n->expressionType;
}

const Type* juli::TypeChecker::visitAssignment(NAssignment* n) {
	visit(n->rhs);
	return 0;
}

const Type* juli::TypeChecker::visitBlock(NBlock* n) {
	for (StatementList::iterator i = n->statements.begin(); i != n->statements.end(); ++i) {
		visit(*i);
	}
	return 0;
}

const Type* juli::TypeChecker::visitExpressionStatement(
		NExpressionStatement* n) {
	visit(n->expression);
	return 0;
}

const Type* juli::TypeChecker::visitVariableDecl(NVariableDeclaration* n) {
	if (n->assignmentExpr)
		visit(n->assignmentExpr);
	symbolTable[n->name] = n->type->resolve(typeInfo);
	return 0;
}

const Type* juli::TypeChecker::visitFunctionDef(NFunctionDefinition* n) {
	VariableList args = n->signature->arguments;
	for (VariableList::iterator i = args.begin(); i != args.end(); ++i) {
		symbolTable[(*i)->name] = (*i)->type->resolve(typeInfo);
	}
	if (n->body)
		visit(n->body);
	return 0;
}

const Type* juli::TypeChecker::visitReturn(NReturnStatement* n) {
	if (n->expression)
		visit(n->expression);
	return 0;
}

const Type* juli::TypeChecker::visitIf(NIfStatement* n) {
	for (vector<NIfClause*>::iterator i = n->clauses.begin(); i != n->clauses.end(); ++i) {
		if ((*i)->condition)
			visit((*i)->condition);
		visit((*i)->body);
	}
	return 0;
}
