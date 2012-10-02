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
#include <codegen/llvm/translationUnit.h>

#include <utility>

namespace juli {

class IRGenerator {
private:
	const TypeInfo& typeInfo;
	TranslationUnit translationUnit;
	llvm::IRBuilder<> builder;
	llvm::Module& module;
	llvm::LLVMContext& context;

	std::map<std::string, llvm::Function*> llvmFunctionTable;
	llvm::ConstantInt* zero_ui8;
	llvm::ConstantInt* zero_ui16;
	llvm::ConstantInt* zero_ui32;
	llvm::ConstantInt* zero_i8;
	llvm::ConstantInt* zero_i16;
	llvm::ConstantInt* zero_i32;
	llvm::ConstantFP* zero_float;

	llvm::ConstantInt* one_i32;

public:



	llvm::Function* getFunction(const Function* f);

	llvm::Value* visit(const Node* n);

	llvm::Value* visitDoubleLiteral(const NLiteral<double>* n);

	llvm::Value* visitIntegerLiteral(const NLiteral<uint64_t>* n);

	llvm::Value* visitStringLiteral(const NStringLiteral* n);

	llvm::Value* visitVariableRef(const NVariableRef* n);

	llvm::Value* visitCast(const NCast* n);

	llvm::Value* visitUnaryOperator(const NUnaryOperator* n);

	llvm::Value* visitBinaryOperator(const NBinaryOperator* n);

	llvm::Value* visitFunctionCall(const NFunctionCall* n);

	llvm::Value* visitArrayAccess(const NArrayAccess* n);

	llvm::Value* visitAssignment(const NAssignment* n);

	llvm::Value* visitBlock(const NBlock* n);

	llvm::Value* visitExpressionStatement(const NExpressionStatement* n);

	llvm::Value* visitVariableDecl(const NVariableDeclaration* n);

	llvm::FunctionType* createFunctionType(const Function * n);

	llvm::Function* createFunction(const Function * n);

	llvm::Value* visitFunctionDef(const NFunctionDefinition* n);

	llvm::Value* visitReturn(const NReturnStatement* n);

	llvm::Value* visitIf(const NIfStatement* n);

	llvm::Value* visitWhile(const NWhileStatement* n);

	llvm::Type* resolveType(const NType* n);
	llvm::Type* resolveType(const Type* n);

	IRGenerator(const std::string& moduleName, const TypeInfo& typeInfo) : typeInfo(typeInfo),
			translationUnit(moduleName, typeInfo), builder(translationUnit.getContext()), module(
					*translationUnit.module), context(
					translationUnit.getContext()) {
		zero_ui8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0, bool(false)));
		zero_ui16 = llvm::ConstantInt::get(context, llvm::APInt(16, 0, bool(false)));
		zero_ui32 = llvm::ConstantInt::get(context, llvm::APInt(32, 0, bool(false)));
		zero_i8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0, true));
		zero_i16 = llvm::ConstantInt::get(context, llvm::APInt(16, 0, true));
		zero_i32 = llvm::ConstantInt::get(context, llvm::APInt(32, 0, true));

		one_i32 = llvm::ConstantInt::get(context, llvm::APInt(32, 1, true));

		zero_float = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
	}

	void process(const Node* n);

	const TranslationUnit& getTranslationUnit() const {
		return translationUnit;
	}
};

}

#endif /* IR_H_ */
