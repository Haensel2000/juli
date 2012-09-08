#include <iostream>
#include <parser/antlr/JLParser.h>
#include <parser/antlr/JLLexer.h>
#include <debug/print.h>
#include <parser/ast/node.h>
//extern NBlock* programBlock;
//extern int yyparse();

using std::cerr;

int main(int argc, char **argv) {
	pANTLR3_INPUT_STREAM input;
	pANTLR3_COMMON_TOKEN_STREAM tokenStream;
	pJLParser parser;
	pJLLexer lexer;

	input = antlr3FileStreamNew((pANTLR3_UINT8) argv[1], ANTLR3_ENC_UTF8);
	if (input == NULL) {
		cerr << "Could not find file " << argv[1] << std::endl;
		return 1;
	}

	lexer = JLLexerNew(input);
	if (lexer == NULL) {
		cerr << "Unable to create the lexer due to malloc() failure1"
				<< std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
			TOKENSOURCE(lexer));
	if (tokenStream == NULL) {
		cerr << "Out of memory trying to allocate token stream" << std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	parser = JLParserNew(tokenStream);
	if (parser == NULL) {
		cerr << "Out of memory trying to allocate parser" << std::endl;
		return ANTLR3_ERR_NOMEM;
	}

	NBlock* program = parser->rule(parser);

	std::cout << "Parsing finished" << std::endl;

	std::cout << program << std::endl;

	//putc('*', stdout);
	//fflush(stdout);

	parser->free(parser);
	parser = NULL;
	tokenStream->free(tokenStream);
	tokenStream = NULL;
	lexer->free(lexer);
	lexer = NULL;
	input->close(input);
	input = NULL;

}
