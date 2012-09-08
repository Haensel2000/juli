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

rule returns [NBlock* result]: 
{ result = new NBlock(); }
(stmt=statement { result->addStatement(stmt); })+ 
;

statement returns [NStatement* result]: 
stmt=assignment { result = stmt; } | 
exp=expression { result = new NExpressionStatement(exp); }
;

assignment returns [NAssignment* result]: 
id=identifier '=' exp=expression NEWLINE 
{ result = new NAssignment(id, exp); }
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
    op2=literal  { result = new NBinaryOperator(result, type, op2); }
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
IDENTIFIER { result = new NIdentifier(getTokenString($IDENTIFIER)); } 
;

double_literal returns [NDoubleLiteral* result]:
DOUBLE_LITERAL
{ 
  std::stringstream valueStr(getTokenString($DOUBLE_LITERAL));
  double value = 0.0;
  valueStr >> value;
  result = new NDoubleLiteral(value); 
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