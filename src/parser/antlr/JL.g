grammar JL;

options {
  language = C;
}

@includes {
  #include <iostream>
  #include <string>
  #include <sstream>
  #include <cstdio>
  
  #include <parser/ast/node.h>
  #include <parser/antlr/antlr_utils.h>
}

@postinclude {
  TranslationUnit* translationUnit;
}

translation_unit returns [NBlock* result]: 
{ 
  translationUnit = new TranslationUnit("test");
  result = new NBlock(translationUnit); 
}
(stmt=statement { result->addStatement(stmt); })+ 
;

statement returns [NStatement* result]: 
stmt1=assignment { result = stmt1; } | 
stmt2=expression_statement { result = stmt2; }
;

expression_statement returns [NExpressionStatement* result]:
exp=expression NEWLINE { result = new NExpressionStatement(translationUnit, exp); }
;

assignment returns [NAssignment* result]: 
id=identifier '=' exp=expression NEWLINE 
{ result = new NAssignment(translationUnit, id, exp); }
;

expression returns [NExpression* result]
@declarations
{
   Operator type = UNKNOWN;
}
:
  op1=literal { result=op1; }
  (
    OP_PLUS      { type = PLUS; } 
    op2=literal  { result = new NBinaryOperator(translationUnit, result, type, op2); }
  )*
;

literal returns [NExpression* result]: 
val=double_literal { result = val; }
| 
val=identifier { result = val; } | 
'(' val=expression ')'
{ result = val; }
;

identifier returns [NIdentifier* result]:
IDENTIFIER { result = new NIdentifier(translationUnit, getTokenString($IDENTIFIER)); } 
;

double_literal returns [NDoubleLiteral* result]:
DOUBLE_LITERAL
{ 
  std::stringstream valueStr(getTokenString($DOUBLE_LITERAL));
  double value = 0.0;
  valueStr >> value;
  result = new NDoubleLiteral(translationUnit, value); 
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