/*
 * codgen.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef CODGEN_H_
#define CODGEN_H_

#include <map>

#include <parser/ast/ast.h>

#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

namespace juli {

	class TranslationUnit {
	private:
		NBlock* ast;
	public:
		llvm::Module* module;
		std::map<std::string, llvm::Value*> namedValues;

		TranslationUnit(const std::string& name);

		~TranslationUnit();

		llvm::LLVMContext& getContext();

		void generateCode();

		void generateCode(llvm::IRBuilder<> builder);

		void setAST(NBlock* ast);

		NBlock* getAST();
	};

}


#endif /* CODGEN_H_ */
