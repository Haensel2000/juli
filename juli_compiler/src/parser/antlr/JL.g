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
  #include <parser/ast/translationUnit.h>
  #include <parser/antlr/antlr_utils.h>
}

@postinclude {
  juli::TranslationUnit* translationUnit;
}

translation_unit [const char* moduleName] returns [juli::TranslationUnit* result]:
{ 
  result = new juli::TranslationUnit("test");
  translationUnit = result;
}
(stmt=function_definition { result->addStatement(stmt); })+ 
;

statement returns [juli::NStatement* result]: 
stmt1=assignment { result = stmt1; } | 
stmt2=expression_statement { result = stmt2; } |
stmt3=return_statement { result = stmt3; } |
stmt4=function_definition { result = stmt4; } |
stmt5=variable_declaration ';' { result = stmt5; }
;


function_definition returns [juli::NFunctionDefinition* result]:
decl=function_declaration bl=block
{
  result = new juli::NFunctionDefinition(translationUnit, decl, bl);
}
;

block returns [juli::NBlock* result]:
{
  result = new juli::NBlock(translationUnit);
}
'{'
(stmt=statement { result->addStatement(stmt); })*
'}' 
;

function_declaration returns [juli::NFunctionDeclaration* result]
@declarations
{
   juli::VariableList arguments;
   juli::NIdentifier* type;
   juli::NIdentifier* id;
}:
sign=variable_declaration { id = sign->id; type = sign->type; }
'(' 
(first_arg=variable_declaration { arguments.push_back(first_arg); }
(',' arg=variable_declaration { arguments.push_back(arg); } )
*)
? 
')'
{
  result = new juli::NFunctionDeclaration(translationUnit, type, id, arguments);
}
;

variable_declaration returns [juli::NVariableDeclaration* result]:
type=identifier id=identifier
{
  result = new juli::NVariableDeclaration(translationUnit, type, id);
}
;

return_statement returns [juli::NReturnStatement* result]:
{
  result = new juli::NReturnStatement(translationUnit, 0);
}
'return' 
(exp=expression {result = new juli::NReturnStatement(translationUnit, exp);})? 
';'
;

expression_statement returns [juli::NExpressionStatement* result]:
exp=expression ';' { result = new juli::NExpressionStatement(translationUnit, exp); }
;

assignment returns [juli::NAssignment* result]: 
id=identifier '=' exp=expression ';' 
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
val=double_literal { result = val; } | 
val=identifier { result = val; } |
val=function_call { result = val; } | 
'(' val=expression ')' { result = val; }
;

function_call returns [juli::NFunctionCall* result]
@declarations 
{
  juli::ExpressionList arguments;
}:
id=identifier 
  '(' 
    (arg=expression { arguments.push_back(arg); })? 
    (',' arg=expression { arguments.push_back(arg); })* 
  ')'
{
  result = new juli::NFunctionCall(translationUnit, id, arguments);
}
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
//NEWLINE : (('\u000C')?('\r')? '\n' )+;
OP_PLUS : '+' ;
WS : (' ' | '\t' | '\n' | '\r' | '\f')+ { $channel = HIDDEN; };
fragment ALPHANUM : ('a'..'z' | 'A'..'Z' | '0'..'9') ;
fragment CHAR : 'a'..'z' | 'A'..'Z' ;
fragment DIGIT : '0'..'9' ; 