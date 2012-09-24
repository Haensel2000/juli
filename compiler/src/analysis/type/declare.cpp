#include "declare.h"

#include <stdexcept>

using namespace juli;

juli::Declarator::Declarator() {
}

void juli::Declarator::visit(const Node* n) {
	visitAST<Declarator, void>(*this, n);
}

void juli::Declarator::visitDoubleLiteral(const NLiteral<double>* n) {
}

void juli::Declarator::visitIntegerLiteral(const NLiteral<uint64_t>* n) {
}

void juli::Declarator::visitStringLiteral(const NStringLiteral* n) {
}

void juli::Declarator::visitVariableRef(const NIdentifier* n) {
}

void juli::Declarator::visitBinaryOperator(const NBinaryOperator* n) {
}

void juli::Declarator::visitFunctionCall(const NFunctionCall* n) {
}

void juli::Declarator::visitArrayAccess(const NArrayAccess* n) {
}

void juli::Declarator::visitAssignment(const NAssignment* n) {
}

void juli::Declarator::visitBlock(const NBlock* n) {
}

void juli::Declarator::visitExpressionStatement(const NExpressionStatement* n) {
}

void juli::Declarator::visitVariableDecl(const NVariableDeclaration* n) {
}

void juli::Declarator::visitFunctionDef(const NFunctionDefinition* n) {
	typeInfo.declareFunction(n);
}

void juli::Declarator::visitReturn(const NReturnStatement* n) {
}

void juli::Declarator::visitIf(const NIfStatement* n) {
}
