/*
 * ir.h
 *
 *  Created on: Sep 18, 2012
 *      Author: hannes
 */

#ifndef IR_H_
#define IR_H_

#include <parser/ast/ast.h>

namespace juli {

class IRGenerator {
public:

	void visit(const Node* n);

	TranslationUnit* generate(const Node* ast);

};

}

#endif /* IR_H_ */
