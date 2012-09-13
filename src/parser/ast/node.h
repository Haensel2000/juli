#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdio>

#include <debug/print.h>

namespace juli {

enum Operator {
	PLUS, UNKNOWN
};

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class TranslationUnit;

class Node {
protected:
	TranslationUnit* translationUnit;
public:
	Node(TranslationUnit* module) :
			translationUnit(module) {
	}

	virtual ~Node() {
	}

	void beginLine(std::ostream& os, int indent) const {
		os << std::string(indent * 2, ' ');
	}

	virtual void print(std::ostream& os, int indent) const = 0;

	const TranslationUnit& getTranslationUnit() const  {
		return *translationUnit;
	}
//
//	const llvm::Module& getModule() const {
//		return *(translationUnit->module);
//	}

};

}

std::ostream& operator<<(std::ostream& os, const juli::Node* object);

std::ostream& operator<<(std::ostream& os, const juli::Node& object);

#endif
