#include "typecheck.h"

#include <stdexcept>

#include <debug/print.h>
#include <cassert>

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
	return 0;
}

void juli::SymbolTable::addSymbol(const std::string& name, const Type* type) {
	scopes.back()[name] = type;
}

void juli::SymbolTable::addSymbol(NVariableDeclaration* node) {
	addSymbol(node->name->name, node->type->resolve(typeInfo));
}

juli::TypeChecker::TypeChecker(const TypeInfo& typeInfo) :
		symbolTable(typeInfo), typeInfo(typeInfo) {
	_newScope = false;
	_currentFunction = 0;
}

NExpression* juli::TypeChecker::checkAssignment(const Type* left,
		NExpression* right, const Indentable* n,
		const std::string& message) const {
	if (!(*left == *right->expressionType)) {

		if (!right->expressionType->isAssignableTo(left)) {
			CompilerError err(n);
			err.getStream() << message << "Invalid Types: Cannot assign "
					<< right->expressionType << " to " << left;
			throw err;
		}
		NCast* c = new NCast(right, 0);
		c->expressionType = left;
		return c;
	} else {
		return right;
	}
}

NExpression* juli::TypeChecker::coerce(NExpression* e, const Type* type) const {
	if (!(*e->expressionType == *type)) {
		NCast* c = new NCast(e, 0);
		c->expressionType = type;
		return c;
	} else {
		return e;
	}
}

void juli::TypeChecker::coerce(ExpressionList& expressions,
		std::vector<FormalParameter> params) const {
	ExpressionList::iterator i = expressions.begin();
	std::vector<FormalParameter>::iterator fi = params.begin();

	while (fi != params.end()) {
		*i = coerce(*i, fi->type);

		++i;
		++fi;
	}
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

const Type* juli::TypeChecker::visitVariableRef(NVariableRef* n) {
	n->expressionType = symbolTable.getSymbol(n->name);
	if (!n->expressionType) {
		CompilerError err(n);
		err.getStream() << "Unknown symbol " << n->name;
		throw err;
	}
	return n->expressionType;
}

const Type* juli::TypeChecker::visitCast(NCast* n) {
	assert(n->target);
	n->expressionType = n->target->resolve(typeInfo);
	visit(n->expression);

	if (!n->expression->expressionType->canCastTo(n->expressionType)) {
		n->expressionType = 0;
		CompilerError err(n);
		err.getStream() << "Invalid Explicit Cast: Cannot cast "
				<< n->expression->expressionType << " to " << n->expressionType;
	}
	return n->expressionType;
}

const Type* juli::TypeChecker::visitUnaryOperator(NUnaryOperator* n) {
	n->expressionType = 0;

	const Type* etype = visit(n->expression);

	std::stringstream sstr;
	sstr << n->op;

	std::vector<const Type*> argTypes;
	argTypes.push_back(etype);

	Function* f = typeInfo.resolveFunction(sstr.str(), argTypes, n);
	n->expression = coerce(n->expression, f->formalArguments[0].type);

	n->expressionType = f->resultType;
	return n->expressionType;
}

const Type* juli::TypeChecker::visitBinaryOperator(NBinaryOperator* n) {
	n->expressionType = 0;

	const Type* lhs = visit(n->lhs);
	const Type* rhs = visit(n->rhs);

	std::stringstream sstr;
	sstr << n->op;

	std::vector<const Type*> argTypes;
	argTypes.push_back(lhs);
	argTypes.push_back(rhs);

	Function* f = typeInfo.resolveFunction(sstr.str(), argTypes, n);
	n->lhs = coerce(n->lhs, f->formalArguments[0].type);
	n->rhs = coerce(n->rhs, f->formalArguments[1].type);

	n->expressionType = f->resultType;
	return n->expressionType;
}

const Type* juli::TypeChecker::visitFunctionCall(NFunctionCall* n) {

	ExpressionList& args = n->arguments;
	std::vector<const Type*> argTypes;
	for (ExpressionList::iterator i = args.begin(); i != args.end(); ++i) {
		argTypes.push_back(visit(*i));
	}

	n->expressionType = 0;

	n->function = typeInfo.resolveFunction(n->name->name, argTypes, n);
	n->expressionType = n->function->resultType;

	coerce(n->arguments, n->function->formalArguments);

	return n->expressionType;
}

const Type* juli::TypeChecker::visitArrayAccess(NArrayAccess* n) {
	n->expressionType = 0;
	const ArrayType* t = dynamic_cast<const ArrayType*>(visit(n->ref));
	if (t == 0) {
		CompilerError err(n);
		err.getStream()
				<< "Left hand side of array access must be of array type";
		throw err;
	}

	visit(n->index);

	n->index = checkAssignment(&PrimitiveType::INT32_TYPE, n->index, n->index);

	n->expressionType = t->getElementType();
	return n->expressionType;
}

const Type* juli::TypeChecker::visitAssignment(NAssignment* n) {
	visit(n->rhs);
	const Type* varType = symbolTable.getSymbol(n->lhs->name);
	if (!varType) {
		CompilerError err(n);
		err.getStream() << "Unknown symbol " << n->lhs;
		throw err;
	}
	n->rhs = checkAssignment(varType, n->rhs, n);
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
	if (n->assignmentExpr) {
		visit(n->assignmentExpr);
		const Type* varType = n->type->resolve(typeInfo);
		n->assignmentExpr = checkAssignment(varType, n->assignmentExpr, n);
	}

	symbolTable.addSymbol(n);
	return 0;
}

const Type* juli::TypeChecker::visitFunctionDef(NFunctionDefinition* n) {
	_currentFunction = n;
	symbolTable.startScope(n->signature->arguments);
	_newScope = false;
	if (n->body)
		visit(n->body);
	_currentFunction = 0;
	return 0;
}

const Type* juli::TypeChecker::visitReturn(NReturnStatement* n) {
	const Type* functionReturnType = _currentFunction->signature->type->resolve(
			typeInfo);
	if (n->expression) {
		visit(n->expression);

		n->expression = checkAssignment(functionReturnType, n->expression, n);
	} else {
		if (!(*functionReturnType == PrimitiveType::VOID_TYPE)) {
			CompilerError err(n);
			err.getStream() << "Function need to return a value of type "
					<< functionReturnType;
			throw err;
		}
	}

	return 0;
}

const Type* juli::TypeChecker::visitIf(NIfStatement* n) {
	for (vector<NIfClause*>::iterator i = n->clauses.begin();
			i != n->clauses.end(); ++i) {
		if ((*i)->condition) {
			visit((*i)->condition);
			(*i)->condition = checkAssignment(&PrimitiveType::BOOLEAN_TYPE,
					(*i)->condition, (*i)->condition);
		}

		visit((*i)->body);
	}
	return 0;
}

const Type* juli::TypeChecker::visitWhile(NWhileStatement* n) {
	visit(n->condition);
	n->condition = checkAssignment(&PrimitiveType::BOOLEAN_TYPE, n->condition,
			n->condition);
	visit(n->body);
	return 0;
}
