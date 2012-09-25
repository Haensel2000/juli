#include "typecheck.h"

#include <stdexcept>

#include <debug/print.h>

using namespace juli;

juli::SymbolTable::SymbolTable(const TypeInfo& typeInfo) :
		typeInfo(typeInfo) {
}

void juli::SymbolTable::startScope(
		const std::vector<NVariableDeclaration*> functionParams) {
	startScope();
	for (std::vector<NVariableDeclaration*>::const_iterator i =
			functionParams.begin(); i != functionParams.end(); ++i) {
		addSymbol(*i);
	}
}

void juli::SymbolTable::startScope() {
	scopes.push_back(std::map<std::string, const Type*>());
}

void juli::SymbolTable::endScope() {
	scopes.pop_back();
}

const Type* juli::SymbolTable::getSymbol(const std::string& name) const {
	for (ScopeStack::const_iterator i = --scopes.end(); i >= scopes.begin();
			--i) {
		Scope::const_iterator si = i->find(name);
		if (si != i->end())
			return si->second;
	}
	CompilerError err;
	err.getStream() << "Unknown symbol " << name;
	throw err;
}

void juli::SymbolTable::addSymbol(const std::string& name, const Type* type) {
	scopes.back()[name] = type;
}

void juli::SymbolTable::addSymbol(NVariableDeclaration* node) {
	addSymbol(node->name, node->type->resolve(typeInfo));
}

juli::TypeChecker::TypeChecker(const TypeInfo& typeInfo) :
		symbolTable(typeInfo), typeInfo(typeInfo) {
}

const Type* juli::TypeChecker::visit(Node* n) {
	_newScope = false;
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
	n->expressionType = symbolTable.getSymbol(n->name);
	return n->expressionType;
	return 0;
}

const Type* juli::TypeChecker::visitBinaryOperator(NBinaryOperator* n) {
	const Type* lhs = visit(n->lhs);
	const Type* rhs = visit(n->rhs);

	n->expressionType = lhs->getCommonType(rhs);
	if (n->expressionType == 0) {
		CompilerError err;
		err.getStream() << "Incompatible types '" << lhs << "' and '" << rhs
				<< "'";
		throw err;
	}

	return n->expressionType;
}

const Type* juli::TypeChecker::visitFunctionCall(NFunctionCall* n) {
	n->expressionType = typeInfo.getFunction(n->id)->signature->type->resolve(
			typeInfo);

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
		err.getStream()
				<< "Left hand side of array access must be of array type";
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
	if (_newScope) {
		symbolTable.startScope();
	} else {
		_newScope = true;
	}

	for (StatementList::iterator i = n->statements.begin();
			i != n->statements.end(); ++i) {
		visit(*i);
	}

	symbolTable.endScope();
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
	symbolTable.addSymbol(n);
	return 0;
}

const Type* juli::TypeChecker::visitFunctionDef(NFunctionDefinition* n) {
	symbolTable.startScope(n->signature->arguments);
	_newScope = false;
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
	for (vector<NIfClause*>::iterator i = n->clauses.begin();
			i != n->clauses.end(); ++i) {
		if ((*i)->condition)
			visit((*i)->condition);
		visit((*i)->body);
	}
	return 0;
}
