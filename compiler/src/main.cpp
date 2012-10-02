#include <iostream>
#include <fstream>

#include <parser/parser.h>
#include <codegen/llvm/native.h>

#include <debug/print.h>
#include <parser/ast/node.h>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include <llvm/ExecutionEngine/GenericValue.h>

#include <llvm/PassManager.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/PathV1.h>

#include "llvm/Support/Host.h"

#include <analysis/type/declare.h>
#include <analysis/type/typecheck.h>
#include <codegen/llvm/ir.h>

//extern NBlock* programBlock;
//extern int yyparse();

using namespace llvm;
using namespace juli;

using std::cerr;

int main(int argc, char **argv) {

	CodeEmitter emitter;
	Parser parser;

	for (int i = 1; i < argc; ++i) {
		std::cout << "Building file: " << argv[i] << std::endl;
		try {
			NBlock* ast = parser.parse(argv[i]);

			Declarator declarator;
			declarator.visit(ast);

			//declarator.getTypeInfo().dump();

			TypeChecker typeChecker(declarator.getTypeInfo());
			typeChecker.visit(ast);

			ast->print(std::cout, 0, Indentable::FLAG_TREE);

			IRGenerator irgen("test", declarator.getTypeInfo());
			irgen.process(ast);

			irgen.getTranslationUnit().module->dump();

			if (irgen.getTranslationUnit().getErrors().empty()) {
				std::string filename(argv[i]);
				std::string basename = filename.substr(0,
						filename.find_last_of('.', filename.size()));
				filename = basename + ".o";
				emitter.emitCode(filename.c_str(),
						irgen.getTranslationUnit().module);
			} else {
				std::vector<CompilerError> errors =
						irgen.getTranslationUnit().getErrors();
				for (std::vector<CompilerError>::iterator i = errors.begin();
						i != errors.end(); ++i) {
					std::cerr << *i;
				}
			}

		} catch (CompilerError& ce) {
			cerr << "Uncaught error: " << ce;
		}

	}

}
