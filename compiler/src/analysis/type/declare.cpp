#include "declare.h"

#include <stdexcept>

#include <builder/builder.h>

using namespace juli;

juli::Declarator::Declarator(Importer& importer, bool implicit) : typeInfo(new TypeInfo(implicit)), importer(importer) {
}

void juli::Declarator::visit(const Node* n) {
	visitAST<Declarator, void>(*this, n);
}

TypeInfo* juli::Declarator::declare(const Node* n) {
	visit(n);
	for (std::vector<const NClassDefinition*>::iterator i =
			classDefinitions.begin(); i != classDefinitions.end(); ++i) {
		typeInfo->declareClass(*i);
	}
	for (std::vector<const NFunctionDefinition*>::iterator i =
			functionDefinitions.begin(); i != functionDefinitions.end(); ++i) {
		typeInfo->defineFunction(*i);
	}

	return typeInfo;
}

void juli::Declarator::visitDoubleLiteral(const NLiteral<double>* n) {
}

void juli::Declarator::visitIntegerLiteral(const NLiteral<uint64_t>* n) {
}

void juli::Declarator::visitStringLiteral(const NStringLiteral* n) {
}

void juli::Declarator::visitCharLiteral(const NCharLiteral* n) {
}

void juli::Declarator::visitBooleanLiteral(const NLiteral<bool>* n) {
}

void juli::Declarator::visitNullLiteral(const NLiteral<int>* n) {
}

void juli::Declarator::visitVariableRef(const NVariableRef* n) {
}

void juli::Declarator::visitQualifiedAccess(const NQualifiedAccess* n) {
}

void juli::Declarator::visitCast(const NCast* n) {
}

void juli::Declarator::visitUnaryOperator(const NUnaryOperator* n) {
}

void juli::Declarator::visitBinaryOperator(const NBinaryOperator* n) {
}

void juli::Declarator::visitAllocateArray(const NAllocateArray* n) {
}

void juli::Declarator::visitAllocateObject(const NAllocateObject* n) {
}

void juli::Declarator::visitFunctionCall(const NFunctionCall* n) {
}

void juli::Declarator::visitArrayAccess(const NArrayAccess* n) {
}

void juli::Declarator::visitAssignment(const NAssignment* n) {
}

void juli::Declarator::visitBlock(const NBlock* n) {
	StatementList st = n->statements;
	for (StatementList::const_iterator i = st.begin(); i != st.end(); ++i) {
		visit(*i);
	}
}

void juli::Declarator::visitExpressionStatement(const NExpressionStatement* n) {
}

void juli::Declarator::visitVariableDecl(const NVariableDeclaration* n) {
}

void juli::Declarator::visitFunctionDef(const NFunctionDefinition* n) {
	functionDefinitions.push_back(n);
}

void juli::Declarator::visitReturn(const NReturnStatement* n) {
}

void juli::Declarator::visitIf(const NIfStatement* n) {
}

void juli::Declarator::visitWhile(const NWhileStatement* n) {
}

void juli::Declarator::visitClassDef(const NClassDefinition* n) {
	classDefinitions.push_back(n);
}

void juli::Declarator::visitImport(const NImportStatement* n) {
	typeInfo->merge(importer.getTypes(n->name->name));
}
