#include "ast.h"
#include <codegen/llvm/translationUnit.h>
#include <iostream>

#include <llvm/Analysis/Verifier.h>

using namespace juli;

const Type* NBasicType::resolve(const TranslationUnit& module) const  throw (CompilerError) {
	return module.getType(name);
}

const Type* NArrayType::resolve(const TranslationUnit& module) const  throw (CompilerError)  {
	return new ArrayType(elementType->resolve(module));
}
