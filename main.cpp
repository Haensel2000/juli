#include <iostream>
#include <fstream>

#include <parser/parser.h>
#include <codegen/codegen.h>


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

//extern NBlock* programBlock;
//extern int yyparse();

using namespace llvm;
using namespace juli;

using std::cerr;

int main(int argc, char **argv) {
	Parser parser;

	TranslationUnit* unit = parser.parse(argv[1]);
	unit->generateCode();
	CodeEmitter emitter;
	emitter.emitCode("test", unit->module);

}
