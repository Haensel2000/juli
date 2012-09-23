#include "ir.h"

#include <llvm/Analysis/Verifier.h>
#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

using namespace juli;

llvm::Type* juli::IRGenerator::resolveType(const NType* n) {
	return translationUnit.resolveLLVMType(n);
}

llvm::Value* juli::IRGenerator::visitDoubleLiteral(const NDoubleLiteral* n) {
	return llvm::ConstantFP::get(context, llvm::APFloat(n->value));
}

llvm::Value* juli::IRGenerator::visitStringLiteral(const NStringLiteral* n) {
	llvm::Constant * s = llvm::ConstantDataArray::getString(context,
			llvm::StringRef(n->value));
	llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(module,
			s->getType(), true, llvm::GlobalValue::PrivateLinkage, 0, ".str");
	globalStr->setInitializer(s);

	std::vector<llvm::Constant*> indices;
	llvm::ConstantInt* const_int64_8 = llvm::ConstantInt::get(context,
			llvm::APInt(64, llvm::StringRef("0"), 10));
	indices.push_back(const_int64_8);
	indices.push_back(const_int64_8);
	return llvm::ConstantExpr::getGetElementPtr(globalStr, indices);
}

llvm::Value* juli::IRGenerator::visitVariableRef(const NIdentifier* n) {
	llvm::Value* v = translationUnit.getSymbolTable()[n->name];
	if (!v)
		std::cerr << "Unknown variable name " << n->name << std::endl;
	return builder.CreateLoad(v);
}

llvm::Value* juli::IRGenerator::visitBinaryOperator(const NBinaryOperator* n) {
	llvm::Value* left = visit(n->lhs);
	llvm::Value* right = visit(n->rhs);

	if (left == 0 || right == 0) // error handling
		return 0;

	switch (n->op) {
	case PLUS:
		return builder.CreateFAdd(left, right, "add_res");
	case EQ:
		return builder.CreateFCmpOEQ(left, right, "eq_res");
	default:
		std::cerr << "Invalid binary operator " << n->op << std::endl;
		return 0;
	}
}

llvm::Value* juli::IRGenerator::visitFunctionCall(const NFunctionCall* n) {
	llvm::Function* function = module.getFunction(n->id);

	std::vector<llvm::Value*> argValues;
	for (unsigned i = 0, e = n->arguments.size(); i != e; ++i) {
		argValues.push_back(visit(n->arguments[i]));
		if (argValues.back() == 0)
			return 0;
	}

	return builder.CreateCall(function, argValues, "calltmp");
}

llvm::Value* juli::IRGenerator::visitAssignment(const NAssignment* n) {
	llvm::Value* addr = translationUnit.getSymbolTable()[n->lhs];
	llvm::Value* value = visit(n->rhs);
	builder.CreateStore(value, addr);
	return 0;
}

llvm::Value* juli::IRGenerator::visitBlock(const NBlock* n) {
	for (std::vector<NStatement*>::const_iterator i = n->statements.begin();
			i != n->statements.end(); ++i) {
		visit(*i);
	}
	return 0;
}

llvm::Value* juli::IRGenerator::visitExpressionStatement(
		const NExpressionStatement* n) {
	visit(n->expression);
	return 0;
}

llvm::Value* juli::IRGenerator::visitVariableDecl(const NVariableDeclaration* n) {
	llvm::Value* param = builder.CreateAlloca(resolveType(n->type));
	if (n->assignmentExpr)
		builder.CreateStore(visit(n->assignmentExpr), param);
	translationUnit.getSymbolTable()[n->name] = param;
	return 0;
}

llvm::FunctionType* juli::IRGenerator::createFunctionType(
		const NFunctionDeclaration* n) {
	llvm::Type* returnType = resolveType(n->type);
	std::vector<llvm::Type*> argumentTypes;

	for (std::vector<NVariableDeclaration*>::const_iterator i =
			n->arguments.begin(); i != n->arguments.end(); ++i) {
		argumentTypes.push_back(resolveType((*i)->type));
	}

	return llvm::FunctionType::get(returnType, argumentTypes, n->varArgs);
}

