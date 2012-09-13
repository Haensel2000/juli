#include "translationUnit.h"

using namespace juli;

juli::TranslationUnit::TranslationUnit(const std::string& name) {
	module = new llvm::Module(name, llvm::getGlobalContext());
}

juli::TranslationUnit::~TranslationUnit() {
	delete module;
	delete ast;
}

llvm::LLVMContext& juli::TranslationUnit::getContext() {
	return module->getContext();
}

void juli::TranslationUnit::generateCode() {
	llvm::IRBuilder<> builder(module->getContext());
	generateCode(builder);
}

void juli::TranslationUnit::generateCode(llvm::IRBuilder<> builder) {
	ast->generateCode(builder);
}

void juli::TranslationUnit::setAST(NBlock* ast) {
	this->ast = ast;
}

NBlock* juli::TranslationUnit::getAST() {
	return ast;
}
