#include "antlr_utils.h"
#include <parser/parser.h>

const char* getTokenText(pANTLR3_COMMON_TOKEN token) {
	return (const char*) (token->getText(token)->chars);
}

std::string getTokenString(pANTLR3_COMMON_TOKEN token) {
	return std::string(getTokenText(token));
}

void setSourceLoc(juli::Indentable* node, const std::string& filename,
		pANTLR3_COMMON_TOKEN token) {
	juli::Marker start = getSourceMarker(token);
	juli::Marker end = getSourceMarker(token, true);

	node->setSourceLocation(filename, start, end);
}

void setSourceLoc(juli::Indentable* node, juli::Indentable* first,
		juli::Indentable* last) {
	node->setSourceLocation(first->filename, first->start, last->end);
}

void setSourceLoc(juli::Indentable* node, juli::Indentable* first,
		pANTLR3_COMMON_TOKEN last) {
	node->setSourceLocation(first->filename, first->start, getSourceMarker(last, true));
}

void setSourceLoc(juli::Indentable* node, pANTLR3_COMMON_TOKEN first,
		juli::Indentable* last) {
	node->setSourceLocation(last->filename, getSourceMarker(first), last->end);
}

void setSourceLoc(juli::Indentable* node, const std::string& filename,
		pANTLR3_COMMON_TOKEN first, pANTLR3_COMMON_TOKEN last) {
	node->setSourceLocation(filename, getSourceMarker(first), getSourceMarker(last, true));
}

juli::Marker getSourceMarker(pANTLR3_COMMON_TOKEN token, bool end) {
	unsigned int line = token->getLine(token);
	unsigned int column = token->getCharPositionInLine(token) + 1;
	if (end) {
		unsigned long tokenLength = token->getStopIndex(token)
					- token->getStartIndex(token) + 1;
		column += tokenLength;
	}
	return juli::Marker(line, column);
}

pANTLR3_STRING getString(const char* s) {
	return juli::Parser::getString(s);
}
