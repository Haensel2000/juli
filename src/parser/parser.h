/*
 * parser.h
 *
 *  Created on: Sep 13, 2012
 *      Author: hannes
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string>

#include <codegen/translationUnit.h>

using std::string;

namespace juli {

class Parser {
private:
public:

	TranslationUnit* parse(const string& filename);

};

}


#endif /* PARSER_H_ */
