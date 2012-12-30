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
    case Node::DOUBLE_LITERAL:
		return handler.visitDoubleLiteral((NLiteral<double>*) n);
	case Node::INTEGER_LITERAL:
		return handler.visitIntegerLiteral((NLiteral<uint64_t>*) n);
	case Node::STRING_LITERAL:
		return handler.visitStringLiteral((NStringLiteral*) n);
	case Node::CHAR_LITERAL:
			return handler.visitCharLiteral((NCharLiteral*) n);
	case Node::BOOLEAN_LITERAL:
			return handler.visitBooleanLiteral((NLiteral<bool>*) n);
	case Node::NULL_LITERAL:
			return handler.visitNullLiteral((NLiteral<int>*) n);
	case Node::VARIABLE_REF:
		return handler.visitVariableRef((NVariableRef*) n);
	case Node::QUALIFIED_ACCESS:
		return handler.visitQualifiedAccess((NQualifiedAccess*) n);
	case Node::CAST:
		return handler.visitCast((NCast*) n);
	case Node::FUNCTION_CALL:
		return handler.visitFunctionCall((NFunctionCall*) n);
	case Node::ARRAY_ACCESS:
		return handler.visitArrayAccess((NArrayAccess*) n);
	case Node::UNARY_OPERATOR:
		return handler.visitUnaryOperator((NUnaryOperator*) n);
	case Node::BINARY_OPERATOR:
		return handler.visitBinaryOperator((NBinaryOperator*) n);
	case Node::NEW_ARRAY:
		return handler.visitAllocateArray((NAllocateArray*) n);
	case Node::NEW_OBJECT:
		return handler.visitAllocateObject((NAllocateObject*) n);
	case Node::EXPRESSION:
		return handler.visitExpressionStatement((NExpressionStatement*) n);
	case Node::VARIABLE_DECL:
		return handler.visitVariableDecl((NVariableDeclaration*) n);
	case Node::ASSIGNMENT:
		return handler.visitAssignment((NAssignment*) n);
	case Node::BLOCK:
		return handler.visitBlock((NBlock*) n);
	case Node::IF:
		return handler.visitIf((NIfStatement*) n);
	case Node::WHILE:
		return handler.visitWhile((NWhileStatement*) n);
	case Node::RETURN:
		return handler.visitReturn((NReturnStatement*) n);
	case Node::FUNCTION_DEF:
		return handler.visitFunctionDef((NFunctionDefinition*) n);
	case Node::CLASS_DEF:
		return handler.visitClassDef((NClassDefinition*) n);
	case Node::IMPORT:
		return handler.visitImport((NImportStatement*) n);
	}
	return ReturnValue();
}

}

#endif /* VISITOR_H_ */
