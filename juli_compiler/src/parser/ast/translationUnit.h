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
#include <parser/ast/types.h>

#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

#include <parser/ast/error.h>

namespace juli {

	class TranslationUnit {
	private:
		StatementList statements;
		std::map<std::string, Type*> typeTable;
		std::map<std::string, llvm::Value*> symbolTable;

		mutable std::vector<CompilerError> compilerErrors;
	public:
		llvm::Module* module;


		TranslationUnit(const std::string& name);

		~TranslationUnit();

		llvm::LLVMContext& getContext();

		void generateCode();

		void generateCode(llvm::IRBuilder<> builder);

		void addStatement(NStatement* statement);

		const StatementList getStatements();

		const Type* resolveType(const NIdentifier* id) const throw (CompilerError);

		std::map<std::string, llvm::Value*>& getSymbolTable() {
			return symbolTable;
		}

		const std::vector<CompilerError>& getErrors() const {
			return compilerErrors;
		}

		void reportError(CompilerError& e) const {
			compilerErrors.push_back(e);
		}

		void printStatements(std::ostream& os) const {
			for (StatementList::const_iterator i = statements.begin(); i != statements.end(); ++i) {
				os << *i << std::endl;
			}
		}


	};

}


#endif /* CODGEN_H_ */
