#include "ast.h"
#include <parser/ast/translationUnit.h>
#include <iostream>

#include <llvm/Analysis/Verifier.h>

using namespace juli;
using namespace llvm;

llvm::Value* juli::NDoubleLiteral::generateCode(
		llvm::IRBuilder<>& builder) const {
	return llvm::ConstantFP::get(translationUnit->getContext(),
			llvm::APFloat(value));
}

llvm::Value* juli::NStringLiteral::generateCode(
		llvm::IRBuilder<>& builder) const {
	Constant* s = ConstantDataArray::getString(translationUnit->getContext(),
			StringRef(value));
	GlobalVariable* globalStr = new GlobalVariable(*translationUnit->module,
			s->getType(), true, GlobalValue::PrivateLinkage, 0, ".str");
	globalStr->setInitializer(s);

//	GlobalVariable* globalStr = translationUnit->module->getGlobalVariable(
//			".str", true);
	std::vector<Constant*> indices;
	ConstantInt* const_int64_8 = ConstantInt::get(translationUnit->getContext(),
			APInt(64, StringRef("0"), 10));
	indices.push_back(const_int64_8);
	indices.push_back(const_int64_8);
	return ConstantExpr::getGetElementPtr(globalStr, indices);
}

llvm::Value* juli::NIdentifier::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Value* v = translationUnit->getSymbolTable()[name];
	if (!v)
		std::cerr << "Unknown variable name " << name << std::endl;
	return builder.CreateLoad(v);
}

llvm::Value* juli::NBinaryOperator::generateCode(
		llvm::IRBuilder<>& builder) const {
	llvm::Value* left = lhs->generateCode(builder);
	llvm::Value* right = rhs->generateCode(builder);

	if (left == 0 || right == 0) // error handling
		return 0;

	switch (op) {
	case PLUS:
		return builder.CreateFAdd(left, right, "add_res");
	case EQ:
		return builder.CreateFCmpOEQ(left, right, "eq_res");
//  case '-': return Builder.CreateFSub(L, R, "subtmp");
//  case '*': return Builder.CreateFMul(L, R, "multmp");
//  case '<':
//    L = Builder.CreateFCmpULT(L, R, "cmptmp");
//    // Convert bool 0/1 to double 0.0 or 1.0
//    return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
//                                "booltmp");
	default:
		std::cerr << "Invalid binary operator " << op << std::endl;
		return 0;
	}
}

llvm::Value* juli::NFunctionCall::generateCode(
		llvm::IRBuilder<>& builder) const {
	// Look up the name in the global module table.
	llvm::Function* function = translationUnit->module->getFunction(id->name);
//	if (function == 0) {
//		std::cerr << "Unknown function " << id->name << std::endl;
//		return 0;
//	}
//	// If argument mismatch error.
//	if (function->arg_size() != arguments.size()) {
//		std::cerr << "Incorrect # arguments passed" << std::endl;
//		return 0;
//	}

	std::vector<llvm::Value*> argValues;
	for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
		argValues.push_back(arguments[i]->generateCode(builder));
		if (argValues.back() == 0)
			return 0;
	}

	return builder.CreateCall(function, argValues, "calltmp");
}

void juli::NAssignment::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Value* addr = translationUnit->getSymbolTable()[lhs->name];
	llvm::Value* value = rhs->generateCode(builder);
	builder.CreateStore(value, addr);
}

void juli::NBlock::generateCode(llvm::IRBuilder<>& builder) const {

//	Constant* s = ConstantDataArray::getString(translationUnit->getContext(),
//			StringRef("%f\n"));
//	GlobalVariable* gvar_array__str = new GlobalVariable(
//			*translationUnit->module, s->getType(), true,
//			GlobalValue::PrivateLinkage, 0, ".str");
//	gvar_array__str->setInitializer(s);
//
//	std::vector<llvm::Type*> paramTypes1;
//	llvm::Type* returnType = llvm::Type::getInt32Ty(
//			translationUnit->getContext());
//
//	paramTypes1.push_back(
//			llvm::Type::getInt8PtrTy(translationUnit->getContext()));
//	FunctionType* printFDeclFT = FunctionType::get(returnType, paramTypes1,
//			true);
//
//	Function* printFDecl = Function::Create(printFDeclFT,
//			Function::ExternalLinkage, "printf", translationUnit->module);
//	printFDecl->setCallingConv(CallingConv::C);
//	//printFDecl->setDoesNotCapture(1U, true);
//	//printFDecl->setDoesNotThrow(true);
//
//	std::vector<llvm::Type*> paramTypes2;
//	FunctionType* mainFunctionType = FunctionType::get(returnType, paramTypes2,
//			false);
//
//	Function* mainFunction = Function::Create(mainFunctionType,
//			Function::ExternalLinkage, "main", translationUnit->module);
//	//mainFunction->setDoesNotThrow(true);
//
//	BasicBlock* block = BasicBlock::Create(translationUnit->getContext(),
//			"entry", mainFunction);
//	builder.SetInsertPoint(block);

	//llvm::Value* v = builder.CreateFAdd(ConstantFP::get(getGlobalContext(), APFloat(4.0)), ConstantFP::get(getGlobalContext(), APFloat(5.0)), "a");
	//builder.CreateRet(v);

	for (std::vector<NStatement*>::const_iterator i = statements.begin();
			i != statements.end(); ++i) {
		(*i)->generateCode(builder);
	}

//	builder.CreateRet(
//			ConstantInt::get(translationUnit->getContext(),
//					APInt(64, StringRef("0"), 10)));
}

void juli::NExpressionStatement::generateCode(
		llvm::IRBuilder<>& builder) const {
	llvm::Value* value = expression->generateCode(builder);
}

