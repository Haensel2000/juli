/*
 * typecheck.h
 *
 *  Created on: Sep 23, 2012
 *      Author: hannes
 */

#ifndef TYPECHECK_H_
#define TYPECHECK_H_

#include <parser/ast/visitor.h>
#include <analysis/type/declare.h>
#include <analysis/type/functions.h>

#include <map>
#include <vector>
#include <stack>

namespace juli {

class SymbolTable {
private:

	typedef std::map<std::string, const Type*> Scope;
	typedef std::vector<Scope> ScopeStack;

	ScopeStack scopes;
	const TypeInfo& typeInfo;
public:

	SymbolTable(const TypeInfo& typeInfo);

	~SymbolTable() {
	}

	void startScope(const std::vector<NVariableDeclaration*> functionParams);

	void startScope();

	void endScope();

	const Type* getSymbol(const std::string& name) const;

	void addSymbol(const std::string& name, const Type* type, const Node* n);

	void addSymbol(NVariableDeclaration* node);

};

class TypeChecker {
private:
	SymbolTable symbolTable;
	const TypeInfo& typeInfo;

	bool _newScope;
	bool _addressing;
	NFunctionDefinition* _currentFunction;
public:

	TypeChecker(const TypeInfo& typeInfo);

	NExpression* checkAssignment(const Type* left, NExpression* right, const Indentable* n, const std::string& message = "") const;

	NExpression* coerce(NExpression* e, const Type* type) const;

	void coerce(ExpressionList& expressions, std::vector<FormalParameter> params) const;

	const Type* visit(Node* n);

	const Type* visitDoubleLiteral(NLiteral<double>* n);

	const Type* visitIntegerLiteral(NLiteral<uint64_t>* n);

	const Type* visitStringLiteral(NStringLiteral* n);

	const Type* visitCharLiteral(const NCharLiteral* n);

	const Type* visitBooleanLiteral(const NLiteral<bool>* n);

	const Type* visitNullLiteral(const NLiteral<int>* n);

	const Type* visitVariableRef(NVariableRef* n);

	const Type* visitQualifiedAccess(NQualifiedAccess* n);

	const Type* visitCast(NCast* n);

	const Type* visitUnaryOperator(NUnaryOperator* n);

	const Type* visitBinaryOperator(NBinaryOperator* n);

	const Type* visitAllocateArray(NAllocateArray* n);

	const Type* visitAllocateObject(NAllocateObject* n);

	const Type* visitFunctionCall(NFunctionCall* n);

	const Type* visitArrayAccess(NArrayAccess* n);

	const Type* visitAssignment(NAssignment* n);

	const Type* visitBlock(NBlock* n);

	const Type* visitExpressionStatement(NExpressionStatement* n);

	const Type* visitVariableDecl(NVariableDeclaration* n);

	const Type* visitFunctionDecl(NFunctionSignature * n);

	const Type* visitFunctionDef(NFunctionDefinition* n);

	const Type* visitReturn(NReturnStatement* n);

	const Type* visitIf(NIfStatement* n);

	const Type* visitWhile(NWhileStatement* n);

	const Type* visitClassDef(const NClassDefinition* n);

	const Type* visitImport(const NImportStatement* n);

};

}

#endif /* TYPECHECK_H_ */
