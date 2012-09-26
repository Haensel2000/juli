/*
 * visitor.h
 *
 *  Created on: Sep 23, 2012
 *      Author: hannes
 */

#ifndef VISITOR_H_
#define VISITOR_H_

#include <parser/ast/node.h>
#include <parser/ast/ast.h>

#include <stdint.h>

namespace juli {

template<class Handler, class ReturnValue>
ReturnValue visitAST(Handler& handler, const Node* n) {
	switch (n->getType()) {
	case DOUBLE_LITERAL:
		return handler.visitDoubleLiteral((NLiteral<double>*) n);
	case INTEGER_LITERAL:
		return handler.visitIntegerLiteral((NLiteral<uint64_t>*) n);
	case STRING_LITERAL:
		return handler.visitStringLiteral((NStringLiteral*) n);
	case VARIABLE_REF:
		return handler.visitVariableRef((NIdentifier*) n);
	case CAST:
		return handler.visitCast((NCast*) n);
	case FUNCTION_CALL:
		return handler.visitFunctionCall((NFunctionCall*) n);
	case ARRAY_ACCESS:
		return handler.visitArrayAccess((NArrayAccess*) n);
	case UNARY_OPERATOR:
		return handler.visitUnaryOperator((NUnaryOperator*) n);
	case BINARY_OPERATOR:
		return handler.visitBinaryOperator((NBinaryOperator*) n);
	case EXPRESSION:
		return handler.visitExpressionStatement((NExpressionStatement*) n);
	case VARIABLE_DECL:
		return handler.visitVariableDecl((NVariableDeclaration*) n);
	case ASSIGNMENT:
		return handler.visitAssignment((NAssignment*) n);
	case BLOCK:
		return handler.visitBlock((NBlock*) n);
	case IF:
		return handler.visitIf((NIfStatement*) n);
	case WHILE:
		return handler.visitWhile((NWhileStatement*) n);
	case RETURN:
		return handler.visitReturn((NReturnStatement*) n);
	case FUNCTION_DEF:
		return handler.visitFunctionDef((NFunctionDefinition*) n);
	}
}

}

#endif /* VISITOR_H_ */
