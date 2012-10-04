#include "ir.h"

#include <parser/ast/visitor.h>
#include <analysis/type/functions.h>

#include <llvm/Analysis/Verifier.h>
#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

using namespace juli;

const int juli::IRGenerator::ARRAY_FIELD_PTR = 0;
const int juli::IRGenerator::ARRAY_FIELD_LENGTH = 1;

llvm::Value* juli::IRGenerator::fieldPtr(llvm::Value* objAddr, int index) {
	std::vector<llvm::Value*> indices;
	indices.push_back(zero_i32);
	indices.push_back(
			llvm::ConstantInt::get(context, llvm::APInt(32, index, true)));
	return builder.CreateGEP(objAddr, indices);
}

llvm::Value* juli::IRGenerator::fieldGet(llvm::Value* objAddr, int index) {
	return builder.CreateLoad(fieldPtr(objAddr, index));
}

void juli::IRGenerator::fieldSet(llvm::Value* objAddr, int index,
		llvm::Value* value) {
	builder.CreateStore(value, fieldPtr(objAddr, index));
}

llvm::ConstantInt* juli::IRGenerator::getConstantInt32(int v) {
	return llvm::ConstantInt::get(context, llvm::APInt(32, v, true));
}

llvm::ConstantFP* juli::IRGenerator::getConstantDouble(double v) {
	return llvm::ConstantFP::get(context, llvm::APFloat(v));
}

unsigned int juli::IRGenerator::getSizeOf(const Type* type) {
	switch (type->getCategory()) {
	case PRIMITIVE:
		switch (static_cast<const PrimitiveType*>(type)->getPrimitive()) {
		case VOID:
			return 0;
		case BOOLEAN:
			return 1;
		case INT8:
			return 1;
		case INT32:
			return 4;
		case FLOAT64:
			return 8;
		}
	case ARRAY:
		unsigned int pointerSize =
				(module.getPointerSize() == llvm::Module::Pointer64) ? 8 : 4;
		return pointerSize
				+ getSizeOf(&PrimitiveType::INT32_TYPE)
						* static_cast<const ArrayType*>(type)->getDimension();
	}

}

juli::IRGenerator::IRGenerator(const std::string& moduleName,
		const TypeInfo& typeInfo) :
		typeInfo(typeInfo), translationUnit(moduleName, typeInfo), builder(
				translationUnit.getContext()), module(*translationUnit.module), context(
				translationUnit.getContext()) {
	zero_ui8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0, bool(false)));
	zero_ui16 = llvm::ConstantInt::get(context,
			llvm::APInt(16, 0, bool(false)));
	zero_ui32 = llvm::ConstantInt::get(context,
			llvm::APInt(32, 0, bool(false)));
	zero_i8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0, true));
	zero_i16 = llvm::ConstantInt::get(context, llvm::APInt(16, 0, true));
	zero_i32 = getConstantInt32(0);

	one_i32 = getConstantInt32(1);

	zero_float = getConstantDouble(0.0);

	array_struct_size = llvm::ConstantInt::get(context,
			llvm::APInt(32, 8, true));

	const Type* t_int8 = &PrimitiveType::INT8_TYPE;
	const Type* t_int32 = &PrimitiveType::INT32_TYPE;
	const Type* t_arr_int8 = new ArrayType(t_int8);

	std::vector<FormalParameter> params;
	params.push_back(FormalParameter(t_arr_int8, "str"));
	createFunction(
			new Function("strlen", t_int32, params, false, MODIFIER_C, 0));

	params.clear();
	params.push_back(FormalParameter(t_int32, "size"));
	createFunction(
			new Function("malloc", t_arr_int8, params, false, MODIFIER_C, 0));
}

