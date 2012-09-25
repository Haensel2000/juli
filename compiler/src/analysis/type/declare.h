/*
 * declare.h
 *
 *  Created on: Sep 24, 2012
 *      Author: hannes
 */

#ifndef DECLARE_H_
#define DECLARE_H_

#include <parser/ast/visitor.h>

#include <map>

namespace juli {



class Declarator {
private:
	TypeInfo typeInfo;
public:

	Declarator();

	const TypeInfo& getTypeInfo() {
		return typeInfo;
	}

	void visit(const Node* n);

	void visitDoubleLiteral(const NLiteral<double>* n);

	void visitIntegerLiteral(const NLiteral<uint64_t>* n);

	void visitStringLiteral(const NStringLiteral* n);

	void visitVariableRef(const NIdentifier* n);

	void visitCast(const NCast* n);

	void visitBinaryOperator(const NBinaryOperator* n);

	void visitFunctionCall(const NFunctionCall* n);

	void visitArrayAccess(const NArrayAccess* n);

	void visitAssignment(const NAssignment* n);

	void visitBlock(const NBlock* n);

	void visitExpressionStatement(const NExpressionStatement* n);

	void visitVariableDecl(const NVariableDeclaration* n);

	void visitFunctionDecl(const NFunctionSignature * n);

	void visitFunctionDef(const NFunctionDefinition* n);

	void visitReturn(const NReturnStatement* n);

	void visitIf(const NIfStatement* n);

};

}

#endif /* DECLARE_H_ */
