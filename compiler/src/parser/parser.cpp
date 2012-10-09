#include "parser.h"
#include <iostream>

#include <parser/antlr/JLParser.h>
#include <parser/antlr/JLLexer.h>

using namespace juli;
using namespace std;

pANTLR3_STRING_FACTORY Parser::strFactory;

pANTLR3_STRING juli::Parser::getString(const char* s) {
	return strFactory->newStr(strFactory, (pANTLR3_UINT8) s);
}

juli::Parser::Parser() {
	strFactory = antlr3StringFactoryNew(ANTLR3_ENC_UTF8);
}

juli::Parser::~Parser() {
	delete strFactory;
}

NBlock* juli::Parser::parse(const string& filename) {
	pANTLR3_INPUT_STREAM input;
	pANTLR3_COMMON_TOKEN_STREAM tokenStream;
	pJLParser parser;
	pJLLexer lexer;

	input = antlr3FileStreamNew((pANTLR3_UINT8) filename.c_str(),
			ANTLR3_ENC_UTF8);
	if (input == NULL) {
		cerr << "Could not find file " << filename << std::endl;
		return 0;
	}

	lexer = JLLexerNew(input);
	if (lexer == NULL) {
		cerr << "Unable to create the lexer due to malloc() failure1"
				<< std::endl;
		return 0;
	}

	tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
			TOKENSOURCE(lexer));
	if (tokenStream == NULL) {
		cerr << "Out of memory trying to allocate token stream" << std::endl;
		return 0;
	}

	parser = JLParserNew(tokenStream);
	if (parser == NULL) {
		cerr << "Out of memory trying to allocate parser" << std::endl;
		return 0;
	}

	NBlock* ast = parser->translation_unit(parser, filename);

	parser->free(parser);
	parser = NULL;
	tokenStream->free(tokenStream);
	tokenStream = NULL;
	lexer->free(lexer);
	lexer = NULL;
	input->close(input);
	input = NULL;

	return ast;
}
