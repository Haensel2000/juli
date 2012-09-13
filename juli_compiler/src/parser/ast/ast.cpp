#include "ast.h"
#include <codegen/translationUnit.h>

using namespace juli;
using namespace llvm;


void error(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
}


llvm::Value* juli::NDoubleLiteral::generateCode(llvm::IRBuilder<>& builder) const {
	return llvm::ConstantFP::get(translationUnit->getContext(), llvm::APFloat(value));
}

llvm::Value* juli::NIdentifier::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Value* v = translationUnit->namedValues[name];
	if (!v)
		error("Unknown variable name");
	return v;
}

llvm::Value* juli::NBinaryOperator::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Value* left = lhs->generateCode(builder);
	llvm::Value* right = rhs->generateCode(builder);

	if (left == 0 || right == 0) // error handling
		return 0;

	switch (op) {
	case PLUS:
		return builder.CreateFAdd(left, right, "addtmp");
//  case '-': return Builder.CreateFSub(L, R, "subtmp");
//  case '*': return Builder.CreateFMul(L, R, "multmp");
//  case '<':
//    L = Builder.CreateFCmpULT(L, R, "cmptmp");
//    // Convert bool 0/1 to double 0.0 or 1.0
//    return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
//                                "booltmp");
	default:
		error("invalid binary operator");
		return 0;
	}
}

llvm::Value* juli::NMethodCall::generateCode(llvm::IRBuilder<>& builder) const {
	// Look up the name in the global module table.
	llvm::Function* function = translationUnit->module->getFunction(id->name);
	if (function == 0)
		error("Unknown function referenced");
		return 0;

	// If argument mismatch error.
	if (function->arg_size() != arguments.size())
		error("Incorrect # arguments passed");
		return 0;

	std::vector<llvm::Value*> argValues;
	for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
		argValues.push_back(arguments[i]->generateCode(builder));
		if (argValues.back() == 0)
			return 0;
	}

	return builder.CreateCall(function, argValues, "calltmp");
}

void juli::NAssignment::generateCode(llvm::IRBuilder<>& builder) const {
	translationUnit->namedValues[lhs->name] = rhs->generateCode(builder);
	//return namedValues[lhs->name];
}

void juli::NBlock::generateCode(llvm::IRBuilder<>& builder) const {

	Constant* s = ConstantDataArray::getString(translationUnit->getContext(),
				StringRef("%f\n"));
	GlobalVariable* gvar_array__str = new GlobalVariable(
			*translationUnit->module, s->getType(), true,
			GlobalValue::PrivateLinkage, 0, ".str");
	gvar_array__str->setInitializer(s);

	std::vector<Type*> paramTypes1;
	Type* returnType = Type::getInt32Ty(translationUnit->getContext());

	paramTypes1.push_back(Type::getInt8PtrTy(translationUnit->getContext()));
	FunctionType* printFDeclFT = FunctionType::get(returnType, paramTypes1,
			true);

	Function* printFDecl = Function::Create(printFDeclFT,
			Function::ExternalLinkage, "printf", translationUnit->module);
	printFDecl->setCallingConv(CallingConv::C);
	//printFDecl->setDoesNotCapture(1U, true);
	//printFDecl->setDoesNotThrow(true);

	std::vector<Type*> paramTypes2;
	FunctionType* mainFunctionType = FunctionType::get(returnType, paramTypes2,
			false);

	Function* mainFunction = Function::Create(mainFunctionType,
			Function::ExternalLinkage, "main", translationUnit->module);
	//mainFunction->setDoesNotThrow(true);

	BasicBlock* block = BasicBlock::Create(translationUnit->getContext(), "entry",
			mainFunction);
	builder.SetInsertPoint(block);

	//llvm::Value* v = builder.CreateFAdd(ConstantFP::get(getGlobalContext(), APFloat(4.0)), ConstantFP::get(getGlobalContext(), APFloat(5.0)), "a");
	//builder.CreateRet(v);

	for (std::vector<NStatement*>::const_iterator i = statements.begin();
			i != statements.end(); ++i) {
		(*i)->generateCode(builder);
	}

	builder.CreateRet(ConstantInt::get(translationUnit->getContext(), APInt(64, StringRef("0"), 10)));
}

void juli::NExpressionStatement::generateCode(llvm::IRBuilder<>& builder) const {
	llvm::Value* value = expression->generateCode(builder);

	Function* function = translationUnit->module->getFunction("printf");

	GlobalVariable* globalStr = translationUnit->module->getGlobalVariable(".str", true);
	std::vector<Constant*> indices;
	ConstantInt* const_int64_8 = ConstantInt::get(translationUnit->getContext(), APInt(64, StringRef("0"), 10));
	indices.push_back(const_int64_8);
	indices.push_back(const_int64_8);
	Constant* s = ConstantExpr::getGetElementPtr(globalStr, indices);

	std::vector<llvm::Value*> argValues;
	argValues.push_back(s);
	argValues.push_back(value);

	builder.CreateCall(function, argValues);

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
