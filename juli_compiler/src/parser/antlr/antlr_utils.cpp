#include "antlr_utils.h"
#include <parser/parser.h>

const char* getTokenText(pANTLR3_COMMON_TOKEN token) {
	return (const char*)(token->getText(token)->chars);
}

std::string getTokenString(pANTLR3_COMMON_TOKEN token) {
	return std::string(getTokenText(token));
}

pANTLR3_STRING getString(const char* s) {
	return juli::Parser::getString(s);
}
