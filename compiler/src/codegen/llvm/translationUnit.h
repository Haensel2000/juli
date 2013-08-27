/*
 * codgen.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef CODGEN_H_
#define CODGEN_H_

#include <map>
#include <stdexcept>

#include <parser/ast/ast.h>
#include <parser/ast/types.h>

#include <llvm/DerivedTypes.h>
#include <llvm/IRBuilder.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Type.h>

#include <analysis/error.h>
#include <compiler_component.h>

namespace juli {

	class TranslationUnit : public CompilerComponent {
	private:
		StatementList statements;

		std::map<std::string, llvm::Value*> llvmSymbolTable;

		const TypeInfo& types;

		std::string getLLVMTypeName(const Type* t) const;

		llvm::Type* getType(const Type* t) const;
		llvm::Type* createClassType(const ClassType* t) const;

		llvm::Type* getPointerIntType() const;
	public:
		llvm::Module* module;

		TranslationUnit(std::vector<Error>& errors, const std::string& name, const TypeInfo& types);

		~TranslationUnit();

		llvm::LLVMContext& getContext() const;

		llvm::Type* resolveLLVMType(const Type* t) const throw (CompilerError);
		llvm::Type* resolveLLVMType(const NType* t) const throw (CompilerError);

		llvm::Value* getSymbol(const std::string& name, const Node* n) {
			try {
				return llvmSymbolTable.at(name);
			} catch (std::out_of_range& e) {
				CompilerError err(n);
				err.getStream() << "Unknown symbol " << name;
				errors.push_back(err);
                return 0;
			}
		}

		void addSymbol(const std::string& name, llvm::Value* value) {
			llvmSymbolTable[name] = value;
		}

		void removeSymbol(const std::string& name) {
			llvmSymbolTable.erase(name);
		}

		void reportError(CompilerError& e) const {
			errors.push_back(e);
		}

		void printStatements(std::ostream& os) const {
			for (StatementList::const_iterator i = statements.begin(); i != statements.end(); ++i) {
				os << *i << ";" << std::endl;
			}
		}


	};

}


#endif /* CODGEN_H_ */