llvm::Function* juli::IRGenerator::getFunction(const Function* function) {
	const std::string llvmName = function->mangle();
	std::map<std::string, llvm::Function*>::iterator i = llvmFunctionTable.find(
			llvmName);
	if (i == llvmFunctionTable.end()) {
		llvm::Function* f = createFunction(function);

		if (function->body) {

			llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(context,
					"entry", f);
			builder.SetInsertPoint(llvmBlock);

			if (function->name == "main") {
				llvm::Function::arg_iterator i = f->getArgumentList().begin();

				llvm::Type* arrTypePtr = translationUnit.resolveLLVMType(
						function->formalArguments[0].type);
				llvm::Value* args = builder.CreateAlloca(arrTypePtr);
				llvm::Value* argsValue = builder.CreateAlloca(
						arrTypePtr->getPointerElementType());
				std::vector<llvm::Value*> indices;
				indices.push_back(zero_i32);
				indices.push_back(zero_i32);
				llvm::Value* pPtr = builder.CreateGEP(argsValue, indices); // ptr
				indices.pop_back();
				indices.push_back(one_i32);
				indices.push_back(zero_i32);
				llvm::Value* pLength = builder.CreateGEP(argsValue, indices); // length
				builder.CreateStore(i, pLength);
				builder.CreateStore(++i, pPtr);
				builder.CreateStore(argsValue, args);

				translationUnit.addSymbol(function->formalArguments[0].name,
						args);
			} else {
				llvm::Function::arg_iterator i = f->getArgumentList().begin();
				for (std::vector<FormalParameter>::const_iterator vi =
						function->formalArguments.begin();
						vi != function->formalArguments.end(); ++i, ++vi) {
					//(*vi)->generateCode(builder);

					llvm::Value* param = builder.CreateAlloca(i->getType());
					builder.CreateStore(i, param);
					translationUnit.addSymbol(vi->name, param);
				}
			}

			visit(function->body);

			//	if (f->getReturnType() == llvm::Type::getVoidTy(translationUnit->getContext())) {
			//		builder.CreateRet(0);
			//	}

			for (std::vector<FormalParameter>::const_iterator i =
					function->formalArguments.begin();
					i != function->formalArguments.end(); ++i) {
				translationUnit.removeSymbol(i->name);
			}

			if (llvm::verifyFunction(*f, llvm::PrintMessageAction)) {
				f->dump();
			}

		}
		llvmFunctionTable[llvmName] = f;
	}
	return llvmFunctionTable[llvmName];
}

llvm::Type* juli::IRGenerator::resolveType(const Type* n) {
	return translationUnit.resolveLLVMType(n);
}

llvm::Type* juli::IRGenerator::resolveType(const NType* n) {
	return translationUnit.resolveLLVMType(n);
}

llvm::Value* juli::IRGenerator::visitDoubleLiteral(const NLiteral<double>* n) {
	return llvm::ConstantFP::get(context, llvm::APFloat(n->value));
}

