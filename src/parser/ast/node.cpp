#include "node.h"
#include <llvm/GlobalVariable.h>
#include <cassert>

std::ostream& operator<<(std::ostream& os, const Node* object) {
	assert(object != 0);
	object->print(os, 0);
	return os;
}

std::ostream& operator<<(std::ostream& os, const Node& object) {
	object.print(os, 0);
	return os;
}

Value* errorValue(const char *Str) {
	Error(Str);
	return 0;
}

Node* Error(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return 0;
}

Node* ErrorP(const char *Str) {
	Error(Str);
	return 0;
}

Node* ErrorF(const char *Str) {
	Error(Str);
	return 0;
}

Value* NDoubleLiteral::generateCode() const {
	return ConstantFP::get(getGlobalContext(), APFloat(value));
}

Value* NIdentifier::generateCode() const {
	Value *V = namedValues[name];
	return V ? V : errorValue("Unknown variable name");
}

Value* NBinaryOperator::generateCode() const {
	Value* left = lhs->generateCode();
	Value* right = rhs->generateCode();

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
		return errorValue("invalid binary operator");
	}
}

Value* NMethodCall::generateCode() const {
	// Look up the name in the global module table.
	Function* function = translationUnit->module->getFunction(id->name);
	if (function == 0)
		return errorValue("Unknown function referenced");

	// If argument mismatch error.
	if (function->arg_size() != arguments.size())
		return errorValue("Incorrect # arguments passed");

	std::vector<Value*> argValues;
	for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
		argValues.push_back(arguments[i]->generateCode());
		if (argValues.back() == 0)
			return 0;
	}

	return builder.CreateCall(function, argValues, "calltmp");
}

void NAssignment::generateCode() const {
	namedValues[lhs->name] = rhs->generateCode();
	//return namedValues[lhs->name];
}

void NBlock::generateCode() const {

	Constant* s = ConstantDataArray::getString(getGlobalContext(),
				StringRef("%f"));
	GlobalVariable* gvar_array__str = new GlobalVariable(
			*translationUnit->module, s->getType(), true,
			GlobalValue::PrivateLinkage, 0, ".str");
	gvar_array__str->setInitializer(s);

	std::vector<Type*> paramTypes1;
	Type* returnType = Type::getInt32Ty(getGlobalContext());

	paramTypes1.push_back(Type::getInt8PtrTy(getGlobalContext()));
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

	BasicBlock* block = BasicBlock::Create(getGlobalContext(), "entry",
			mainFunction);
	builder.SetInsertPoint(block);

	//Value* v = builder.CreateFAdd(ConstantFP::get(getGlobalContext(), APFloat(4.0)), ConstantFP::get(getGlobalContext(), APFloat(5.0)), "a");
	//builder.CreateRet(v);

	for (std::vector<NStatement*>::const_iterator i = statements.begin();
			i != statements.end(); ++i) {
		(*i)->generateCode();
	}
}

void NExpressionStatement::generateCode() const {
	Value* value = expression->generateCode();

	Function* function = translationUnit->module->getFunction("printf");

	GlobalVariable* globalStr = translationUnit->module->getGlobalVariable(".str", true);
	std::vector<Constant*> indices;
	ConstantInt* const_int64_8 = ConstantInt::get(getGlobalContext(), APInt(64, StringRef("0"), 10));
	indices.push_back(const_int64_8);
	indices.push_back(const_int64_8);
	Constant* s = ConstantExpr::getGetElementPtr(globalStr, indices);

	std::vector<Value*> argValues;
	argValues.push_back(s);
	argValues.push_back(value);

	function->dump();

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

