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

#include <map>

namespace juli {

class TypeChecker {
private:
	std::map<std::string, const Type*> symbolTable;
	const TypeInfo& typeInfo;
public:

	TypeChecker(const TypeInfo& typeInfo);

	const Type* visit(Node* n);

	const Type* visitDoubleLiteral(NLiteral<double>* n);

	const Type* visitIntegerLiteral(NLiteral<uint64_t>* n);

	const Type* visitStringLiteral(NStringLiteral* n);

	const Type* visitVariableRef(NIdentifier* n);

	const Type* visitBinaryOperator(NBinaryOperator* n);

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

};

}

#endif /* TYPECHECK_H_ */
