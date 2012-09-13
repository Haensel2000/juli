#include <iostream>
#include <fstream>
#include <parser/antlr/JLParser.h>
#include <parser/antlr/JLLexer.h>
#include <debug/print.h>
#include <parser/ast/node.h>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include <llvm/ExecutionEngine/GenericValue.h>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetLibraryInfo.h>

#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/PathV1.h>

#include "llvm/Support/Host.h"

//extern NBlock* programBlock;
//extern int yyparse();

using namespace llvm;

using std::cerr;

int main(int argc, char **argv) {
	pANTLR3_INPUT_STREAM input;
	pANTLR3_COMMON_TOKEN_STREAM tokenStream;
	pJLParser parser;
	pJLLexer lexer;

	input = antlr3FileStreamNew((pANTLR3_UINT8) argv[1], ANTLR3_ENC_UTF8);
	if (input == NULL) {
		cerr << "Could not find file " << argv[1] << std::endl;
		return 1;
	}

	lexer = JLLexerNew(input);
	if (lexer == NULL) {
		cerr << "Unable to create the lexer due to malloc() failure1"
				<< std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
			TOKENSOURCE(lexer));
	if (tokenStream == NULL) {
		cerr << "Out of memory trying to allocate token stream" << std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	parser = JLParserNew(tokenStream);
	if (parser == NULL) {
		cerr << "Out of memory trying to allocate parser" << std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	NBlock* program = parser->translation_unit(parser);

	std::cout << "Parsing finished" << std::endl;

	std::cout << program << std::endl;

	program->generateCode();

	Module* module = program->getTranslationUnit().module;

	module->dump();

	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetDisassembler();

	/*EngineBuilder b(module);
	 ExecutionEngine* ee = b.create();
	 vector<GenericValue> noargs;
	 GenericValue v = ee->runFunction(module->getFunction("main"), noargs);*/

//	InitializeAllTargets();
//	InitializeAllTargetMCs();
//	InitializeAllAsmPrinters();
//	InitializeAllAsmParsers();

	PassRegistry *Registry = PassRegistry::getPassRegistry();
	initializeCore(*Registry);
	initializeCodeGen(*Registry);
	initializeLoopStrengthReducePass(*Registry);
	initializeLowerIntrinsicsPass(*Registry);
	initializeUnreachableBlockElimPass(*Registry);

	std::string errorMsg("Target not supported.");

	const Target* target = TargetRegistry::getClosestTargetForJIT(errorMsg);

	printf("%s\n", target->getName());

	string targetTriple = llvm::sys::getDefaultTargetTriple();
	Triple TheTriple;
	TheTriple.setTriple(targetTriple);

	printf("%s\n", targetTriple.c_str());

	string cpu = llvm::sys::getHostCPUName();

	StringMap<bool> features;

	llvm::sys::getHostCPUFeatures(features);

	for (StringMap<bool>::iterator i = features.begin(); i != features.end();
			++i) {
		std::cout << i->getKey() << " --> " << i->getValue() << std::endl;
	}

	TargetOptions opt;

	TargetMachine* targetMachine = target->createTargetMachine(targetTriple,
			cpu, "", opt);

//	std::auto_ptr<TargetMachine>
//	    target(TheTarget->createTargetMachine(TheTriple.getTriple(),
//	                                          MCPU, FeaturesStr, Options,
//	                                          RelocModel, CMModel, OLvl));
//
//	  TargetMachine &Target = *target.get();
//
	std::ofstream os("test", std::ios::out | std::ios::binary);
	raw_os_ostream ros(os);
	formatted_raw_ostream fos(ros);

	PassManager passManager;

	// Add an appropriate TargetLibraryInfo pass for the module's triple.
	TargetLibraryInfo *TLI = new TargetLibraryInfo(TheTriple);
	passManager.add(TLI);

	// Add the target data from the target machine, if it exists, or the module.
	if (const TargetData *TD = targetMachine->getTargetData())
		passManager.add(new TargetData(*TD));
	else
		passManager.add(new TargetData(module));

	// Override default to generate verbose assembly.
	targetMachine->setAsmVerbosityDefault(true);

	bool b = targetMachine->addPassesToEmitFile(passManager, fos,
			TargetMachine::CGFT_ObjectFile);
	if (b) {
		printf("ERROR: cannot emit file");
	}

	passManager.run(*module);

	fos.flush();
	os.close();

//	const llvm::sys::Path gccPath("gcc");
//	const char* args[] = {
//			"gcc", "test", "-o", "test.out", 0
//	};
//
//	llvm::sys::Program::ExecuteAndWait(gccPath, args);

	//putc('*', stdout);
	//fflush(stdout);

	parser->free(parser);
	parser = NULL;
	tokenStream->free(tokenStream);
	tokenStream = NULL;
	lexer->free(lexer);
	lexer = NULL;
	input->close(input);
	input = NULL;

}
