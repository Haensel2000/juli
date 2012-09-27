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
#include <parser/ast/node.h>

const char* getTokenText(pANTLR3_COMMON_TOKEN token);

std::string getTokenString(pANTLR3_COMMON_TOKEN token);

void setSourceLoc(juli::Indentable* node, const std::string& filename, pANTLR3_COMMON_TOKEN token);

void setSourceLoc(juli::Indentable* node, juli::Indentable* first, juli::Indentable* last);

void setSourceLoc(juli::Indentable* node, juli::Indentable* first, pANTLR3_COMMON_TOKEN last);

void setSourceLoc(juli::Indentable* node, pANTLR3_COMMON_TOKEN first, juli::Indentable* last);

void setSourceLoc(juli::Indentable* node, const std::string& filename, pANTLR3_COMMON_TOKEN first, pANTLR3_COMMON_TOKEN last);

juli::Marker getSourceMarker(pANTLR3_COMMON_TOKEN token, bool end = false);

pANTLR3_STRING getString(const char* s);

#endif /* ANTLR_UTILS_H_ */
