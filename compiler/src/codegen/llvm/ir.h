/*
 * ir.h
 *
 *  Created on: Sep 18, 2012
 *      Author: hannes
 */

#ifndef IR_H_
#define IR_H_

#include <string>

#include <parser/ast/ast.h>
#include <parser/ast/translationUnit.h>

namespace juli {

class IRGenerator {
private:
	TranslationUnit translationUnit;
	llvm::IRBuilder<> builder;
	llvm::Module& module;
	llvm::LLVMContext& context;

	llvm::Value* visit(const Node* n);

	llvm::Value* visitDoubleLiteral(const NDoubleLiteral* n);

	llvm::Value* visitStringLiteral(const NStringLiteral* n);

	llvm::Value* visitVariableRef(const NIdentifier* n);

	llvm::Value* visitBinaryOperator(const NBinaryOperator* n);

	llvm::Value* visitFunctionCall(const NFunctionCall* n);

	llvm::Value* visitAssignment(const NAssignment* n);

	llvm::Value* visitBlock(const NBlock* n);

	llvm::Value* visitExpressionStatement(const NExpressionStatement* n);

	llvm::Value* visitVariableDecl(const NVariableDeclaration* n);

	llvm::FunctionType* createFunctionType(const NFunctionDeclaration* n);

	llvm::Function* createFunction(const NFunctionDeclaration* n);

	llvm::Value* visitFunctionDecl(const NFunctionDeclaration* n);

	llvm::Value* visitFunctionDef(const NFunctionDefinition* n);

	llvm::Value* visitReturn(const NReturnStatement* n);

	llvm::Value* visitIf(const NIfStatement* n);

	llvm::Type* resolveType(const NType* n);
public:

	IRGenerator(const std::string& moduleName) :
			translationUnit(moduleName), builder(translationUnit.getContext()),
			module(*translationUnit.module), context(translationUnit.getContext()){
	}

	void process(const Node* n);

	const TranslationUnit& getTranslationUnit() const {
		return translationUnit;
	}
};

}

#endif /* IR_H_ */
