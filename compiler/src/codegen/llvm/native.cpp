#include "native.h"

using namespace juli;

juli::CodeEmitter::CodeEmitter(bool allTargets) {
	if (allTargets) {
		/*llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmPrinters();
		llvm::InitializeAllAsmParsers();*/
	} else {
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();
		llvm::InitializeNativeTargetDisassembler();
	}
}

llvm::TargetMachine* juli::CodeEmitter::getNativeMachine() {

	std::string errorMsg("Target not supported.");
	const llvm::Target* target = llvm::TargetRegistry::getClosestTargetForJIT(
			errorMsg);

	llvm::Triple targetTriple;
	targetTriple.setTriple(llvm::sys::getDefaultTargetTriple());

	std::string cpu = llvm::sys::getHostCPUName();

	llvm::TargetOptions opt;

	llvm::TargetMachine* targetMachine = target->createTargetMachine(
			targetTriple.getTriple(), cpu, "", opt);

	return targetMachine;
}

void juli::CodeEmitter::emitCode(const char* filename, llvm::Module* module,
		llvm::TargetMachine* machine) {
	std::ofstream os(filename, std::ios::out | std::ios::binary);
	emitCode(os, module, machine);
	os.close();
}

void juli::CodeEmitter::emitCode(std::ostream& os, llvm::Module* module,
		llvm::TargetMachine* machine) {

	llvm::raw_os_ostream ros(os);
	llvm::formatted_raw_ostream fos(ros);

	llvm::PassManager passManager;

	bool b = machine->addPassesToEmitFile(passManager, fos,
			llvm::TargetMachine::CGFT_ObjectFile);
	if (b) {
		printf("ERROR: cannot emit file");
	}

	passManager.run(*module);

	fos.flush();
}
