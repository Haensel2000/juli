grammar JL;

options {
  language = C;
}

@includes {
  #include <iostream>
  #include <string>
  #include <sstream>
  #include <cstdio>
  
  #include <parser/ast/ast.h>
  #include <codegen/translationUnit.h>
  #include <parser/antlr/antlr_utils.h>
}

@postinclude {
  juli::TranslationUnit* translationUnit;
}

translation_unit returns [juli::TranslationUnit* result]
@declarations
{
   juli::NBlock* block;
}:
{ 
  result = new juli::TranslationUnit("test");
  block = new juli::NBlock(result); 
  result->setAST(block);
  translationUnit = result;
}
(stmt=statement { block->addStatement(stmt); })+ 
;

statement returns [juli::NStatement* result]: 
stmt1=assignment { result = stmt1; } | 
stmt2=expression_statement { result = stmt2; }
;

expression_statement returns [juli::NExpressionStatement* result]:
exp=expression NEWLINE { result = new juli::NExpressionStatement(translationUnit, exp); }
;

assignment returns [juli::NAssignment* result]: 
id=identifier '=' exp=expression NEWLINE 
{ result = new juli::NAssignment(translationUnit, id, exp); }
;

expression returns [juli::NExpression* result]
@declarations
{
   juli::Operator type = juli::UNKNOWN;
}
:
  op1=literal { result=op1; }
  (
    OP_PLUS      { type = juli::PLUS; } 
    op2=literal  { result = new juli::NBinaryOperator(translationUnit, result, type, op2); }
  )*
;

literal returns [juli::NExpression* result]: 
val=double_literal { result = val; }
| 
val=identifier { result = val; } | 
'(' val=expression ')'
{ result = val; }
;

identifier returns [juli::NIdentifier* result]:
IDENTIFIER { result = new juli::NIdentifier(translationUnit, getTokenString($IDENTIFIER)); } 
;

double_literal returns [juli::NDoubleLiteral* result]:
DOUBLE_LITERAL
{ 
  std::stringstream valueStr(getTokenString($DOUBLE_LITERAL));
  double value = 0.0;
  valueStr >> value;
  result = new juli::NDoubleLiteral(translationUnit, value); 
} 
;

IDENTIFIER : CHAR ALPHANUM* ;
DOUBLE_LITERAL : DIGIT+ ('.' DIGIT+)? ;
NEWLINE : (('\u000C')?('\r')? '\n' )+;
OP_PLUS : '+' ;
WS : (' ' | '\t' | '\n' | '\r' | '\f')+ { $channel = HIDDEN; };
fragment ALPHANUM : ('a'..'z' | 'A'..'Z' | '0'..'9') ;
fragment CHAR : 'a'..'z' | 'A'..'Z' ;
fragment DIGIT : '0'..'9' ; 