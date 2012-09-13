#include "antlr_utils.h"

const char* getTokenText(pANTLR3_COMMON_TOKEN token) {
	return (const char*)(token->getText(token)->chars);
}

std::string getTokenString(pANTLR3_COMMON_TOKEN token) {
	return std::string(getTokenText(token));
}
