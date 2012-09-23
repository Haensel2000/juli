/*
 * parser.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <antlr3.h>
#include <codegen/llvm/translationUnit.h>

using std::string;

namespace juli {

class Parser {
private:
	static pANTLR3_STRING_FACTORY strFactory;
public:

	Parser();

	~Parser();

	static pANTLR3_STRING getString(const char* s);

	NBlock* parse(const string& filename);

};

}


#endif /* PARSER_H_ */