llvm::Value* juli::IRGenerator::visitIntegerLiteral(
		const NLiteral<uint64_t>* n) {
	return llvm::ConstantInt::get(context, llvm::APInt(32, n->value, true));
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

llvm::Value* juli::IRGenerator::visitVariableRef(const NVariableRef* n) {
	llvm::Value* p = translationUnit.getSymbol(n->name, n);
	llvm::Value* result;

	if (n->address)
		result = p;
	else
		result = builder.CreateLoad(p);
	;

	return result;
}

llvm::Value* juli::IRGenerator::visitQualifiedAccess(NQualifiedAccess* n) {
	std::cerr << "Visiting " << n << std::endl;

	llvm::Value* p = visit(n->ref);

	if (p->getType() == llvm::Type::getInt8PtrTy(context)
			&& n->name->name == "length") {
		llvm::Function* function = module.getFunction("strlen");

		std::vector<llvm::Value*> argValues;
		argValues.push_back(p);

		return builder.CreateCall(function, argValues);
	}

	std::cerr << "Reference: ";
	p->dump();

	llvm::Value * f = fieldPtr(p, n->index);

	std::cerr << "Field Access: ";
	f->dump();

	llvm::Value* result;
	if (n->address)
		result = f;
	else
		result = builder.CreateLoad(f);

	std::cerr << "Result: ";
	result->dump();

	return result;
}

llvm::Value* juli::IRGenerator::visitCast(const NCast* n) {
	llvm::Value* v = visit(n->expression);
	llvm::Type* targetType = resolveType(n->expressionType);
	const Type* tfrom = n->expression->expressionType;
	const Type* tto = n->expressionType;
	if (tfrom->getCategory() == PRIMITIVE) {
		if (tto->getCategory() == PRIMITIVE) {
			const PrimitiveType* from =
					dynamic_cast<const PrimitiveType*>(tfrom);
			const PrimitiveType* to = dynamic_cast<const PrimitiveType*>(tto);

			Primitive fp = from->getPrimitive();
			Primitive tp = to->getPrimitive();
			bool firstIsInt = (fp >= INT8 && fp <= INT32);
			bool secondIsInt = (tp >= INT8 && tp <= INT32);
			bool firstIsUnsigned = (firstIsInt);
			bool secondIsUnsigned = (secondIsInt);

			std::cerr << "VISITING CAST:" << std::endl;
			std::cerr << "From = " << from << std::endl;
			std::cerr << "To = " << to << std::endl;

			std::cerr << "fp = " << fp << std::endl;
			std::cerr << "tp = " << tp << std::endl;
			std::cerr << "firstIsInt = " << firstIsInt << std::endl;
			std::cerr << "secondIsInt = " << secondIsInt << std::endl;
			std::cerr << "firstIsUnsigned = " << firstIsUnsigned << std::endl;
			std::cerr << "secondIsUnsigned = " << secondIsUnsigned << std::endl;

			std::cerr << "targetType = ";
			targetType->dump();
			std::cerr << "  valueType = ";
			v->getType()->dump();
			std::cerr << std::endl;

			if (firstIsInt && secondIsInt) {
				if (fp > tp) {
					return builder.CreateTrunc(v, targetType);
				} else {
					if (firstIsUnsigned) {
						return builder.CreateSExt(v, targetType);
					} else {
						return builder.CreateZExt(v, targetType);
					}
				}
			} else if (firstIsInt) {
				if (firstIsUnsigned) {
					return builder.CreateUIToFP(v, targetType);
				} else {
					return builder.CreateSIToFP(v, targetType);
				}
			} else if (secondIsInt) {
				if (secondIsUnsigned) {
					return builder.CreateFPToUI(v, targetType);
				} else {
					return builder.CreateFPToSI(v, targetType);
				}
			}

		}
	}
}

llvm::Value* juli::IRGenerator::visitUnaryOperator(const NUnaryOperator* n) {
	llvm::Value* expressionValue = visit(n->expression);

	const PrimitiveType* pt =
			dynamic_cast<const PrimitiveType*>(n->expressionType);

	switch (n->op) {
	case MINUS:
		if (pt->isFloatingPoint())
			return builder.CreateFSub(zero_float, expressionValue, "neg_res");
		else if (pt->isSignedInteger())
			return builder.CreateSub(zero_i32, expressionValue, "neg_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateSub(zero_ui32, expressionValue, "neg_res");
		break;
	case NOT:
	case TILDE:
		return builder.CreateNot(expressionValue, "not_res");
	case HASH:
		return 0; /* TODO */
	default:
		std::cerr << "Unsupported unary operator " << n->op << std::endl;
		return 0;
	}
	return 0;
}

llvm::Value* juli::IRGenerator::visitBinaryOperator(const NBinaryOperator* n) {
	llvm::Value* left = visit(n->lhs);
	llvm::Value* right = visit(n->rhs);

	if (left == 0 || right == 0) // error handling
		return 0;

	const PrimitiveType* pt =
			dynamic_cast<const PrimitiveType*>(n->lhs->expressionType);

	switch (n->op) {
	case PLUS:
		if (pt->isFloatingPoint())
			return builder.CreateFAdd(left, right, "add_res");
		else if (pt->isInteger())
			return builder.CreateAdd(left, right, "add_res");
		break;
	case SUB:
		if (pt->isFloatingPoint())
			return builder.CreateFSub(left, right, "sub_res");
		else if (pt->isInteger())
			return builder.CreateSub(left, right, "sub_res");
		break;
	case MUL:
		if (pt->isFloatingPoint())
			return builder.CreateFMul(left, right, "mul_res");
		else if (pt->isInteger())
			return builder.CreateMul(left, right, "mul_res");
		break;
	case DIV:
		if (pt->isFloatingPoint())
			return builder.CreateFDiv(left, right, "div_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateUDiv(left, right, "div_res");
		else if (pt->isSignedInteger())
			return builder.CreateSDiv(left, right, "div_res");
		break;
	case MOD:
		if (pt->isUnsignedInteger())
			return builder.CreateURem(left, right, "mod_res");
		else if (pt->isSignedInteger())
			return builder.CreateSRem(left, right, "mod_res");
		break;
	case EQ:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpOEQ(left, right, "eq_res");
		else if (pt->isInteger())
			return builder.CreateICmpEQ(left, right, "eq_res");
		break;
	case NEQ:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpONE(left, right, "ne_res");
		else if (pt->isInteger())
			return builder.CreateICmpNE(left, right, "ne_res");
		break;
	case LT:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpOLT(left, right, "lt_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateICmpULT(left, right, "lt_res");
		else if (pt->isSignedInteger())
			return builder.CreateICmpSLT(left, right, "lt_res");
		break;
	case GT:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpOGT(left, right, "gt_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateICmpUGT(left, right, "gt_res");
		else if (pt->isSignedInteger())
			return builder.CreateICmpSGT(left, right, "lt_res");
		break;
	case LEQ:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpOLE(left, right, "le_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateICmpULE(left, right, "le_res");
		else if (pt->isSignedInteger())
			return builder.CreateICmpSLE(left, right, "lt_res");
		break;
	case GEQ:
		if (pt->isFloatingPoint())
			return builder.CreateFCmpOGE(left, right, "ge_res");
		else if (pt->isUnsignedInteger())
			return builder.CreateICmpUGE(left, right, "ge_res");
		else if (pt->isSignedInteger())
			return builder.CreateICmpSGE(left, right, "lt_res");
		break;
	case LOR:
		return builder.CreateAnd(left, right, "and_res");
		break;
	case LAND:
		return builder.CreateOr(left, right, "and_res");
		break;
	case UNKNOWN:
	default:
		std::cerr << "Unsupported binary operator " << n->op << std::endl;
		return 0;
	}
	return 0;
}

llvm::Value* juli::IRGenerator::visitAllocateArray(const NAllocateArray* n) {
	llvm::Function* malloc = module.getFunction("malloc");

	const ArrayType* at = dynamic_cast<const ArrayType*>(n->expressionType);
	// allocate space to store the array ref:
	llvm::Value* pi8 = builder.CreateCall(malloc, getConstantInt32(getSizeOf(n->expressionType)));
	pi8->dump();
	llvm::Value* result = builder.CreateBitCast(pi8, resolveType(at));
	result->dump();

	// allocate the actual array memory:
	const Type* etype = n->type->resolve(typeInfo);
	llvm::Type* elementType = resolveType(etype);
	std::cerr << "ElementType: " << std::endl;
	elementType->dump();
	std::cerr << std::endl;

	llvm::Value* memorySize = getConstantInt32(getSizeOf(etype));
	std::cerr << "ElementSize: " << std::endl;
	memorySize->dump();
	memorySize = builder.CreateIntCast(memorySize,
			llvm::Type::getInt32Ty(context), true);
	std::cerr << "ElementSize: " << std::endl;
	memorySize->dump();
	int i = 0;
	for (std::vector<NExpression*>::const_iterator s = n->sizes.begin();
			s != n->sizes.end(); ++s) {
		llvm::Value* arraySize = visit(*s);
		std::cerr << "ArraySize: " << std::endl;
		arraySize->dump();
		memorySize = builder.CreateMul(arraySize, memorySize);
		memorySize->dump();

		std::vector<llvm::Value*> indices;
		indices.push_back(zero_i32);
		indices.push_back(one_i32);
		indices.push_back(getConstantInt32(i));
		llvm::Value* sizePtr = builder.CreateGEP(result, indices);
		sizePtr->dump();
		builder.CreateStore(arraySize, sizePtr)->dump();
	}
	pi8 = builder.CreateCall(malloc, memorySize);
	llvm::Value* ptr = builder.CreateBitCast(pi8, llvm::PointerType::get(elementType, 0));
	fieldSet(result, ARRAY_FIELD_PTR, ptr);

	return result;
}

llvm::Value* juli::IRGenerator::visitAllocateObject(const NAllocateObject* n) {
}

llvm::Value* juli::IRGenerator::visitFunctionCall(const NFunctionCall* n) {
	llvm::Function* function = getFunction(n->function);

	std::vector<llvm::Value*> argValues;
	for (unsigned i = 0, e = n->arguments.size(); i != e; ++i) {
		argValues.push_back(visit(n->arguments[i]));
		if (argValues.back() == 0)
			return 0;
	}

	return builder.CreateCall(function, argValues);
}

llvm::Value* juli::IRGenerator::visitArrayAccess(const NArrayAccess* n) {
	std::cerr << "Visiting " << n << std::endl;

	llvm::Value* vref = visit(n->ref);
	llvm::Value* vindex = visit(n->index);
	llvm::Value* ptr;
	llvm::Value* result;
	if (*n->expressionType == PrimitiveType::INT8_TYPE) {
		ptr = builder.CreateGEP(vref, vindex);
	} else {
		std::vector<llvm::Value*> indices;
		indices.push_back(zero_i32);
		indices.push_back(zero_i32);
		ptr = builder.CreateGEP(vref, indices);
		llvm::Value* arr = builder.CreateLoad(ptr);
		ptr = builder.CreateGEP(arr, vindex);
	}
	if (n->address) {
		result = ptr;
	} else {
		result = builder.CreateLoad(ptr);
	}

	std::cerr << "Result: ";
	result->dump();
	return result;
}

llvm::Value* juli::IRGenerator::visitAssignment(const NAssignment* n) {
	llvm::Value* addr = visit(n->lhs);
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

llvm::Value* juli::IRGenerator::visitVariableDecl(
		const NVariableDeclaration* n) {
	llvm::Value* param = builder.CreateAlloca(resolveType(n->type));
	if (n->assignmentExpr)
		builder.CreateStore(visit(n->assignmentExpr), param);
	translationUnit.addSymbol(n->name->name, param);
	return 0;
}

llvm::FunctionType* juli::IRGenerator::createFunctionType(const Function * n) {
	llvm::Type* returnType;
	std::vector<llvm::Type*> argumentTypes;
	if (n->name == "main") {
		returnType = llvm::Type::getInt32Ty(context);
		argumentTypes.push_back(returnType);
		argumentTypes.push_back(
				llvm::PointerType::get(llvm::Type::getInt8PtrTy(context, 0),
						0));
	} else {

		returnType = resolveType(n->resultType);

		for (std::vector<FormalParameter>::const_iterator i =
				n->formalArguments.begin(); i != n->formalArguments.end();
				++i) {
			argumentTypes.push_back(resolveType(i->type));
		}
	}

	return llvm::FunctionType::get(returnType, argumentTypes, n->varArgs);
}

llvm::Function* juli::IRGenerator::createFunction(const Function * n) {
	llvm::Function* f = llvm::Function::Create(createFunctionType(n),
			llvm::Function::ExternalLinkage, n->mangle(), &module);
	return f;
}

llvm::Value* juli::IRGenerator::visitFunctionDef(const NFunctionDefinition* n) {

	getFunction(new Function(n, typeInfo));

	return 0;

//	llvm::Function* f = createFunction(n->signature);
//
//	if (n->body) {
//
//		llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(context, "entry",
//				f);
//		builder.SetInsertPoint(llvmBlock);
//
//		llvm::Function::arg_iterator i = f->getArgumentList().begin();
//		for (VariableList::const_iterator vi = n->signature->arguments.begin();
//				vi != n->signature->arguments.end(); ++i, ++vi) {
//			//(*vi)->generateCode(builder);
//
//			llvm::Value* param = builder.CreateAlloca(i->getType());
//			builder.CreateStore(i, param);
//			translationUnit.getLLVMSymbolTable()[(*vi)->name->name] = param;
//		}
//
//		visit(n->body);
//
////	if (f->getReturnType() == llvm::Type::getVoidTy(translationUnit->getContext())) {
////		builder.CreateRet(0);
////	}
//
//		for (std::vector<NVariableDeclaration*>::const_iterator i =
//				n->signature->arguments.begin();
//				i != n->signature->arguments.end(); ++i) {
//			translationUnit.getLLVMSymbolTable().erase((*i)->name->name);
//		}
//
//		if (llvm::verifyFunction(*f, llvm::PrintMessageAction)) {
//			f->dump();
//		}
//
//	}
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

	llvm::BasicBlock* contBlock = llvm::BasicBlock::Create(context, "continue",
			f);

	while (ifc != n->clauses.end()) {
		if ((*ifc)->condition) {
			llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(context,
					"then", f);
			llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(context,
					"else", f);

			builder.CreateCondBr(visit((*ifc)->condition), thenBlock,
					elseBlock);

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

llvm::Value* juli::IRGenerator::visitWhile(const NWhileStatement* n) {
	llvm::Function* f = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "condition",
			f);
	llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context, "body", f);
	llvm::BasicBlock* contBlock = llvm::BasicBlock::Create(context, "continue",
			f);

	builder.CreateBr(condBlock);
	builder.SetInsertPoint(condBlock);
	builder.CreateCondBr(visit(n->condition), bodyBlock, contBlock);
	builder.SetInsertPoint(bodyBlock);
	visit(n->body);
	builder.CreateBr(condBlock);
	builder.SetInsertPoint(contBlock);

	return 0;

}

llvm::Value* juli::IRGenerator::visit(const Node* n) {
	return visitAST<IRGenerator, llvm::Value*>(*this, n);
}

void juli::IRGenerator::process(const Node* n) {
	visit(n);
}
