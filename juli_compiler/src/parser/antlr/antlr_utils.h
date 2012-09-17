/*
 * antlr_utils.h
 *
 *  Created on: Sep 8, 2012
 *      Author: hannes
 */

#ifndef ANTLR_UTILS_H_
#define ANTLR_UTILS_H_

#include <string>
#include <antlr3.h>

const char* getTokenText(pANTLR3_COMMON_TOKEN token);

std::string getTokenString(pANTLR3_COMMON_TOKEN token);

pANTLR3_STRING getString(const char* s);

#endif /* ANTLR_UTILS_H_ */
