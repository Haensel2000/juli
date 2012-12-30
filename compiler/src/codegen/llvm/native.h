/*
 * codegen.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <string>
#include <ostream>
#include <fstream>

#include <llvm/PassManager.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/FormattedStream.h>
//#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/PathV1.h>

#include "llvm/Support/Host.h"

namespace juli {

class CodeEmitter {
public:

	CodeEmitter(bool allTargets = false);

	static llvm::TargetMachine* getNativeMachine();

	void emitCode(const char* filename, llvm::Module* module, llvm::TargetMachine* machine = getNativeMachine());

	void emitCode(std::ostream& stream, llvm::Module* module, llvm::TargetMachine* machine = getNativeMachine());

};

}


#endif /* CODEGEN_H_ */
