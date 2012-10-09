#include <iostream>
#include <fstream>

#include <parser/parser.h>
#include <codegen/llvm/native.h>
#include <debug/print.h>
#include <parser/ast/node.h>
#include <analysis/type/declare.h>
#include <analysis/type/typecheck.h>
#include <codegen/llvm/ir.h>
#include <builder/builder.h>

#include <llvm/Support/CommandLine.h>

using namespace llvm;
using namespace juli;

using std::cerr;

cl::opt<string> inputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);
cl::opt<string> outputFilename("o", cl::desc("Specify output filename"), cl::value_desc("filename"), cl::Required);
cl::opt<string> outputIRFilename("irtext", cl::desc("Output ir assembly code"), cl::value_desc("filename"));
cl::opt<string> outputASTFilename("ast", cl::desc("Output debug ast"), cl::value_desc("filename"));

int main(int argc, char **argv) {
	int result = 0;

	cl::ParseCommandLineOptions(argc, argv);

	CodeEmitter emitter;
	Importer importer;
	Parser parser;

	importer.add(new SourceImportLoader(parser, importer));
	try {
		Node* ast = parser.parse(inputFilename);

		Declarator declarator(importer);
		TypeInfo* typeInfo = declarator.declare(ast);
		typeInfo->resolveClasses();

		TypeChecker typeChecker(*typeInfo);
		typeChecker.visit(ast);

		if (!outputASTFilename.empty()) {
			std::ofstream astos(outputASTFilename.c_str());
			ast->print(astos, 0, Indentable::FLAG_TREE);
		}

		IRGenerator irgen("test", *typeInfo);
		irgen.process(ast);

		if (!outputIRFilename.empty()) {
			std::ofstream iros(outputIRFilename.c_str());
			llvm::raw_os_ostream ros(iros);
			irgen.getTranslationUnit().module->print(ros, 0);
		}


		if (irgen.getTranslationUnit().getErrors().empty()) {
			emitter.emitCode(outputFilename.c_str(), irgen.getTranslationUnit().module);
		} else {
			std::vector<CompilerError> errors = irgen.getTranslationUnit().getErrors();
			for (std::vector<CompilerError>::iterator i = errors.begin(); i != errors.end(); ++i) {
				std::cerr << *i;
			}
			result = 1;
		}

		delete typeInfo;

	} catch (Error& ce) {
		cerr << "Uncaught error: " << ce;
		result = 2;
	}
	return result;
}
