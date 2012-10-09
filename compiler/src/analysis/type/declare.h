/*
 * declare.h
 *
 *  Created on: Sep 24, 2012
 *      Author: hannes
 */

#ifndef DECLARE_H_
#define DECLARE_H_

#include <parser/ast/visitor.h>
#include <analysis/type/typeinfo.h>

#include <map>

namespace juli {

class Importer;

class Declarator {
private:
	TypeInfo* typeInfo;
	Importer& importer;

	bool importing;

	std::vector<const NFunctionDefinition*> functionDefinitions;
	std::vector<const NClassDefinition*> classDefinitions;

	void declareImplicitOperator(const std::vector<std::string> names,
			const Type* returnType, const Type* type, unsigned int arity);

	void declareImplicitOperator(const std::string& name,
			const Type* returnType, const Type* type, unsigned int arity);

	void declareImplicitOperator(const std::string& name, const Type* type,
			unsigned int arity);
public:

	Declarator(Importer& importer, bool importing = false);

	TypeInfo* declare(const Node* n);

	void visit(const Node* n);

	void visitDoubleLiteral(const NLiteral<double>* n);

	void visitIntegerLiteral(const NLiteral<uint64_t>* n);

	void visitStringLiteral(const NStringLiteral* n);

	void visitCharLiteral(const NCharLiteral* n);

	void visitBooleanLiteral(const NLiteral<bool>* n);

	void visitNullLiteral(const NLiteral<int>* n);

	void visitVariableRef(const NVariableRef* n);

	void visitQualifiedAccess(const NQualifiedAccess* n);

	void visitCast(const NCast* n);

	void visitUnaryOperator(const NUnaryOperator* n);

	void visitBinaryOperator(const NBinaryOperator* n);

	void visitAllocateArray(const NAllocateArray* n);

	void visitAllocateObject(const NAllocateObject* n);

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

	void visitWhile(const NWhileStatement* n);

	void visitClassDef(const NClassDefinition* n);

	void visitImport(const NImportStatement* n);

};

}

#endif /* DECLARE_H_ */
