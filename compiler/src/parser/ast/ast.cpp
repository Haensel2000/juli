#include "ast.h"
#include <codegen/llvm/translationUnit.h>
#include <iostream>

#include <llvm/Analysis/Verifier.h>

using namespace juli;

const Type* NBasicType::resolve(const TypeInfo& types) const  throw (CompilerError) {
	return types.getType(name);
}

const Type* NArrayType::resolve(const TypeInfo& types) const  throw (CompilerError)  {
	return new ArrayType(elementType->resolve(types));
}
