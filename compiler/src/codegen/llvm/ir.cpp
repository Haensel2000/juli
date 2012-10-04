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

llvm::Value* juli::IRGenerator::staticArrayIndex(llvm::Value* arrAddr,
		int index) {
	std::vector<llvm::Value*> indices;
	indices.push_back(zero_i32);
	//indices.push_back(zero_i32);
	indices.push_back(
			llvm::ConstantInt::get(context, llvm::APInt(32, index, true)));
	std::cerr << "AddrType: ";
	arrAddr->getType()->dump();
	std::cerr << std::endl;
	std::cerr << "Indices: " << indices << std::endl;
	llvm::Value* ptr = builder.CreateGEP(arrAddr, indices);
	std::cerr << "StaticArrayIndex: " << std::endl;
	ptr->dump();
	return builder.CreateLoad(ptr);
}

llvm::ConstantInt* juli::IRGenerator::getConstantInt32(int v) {
	return llvm::ConstantInt::get(context, llvm::APInt(32, v, true));
}

llvm::ConstantFP* juli::IRGenerator::getConstantDouble(double v) {
	return llvm::ConstantFP::get(context, llvm::APFloat(v));
}

unsigned int juli::IRGenerator::getSizeOf(const Type* type) {
	switch (type->getCategory()) {
	case PRIMITIVE: {
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
		return 0;
	}
	case ARRAY: {
		unsigned int pointerSize =
				(module.getPointerSize() == llvm::Module::Pointer64) ? 8 : 4;
		const ArrayType* at = static_cast<const ArrayType*>(type);
		if (*at->getElementType() == PrimitiveType::INT8_TYPE
				&& at->getDimension() == 1) {
			return pointerSize;
		}
		return pointerSize
				+ getSizeOf(&PrimitiveType::INT32_TYPE) * at->getDimension();
	}
	case CLASS: {
		unsigned int sum = 0;
		std::vector<Field> fields =
				static_cast<const ClassType*>(type)->getFields();
		for (std::vector<Field>::const_iterator i = fields.begin();
				i != fields.end(); ++i) {
			sum += getSizeOf(i->type);
		}
		return sum;
	}
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

	//emitClassDefinitions();
}

//void juli::IRGenerator::emitClassDefinitions() {
//	std::vector<Type*> types = typeInfo.getTypes();
//	for (std::vector<Type*>::const_iterator i = types.begin(); i != types.end(); ++i) {
//		ClassType* ct = dynamic_cast<ClassType*>(*i);
//		if (ct) {
//			resolveType(ct);
//		}
//	}
//}

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
				//indices.push_back(zero_i32);
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

llvm::Value* juli::IRGenerator::visitCharLiteral(const NCharLiteral* n) {
	return llvm::ConstantInt::get(context, llvm::APInt(8, n->value, true));
}

llvm::Value* juli::IRGenerator::visitBooleanLiteral(const NLiteral<bool>* n) {
	return llvm::ConstantInt::get(context, llvm::APInt(1, n->value, true));
}

llvm::Value* juli::IRGenerator::visitNullLiteral(const NLiteral<int>* n) {
	return zero_i32;
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

	if (n->ref->expressionType->getCategory() == ARRAY
			&& static_cast<const ArrayType*>(n->ref->expressionType)->getStaticSize()
					>= 0) {
		return getConstantInt32(
				static_cast<const ArrayType*>(n->ref->expressionType)->getStaticSize());
	}

	std::cerr << "Reference: ";
	p->dump();

	llvm::Value * f = fieldPtr(p, n->index);

	std::cerr << "Field Access: ";
	f->dump();

	llvm::Value* result;
	if (n->address
			|| (n->expressionType->getCategory() == ARRAY
					&& static_cast<const ArrayType*>(n->expressionType)->getStaticSize()
							>= 0))
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
		const PrimitiveType* from = dynamic_cast<const PrimitiveType*>(tfrom);
		if (tto->getCategory() == PRIMITIVE) {

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

		} else if (from->getPrimitive() == NIL) {
			if (tto->getCategory() != REFERENCE)
				return builder.CreateIntToPtr(v, targetType);
			else
				return v;
		}
	} else if ((tfrom->getCategory() == CLASS || tfrom->getCategory() == ARRAY)
			&& tto->getCategory() == REFERENCE) {
		return builder.CreatePtrToInt(v, targetType);
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

	if (n->lhs->expressionType->getCategory() == PRIMITIVE
			&& n->rhs->expressionType->getCategory() == PRIMITIVE) {
		const PrimitiveType* pt =
				static_cast<const PrimitiveType*>(n->lhs->expressionType);
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
			left->dump();
			left->getType()->dump();
			right->dump();
			right->getType()->dump();
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
			return builder.CreateOr(left, right, "or_res");
			break;
		case LAND:
			return builder.CreateAnd(left, right, "and_res");
			break;
		case UNKNOWN:
		default:
			std::cerr << "Unsupported binary operator " << n->op << std::endl;
			return 0;
		}
	} else if (n->lhs->expressionType->getCategory() == REFERENCE
			&& n->rhs->expressionType->getCategory() == REFERENCE) {
		switch (n->op) {
		case EQ:
			return builder.CreateICmpEQ(left, right, "eq_res");
		case NEQ:
			return builder.CreateICmpNE(left, right, "ne_res");
		}
	}
	return 0;
}

