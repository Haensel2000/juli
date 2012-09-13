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

		TranslationUnit(const std::string& name) {
			module = new llvm::Module(name, llvm::getGlobalContext());
		}

		~TranslationUnit() {
			delete module;
			delete ast;
		}

		llvm::LLVMContext& getContext() {
			return module->getContext();
		}

		void generateCode() {
			llvm::IRBuilder<> builder(module->getContext());
			generateCode(builder);
		}

		void generateCode(llvm::IRBuilder<> builder) {
			ast->generateCode(builder);
		}

		void setAST(NBlock* ast) {
			this->ast = ast;
		}

		NBlock* getAST() {
			return ast;
		}
	};

}


#endif /* CODGEN_H_ */
