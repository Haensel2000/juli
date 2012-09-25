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
	_newScope = false;
	_currentFunction = 0;
}

NExpression* juli::TypeChecker::checkAssignment(const Type* left,
		NExpression* right, const std::string& message) const {
	if (!(*left == *right->expressionType)) {

		if (!right->expressionType->isAssignableTo(left)) {
			CompilerError err;
			err.getStream() << message << "Invalid Types: Cannot assign "
					<< right << " to " << left;
			throw err;
		}
		NCast* c = new NCast(right, 0);
		c->expressionType = left;
		return c;
	} else {
		return right;
	}
}

NExpression* juli::TypeChecker::coerce(NExpression* e, const Type* type) {
	if (!(*e->expressionType == *type)) {
		NCast* c = new NCast(e, 0);
		c->expressionType = type;
		return c;
	} else {
		return e;
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

const Type* juli::TypeChecker::visitVariableRef(NIdentifier* n) {
	n->expressionType = symbolTable.getSymbol(n->name);
	return n->expressionType;
	return 0;
}

const Type* juli::TypeChecker::visitCast(NCast* n) {
	assert(n->target);
	n->expressionType = n->target->resolve(typeInfo);
	visit(n->expression);

	if (!n->expression->expressionType->canCastTo(n->expressionType)) {
		n->expressionType = 0;
		CompilerError err;
		err.getStream() << "Invalid Explicit Cast: Cannot cast "
				<< n->expression->expressionType << " to " << n->expressionType;
	}
	return n->expressionType;
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
	} else {
		n->lhs = coerce(n->lhs, n->expressionType);
		n->rhs = coerce(n->rhs, n->expressionType);
	}

	return n->expressionType;
}

const Type* juli::TypeChecker::visitFunctionCall(NFunctionCall* n) {
	NFunctionSignature* signature = typeInfo.getFunction(n->id)->signature;

	n->expressionType = 0;

	ExpressionList& args = n->arguments;
	VariableList& formalArgs = signature->arguments;

	if (args.size() < formalArgs.size()
			|| (args.size() > formalArgs.size() && !signature->varArgs)) {
		CompilerError err;
		err.getStream() << "Invalid number of arguments for function '"
				<< signature->name << "'. Expected " << formalArgs.size()
				<< " got " << args.size();
		throw err;
	}

	ExpressionList::iterator i = args.begin();
	VariableList::iterator fi = formalArgs.begin();

	int c = 1;
	while (i != args.end()) {
		visit(*i);
		if (fi != formalArgs.end()) {
			const Type* formalType = (*fi)->type->resolve(typeInfo);
			*i = checkAssignment(formalType, *i);
			++fi;
		}

		++c;
		++i;
	}

	n->expressionType = signature->type->resolve(typeInfo);
	return n->expressionType;
}

const Type* juli::TypeChecker::visitArrayAccess(NArrayAccess* n) {
	n->expressionType = 0;
	const ArrayType* t = dynamic_cast<const ArrayType*>(visit(n->ref));
	if (t == 0) {
		CompilerError err;
		err.getStream()
				<< "Left hand side of array access must be of array type";
		throw err;
	}

	visit(n->index);

	n->index = checkAssignment(&PrimitiveType::INT32_TYPE, n->index);

	n->expressionType = t->getElementType();
	return n->expressionType;
}

const Type* juli::TypeChecker::visitAssignment(NAssignment* n) {
	visit(n->rhs);
	const Type* varType = symbolTable.getSymbol(n->lhs);
	n->rhs = checkAssignment(varType, n->rhs);
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
		n->assignmentExpr = checkAssignment(varType, n->assignmentExpr);
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
	if (n->expression) {
		visit(n->expression);

		n->expression = checkAssignment(
				_currentFunction->signature->type->resolve(typeInfo),
				n->expression);
	}

	return 0;
}

const Type* juli::TypeChecker::visitIf(NIfStatement* n) {
	for (vector<NIfClause*>::iterator i = n->clauses.begin();
			i != n->clauses.end(); ++i) {
		if ((*i)->condition) {
			visit((*i)->condition);
			(*i)->condition = checkAssignment(&PrimitiveType::BOOLEAN_TYPE,
					((*i)->condition));
		}

		visit((*i)->body);
	}
	return 0;
}