llvm::Value* juli::IRGenerator::visitAllocateArray(const NAllocateArray* n) {
	llvm::Function* malloc = module.getFunction("malloc");

	const ArrayType* at = dynamic_cast<const ArrayType*>(n->expressionType);

	// Treat char[] differently:
	if (*at->getElementType() == PrimitiveType::INT8_TYPE
			&& n->sizes.size() == 1) {
		llvm::Value* size = visit(n->sizes[0]);
		llvm::Value* sizep1 = builder.CreateAdd(size, one_i32);
		llvm::Value* pi8 = builder.CreateCall(malloc, sizep1);
		llvm::Value* pi8_end = builder.CreateGEP(pi8, size);
		builder.CreateStore(zero_i8, pi8_end);
		return pi8;
	}

	// allocate space to store the array ref:
	llvm::Value* pi8 = builder.CreateCall(malloc,
			getConstantInt32(getSizeOf(n->expressionType)));
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
		if (n->sizes.size() > 1)
			indices.push_back(getConstantInt32(i++));
		llvm::Value* sizePtr = builder.CreateGEP(result, indices);
		sizePtr->dump();
		builder.CreateStore(arraySize, sizePtr)->dump();
	}
	pi8 = builder.CreateCall(malloc, memorySize);
	llvm::Value* ptr = builder.CreateBitCast(pi8,
			llvm::PointerType::get(elementType, 0));
	fieldSet(result, ARRAY_FIELD_PTR, ptr);

	return result;
}

llvm::Value* juli::IRGenerator::visitAllocateObject(const NAllocateObject* n) {
	llvm::Function* malloc = module.getFunction("malloc");
	llvm::Value* size = getConstantInt32(getSizeOf(n->expressionType));
	llvm::Value* pi8 = builder.CreateCall(malloc, size);
	pi8->dump();
	llvm::Value* result = builder.CreateBitCast(pi8,
			resolveType(n->expressionType));
	result->dump();

	return result;
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
	std::cerr << "VREF: " << std::endl;
	vref->dump();
	llvm::Value* vindex;
	if (n->indices.size() > 1) {
		llvm::Value* lengthArray = fieldPtr(vref, ARRAY_FIELD_LENGTH);
		std::cerr << "lengthArray: " << std::endl;
		lengthArray->dump();
		llvm::Value* factor = one_i32;
		int c = 0;
		vindex = zero_i32;
		for (ExpressionList::const_iterator i = n->indices.begin();
				i != n->indices.end(); ++i) {
			llvm::Value * currentIndex = visit(*i);
			std::cerr << "currentIndex: " << std::endl;
			currentIndex->dump();
			llvm::Value * offsetIndex = builder.CreateMul(currentIndex, factor);
			std::cerr << "offsetIndex: " << std::endl;
			offsetIndex->dump();
			vindex = builder.CreateAdd(vindex, offsetIndex);
			std::cerr << "VINDEX: " << std::endl;
			vindex->dump();
			factor = builder.CreateMul(staticArrayIndex(lengthArray, c++),
					factor);
			std::cerr << "factor: " << std::endl;
			factor->dump();
		}
	} else {
		vindex = visit(n->indices[0]);
	}

	llvm::Value* ptr;
	llvm::Value* result;
	vindex->dump();
	const ArrayType* at = dynamic_cast<const ArrayType*>(n->ref->expressionType);

	if (*n->expressionType == PrimitiveType::INT8_TYPE
			&& at->getDimension() == 1) {
		ptr = builder.CreateGEP(vref, vindex);
	} else {
		vindex->dump();
		if (at->getStaticSize() >= 0) {
			std::vector<llvm::Value*> indices;
			indices.push_back(zero_i32);
			indices.push_back(zero_i32);
			indices.push_back(vindex);
			ptr = builder.CreateGEP(vref, vindex);
			ptr->dump();
		} else {
			vindex->dump();
			std::vector<llvm::Value*> indices;
			indices.push_back(zero_i32);
			indices.push_back(zero_i32);

			llvm::Value* p_dataPtr = builder.CreateGEP(vref, indices);
			p_dataPtr->dump();
			llvm::Value* dataPtr = builder.CreateLoad(p_dataPtr);
			dataPtr->dump();
			vindex->dump();
			vindex->getType()->dump();
			ptr = builder.CreateGEP(dataPtr, vindex);
			ptr->dump();
		}
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

llvm::Value* juli::IRGenerator::visitClassDef(const NClassDefinition* n) {
	return 0;
}

llvm::Value* juli::IRGenerator::visit(const Node* n) {
	return visitAST<IRGenerator, llvm::Value*>(*this, n);
}

void juli::IRGenerator::process(const Node* n) {
	visit(n);
}