llvm::Type* juli::NVariableDeclaration::getLLVMType() const {
	return translationUnit->resolveType(type)->getLLVMType();
}

void juli::NVariableDeclaration::generateCode(
		llvm::IRBuilder<>& builder) const {
	llvm::Value* param = builder.CreateAlloca(getLLVMType());
	if (assignmentExpr)
		builder.CreateStore(assignmentExpr->generateCode(builder), param);
	translationUnit->getSymbolTable()[id->name] = param;
}

llvm::FunctionType* juli::NFunctionDeclaration::createFunctionType() const {
	llvm::Type* returnType = translationUnit->resolveType(type)->getLLVMType();
	std::vector<llvm::Type*> argumentTypes;

	for (std::vector<NVariableDeclaration*>::const_iterator i =
			arguments.begin(); i != arguments.end(); ++i) {
		argumentTypes.push_back((*i)->getLLVMType());
	}

	return FunctionType::get(returnType, argumentTypes, varArgs);
}

llvm::Function* juli::NFunctionDeclaration::createFunction() const {
	llvm::Function* f = Function::Create(createFunctionType(),
			Function::ExternalLinkage, id->name, translationUnit->module);
	return f;
}

void juli::NFunctionDeclaration::generateCode(
		llvm::IRBuilder<>& builder) const {
	llvm::Function* f = createFunction();
}

void juli::NFunctionDefinition::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Function* f = declaration->createFunction();

	llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(
			translationUnit->getContext(), "entry", f);
	builder.SetInsertPoint(llvmBlock);

	Function::arg_iterator i = f->getArgumentList().begin();
	for (VariableList::const_iterator vi = declaration->arguments.begin();
			vi != declaration->arguments.end(); ++i, ++vi) {
		//(*vi)->generateCode(builder);

		llvm::Value* param = builder.CreateAlloca(i->getType());
		builder.CreateStore(i, param);
		translationUnit->getSymbolTable()[(*vi)->id->name] = param;
	}

	block->generateCode(builder);

//	if (f->getReturnType() == llvm::Type::getVoidTy(translationUnit->getContext())) {
//		builder.CreateRet(0);
//	}

	for (std::vector<NVariableDeclaration*>::const_iterator i =
			declaration->arguments.begin(); i != declaration->arguments.end();
			++i) {
		translationUnit->getSymbolTable().erase((*i)->id->name);
	}

	if (llvm::verifyFunction(*f, PrintMessageAction)) {
		f->dump();
	}
}

void juli::NReturnStatement::generateCode(llvm::IRBuilder<>& builder) const {
	if (expression)
		builder.CreateRet(expression->generateCode(builder));
	else
		builder.CreateRet(0);
}

void juli::NIfStatement::generateCode(llvm::IRBuilder<>& builder) const {

	llvm::Function* f = builder.GetInsertBlock()->getParent();

	std::vector<NIfClause*>::const_iterator ifc = clauses.begin();

	BasicBlock* contBlock = BasicBlock::Create(translationUnit->getContext(),
			"continue", f);

	while (ifc != clauses.end()) {
		if ((*ifc)->condition) {
			BasicBlock* thenBlock = BasicBlock::Create(
					translationUnit->getContext(), "then", f);
			BasicBlock* elseBlock = BasicBlock::Create(
					translationUnit->getContext(), "else", f);

			builder.CreateCondBr((*ifc)->condition->generateCode(builder),
					thenBlock, elseBlock);

			builder.SetInsertPoint(thenBlock);
			(*ifc)->body->generateCode(builder);
			builder.CreateBr(contBlock);

			//f->getBasicBlockList().push_back(elseBlock);
			builder.SetInsertPoint(elseBlock);
		} else {
			(*ifc)->body->generateCode(builder);
		}
		++ifc;
	}
	builder.CreateBr(contBlock);

	builder.SetInsertPoint(contBlock);

}

//Function* NFunctionDeclaration::generateCode() const {
//  // Make the function type:  double(double,double) etc.
//  std::vector<Type*> Doubles(Args.size(),
//                             Type::getDoubleTy(getGlobalContext()));
//  FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobaCreatelContext()),
//                                       Doubles, false);
//
//  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);
//
//  // If F conflicted, there was already something named 'Name'.  If it has a
//    // body, don't allow redefinition or reextern.
//    if (F->getName() != Name) {
//      // Delete the one we just made and get the existing one.
//      F->eraseFromParent();
//      F = TheModule->getFunction(Name);
//
//      // If F already has a body, reject this.
//          if (!F->empty()) {
//            ErrorF("redefinition of function");
//            return 0;
//          }
//
//          // If F took a different number of args, reject.
//          if (F->arg_size() != Args.size()) {
//            ErrorF("redefinition of function with different # args");
//            return 0;
//          }
//        }
//
//
//    // Set names for all arguments.
//    unsigned Idx = 0;
//    for (Function::arg_iterator AI = F->arg_begin(); Idx != Args.size();
//         ++AI, ++Idx) {
//      AI->setName(Args[Idx]);
//
//      // Add arguments to variable symbol table.
//      NamedValues[Args[Idx]] = AI;
//    }
//
//    if (F == 0)
//        return 0;
//
//    // Create a new basic block to start insertion into.
//      BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", TheFunction);
//      Builder.SetInsertPoint(BB);
//
//      if (Value *RetVal = Body->Codegen()) {
//    	  if (Value *RetVal = Body->Codegen()) {
//    	      // Finish off the function.
//    	      Builder.CreateRet(RetVal);
//
//    	      // Validate the generated code, checking for consistency.
//    	      verifyFunction(*TheFunction);
//
//    	      return TheFunction;
//    	    }
//
//    	  // Error reading body, remove function.
//    	  TheFunction->eraseFromParent();
//    	  return 0;
//    	}
//  }
