#include "parser.h"
#include <iostream>

#include <parser/antlr/JLParser.h>
#include <parser/antlr/JLLexer.h>

using namespace juli;
using namespace std;

TranslationUnit* juli::Parser::parse(const string& filename) {
	pANTLR3_INPUT_STREAM input;
	pANTLR3_COMMON_TOKEN_STREAM tokenStream;
	pJLParser parser;
	pJLLexer lexer;

	input = antlr3FileStreamNew((pANTLR3_UINT8) filename.c_str(), ANTLR3_ENC_UTF8);
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

	TranslationUnit* translationUnit = parser->translation_unit(parser);

	std::cout << "Parsing finished" << std::endl;

	std::cout << translationUnit->getAST() << std::endl;

	parser->free(parser);
	parser = NULL;
	tokenStream->free(tokenStream);
	tokenStream = NULL;
	lexer->free(lexer);
	lexer = NULL;
	input->close(input);
	input = NULL;

	return translationUnit;
}