llvm::Function* juli::IRGenerator::createFunction(
		const NFunctionDeclaration* n) {
	llvm::Function* f = llvm::Function::Create(createFunctionType(n),
			llvm::Function::ExternalLinkage, n->name, &module);
	return f;
}

llvm::Value* juli::IRGenerator::visitFunctionDecl(const NFunctionDeclaration* n) {
	createFunction(n);
	return 0;
}

llvm::Value* juli::IRGenerator::visitFunctionDef(const NFunctionDefinition* n) {
	llvm::Function* f = createFunction(n->declaration);

	llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(
			context, "entry", f);
	builder.SetInsertPoint(llvmBlock);

	llvm::Function::arg_iterator i = f->getArgumentList().begin();
	for (VariableList::const_iterator vi = n->declaration->arguments.begin();
			vi != n->declaration->arguments.end(); ++i, ++vi) {
		//(*vi)->generateCode(builder);

		llvm::Value* param = builder.CreateAlloca(i->getType());
		builder.CreateStore(i, param);
		translationUnit.getSymbolTable()[(*vi)->name] = param;
	}

	visit(n->body);

//	if (f->getReturnType() == llvm::Type::getVoidTy(translationUnit->getContext())) {
//		builder.CreateRet(0);
//	}

	for (std::vector<NVariableDeclaration*>::const_iterator i =
			n->declaration->arguments.begin(); i != n->declaration->arguments.end();
			++i) {
		translationUnit.getSymbolTable().erase((*i)->name);
	}

	if (llvm::verifyFunction(*f, llvm::PrintMessageAction)) {
		f->dump();
	}
	return 0;
}

llvm::Value* juli::IRGenerator::visitReturn(const NReturnStatement* n) {
	if (n->expression)
		builder.CreateRet(visit(n->expression));
	else
		builder.CreateRet(0);
	return 0;
}

llvm::Value* juli::IRGenerator::visitIf(const NIfStatement* n) {

	llvm::Function* f = builder.GetInsertBlock()->getParent();

	std::vector<NIfClause*>::const_iterator ifc = n->clauses.begin();

	llvm::BasicBlock* contBlock = llvm::BasicBlock::Create(context,
			"continue", f);

	while (ifc != n->clauses.end()) {
		if ((*ifc)->condition) {
			llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(
					context, "then", f);
			llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(
					context, "else", f);

			builder.CreateCondBr(visit((*ifc)->condition),
					thenBlock, elseBlock);

			builder.SetInsertPoint(thenBlock);
			visit((*ifc)->body);
			builder.CreateBr(contBlock);

			//f->getBasicBlockList().push_back(elseBlock);
			builder.SetInsertPoint(elseBlock);
		} else {
			visit((*ifc)->body);
		}
		++ifc;
	}
	builder.CreateBr(contBlock);

	builder.SetInsertPoint(contBlock);

	return 0;
}

llvm::Value* juli::IRGenerator::visit(const Node* n) {
	switch (n->getType()) {
	case DOUBLE_LITERAL:
		return visitDoubleLiteral((NDoubleLiteral*)n);
	case STRING_LITERAL:
		return visitStringLiteral((NStringLiteral*)n);
	case VARIABLE_REF:
		return visitVariableRef((NIdentifier*)n);
	case FUNCTION_CALL:
		return visitFunctionCall((NFunctionCall*)n);
	case BINARY_OPERATOR:
		return visitBinaryOperator((NBinaryOperator*)n);
	case EXPRESSION:
		return visitExpressionStatement((NExpressionStatement*)n);
	case VARIABLE_DECL:
		return visitVariableDecl((NVariableDeclaration*)n);
	case ASSIGNMENT:
		return visitAssignment((NAssignment*)n);
	case BLOCK:
		return visitBlock((NBlock*)n);
	case IF:
		return visitIf((NIfStatement*)n);
	case RETURN:
		return visitReturn((NReturnStatement*)n);
	case FUNCTION_DECL:
		return visitFunctionDecl((NFunctionDeclaration*)n);
	case FUNCTION_DEF:
		return visitFunctionDef((NFunctionDefinition*)n);
	}
}

void juli::IRGenerator::process(const Node* n) {
	visit(n);
}
