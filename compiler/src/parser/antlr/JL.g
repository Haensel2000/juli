grammar JL;

options {
  language = C;
  //backtrack = true;
}

@includes {
  #include <iostream>
  #include <string>
  #include <sstream>
  #include <cstdio>
  #include <cstring>
  
  #include <parser/ast/ast.h>
  #include <parser/ast/types.h>
  #include <parser/antlr/antlr_utils.h>
}

@postinclude {
  std::string filename;
}

translation_unit[const std::string& fn] returns [juli::NBlock* result = 0]:
{
  filename = fn;
  result = new juli::NBlock();
}
(stmt=statement { result->addStatement(stmt); })+ 

{
  setSourceLoc(result, *result->statements.begin(), *(--result->statements.end()));
}
;

statement returns [juli::NStatement* result = 0]: 
stmt1=assignment { result = stmt1; } |
stmt3=return_statement { result = stmt3; } |
stmt4=function_definition { result = stmt4; } |
stmt4=class_definition { result = stmt4; } |
stmt5=variable_definition { result = stmt5; } |
stmt7=if_statement { result = stmt7; } |
stmt8=while_statement { result = stmt8; }
;

import_statement returns [juli::NStatement* result = 0]:
IMPORT id=identifier
{
  
}
;

class_definition returns [juli::NStatement* result = 0]
@declarations
{
  juli::FieldList fields;
}:
STRUCT id=identifier
OCBR 
(fd=field_declaration { fields.push_back(fd); })*
CCBR
{
  result = new juli::NClassDefinition(id, fields);
  setSourceLoc(result, filename, $STRUCT, $CCBR);
}
;

field_declaration returns [juli::NFieldDeclaration* result = 0]:
t=type i=identifier SCOL
{
  result = new juli::NFieldDeclaration(t, i);
  setSourceLoc(result, t, $SCOL);
}
;

function_definition returns [juli::NStatement* result = 0]
@declarations 
{
  juli::NBlock* bl = 0;
}:
decl=function_declaration (b=block { bl = b; } | SCOL)
{
  result = new juli::NFunctionDefinition(decl, bl);
  if (bl) {
    setSourceLoc(result, decl, bl);
  } else {
    setSourceLoc(result, decl, $SCOL);
  }
}
;

block returns [juli::NBlock* result = 0]:
{
  result = new juli::NBlock();
}
OCBR
(stmt=statement { result->addStatement(stmt); })*
CCBR
{
  setSourceLoc(result, filename, $OCBR, $CCBR);
}
;

while_statement returns [juli::NStatement* result = 0]:
WHILE OPAR cond=expression CPAR b=block
{
  result = new juli::NWhileStatement(cond, b);
  setSourceLoc(result, $WHILE, b);
}
;

if_statement returns [juli::NStatement* result = 0]
@declarations
{
  std::vector<juli::NIfClause*> clauses;
}:
cl=if_clause { clauses.push_back(cl); cl->first = true; }
('else' cl=if_clause { clauses.push_back(cl); })*
(cl=else_clause { clauses.push_back(cl); })?
{
  result = new juli::NIfStatement(clauses);
  setSourceLoc(result, *clauses.begin(), *(--clauses.end()));
}
;

if_clause returns [juli::NIfClause* result = 0]:
IF OPAR cond=expression CPAR bl=block
{
  result = new juli::NIfClause(cond, bl);
  setSourceLoc(result, $IF, bl);
}
;

else_clause returns [juli::NIfClause* result = 0]:
ELSE bl=block
{
  result = new juli::NIfClause(0, bl);
  setSourceLoc(result, $ELSE, bl);
}
;

function_declaration returns [juli::NFunctionSignature* result = 0]
@declarations
{
   juli::VariableList arguments;
   juli::NType* type;
   std::string name;
   bool varArgs = false;
   bool cmod = false;
}:
(C_MOD { cmod = true; })? sign=variable_declaration { name = sign->name->name; type = sign->type; }
OPAR
(first_arg=variable_declaration { arguments.push_back(first_arg); }
(',' arg=variable_declaration { arguments.push_back(arg); } )
*)
?
(',' VarArgs { varArgs = true; } )?
CPAR
{
  result = new juli::NFunctionSignature(type, name, arguments, varArgs, (cmod) ? juli::MODIFIER_C : 0);
  if (cmod) {
    setSourceLoc(result, filename, $C_MOD, $CPAR);
  } else {
    setSourceLoc(result, sign, $CPAR);
  }
}
;

variable_definition returns [juli::NVariableDeclaration* result = 0]
@declarations
{
  juli::NExpression* exp = 0;
}:
vtype=type id=identifier ('=' e=expression { exp = e; })? SCOL
{
  result = new juli::NVariableDeclaration(vtype, id, exp);
  setSourceLoc(result, vtype, $SCOL);
}
;

variable_declaration returns [juli::NVariableDeclaration* result = 0]:
vtype=type id=identifier
{
  result = new juli::NVariableDeclaration(vtype, id);
  setSourceLoc(result, vtype, id);
}
;

return_statement returns [juli::NReturnStatement* result = 0]
@declarations {
  juli::NExpression* exp = 0;
}:
RETURN
(e=expression { exp = e; })? 
SCOL
{
  result = new juli::NReturnStatement(exp);
  setSourceLoc(result, filename, $RETURN, $SCOL);
}
;

expression_statement returns [juli::NExpressionStatement* result = 0]:
exp=expression SCOL 
{ 
  result = new juli::NExpressionStatement(exp);
  setSourceLoc(result, exp, $SCOL); 
}
;

assignment returns [juli::NStatement* result = 0]
@declarations
{
  juli::NExpression* rhs = 0;
}: 
lhs=expression 
(
  '=' e=expression
  {
    rhs = e;
  }
)? 
SCOL
{
  if (rhs) {
    result = new juli::NAssignment(lhs, rhs);
  } else {
    result = new juli::NExpressionStatement(lhs);
  }
  setSourceLoc(result, lhs, $SCOL);
}
;

expression returns [juli::NExpression* result = 0]:
  val=relation { result = val; } |
  val=allocation { result = val; }
;

relation returns [juli::NExpression* result = 0]
@declarations
{
   juli::Operator type = juli::UNKNOWN;
}
:
  op1=comparison { result = op1; }
  (
    ( OP_LAND      { type = juli::LAND; }
    | OP_LOR     { type = juli::LOR; }
    )
    op2=comparison
    { 
      result = new juli::NBinaryOperator(result, type, op2);
      setSourceLoc(result, op1, op2);
    }
  )*
;

comparison returns [juli::NExpression* result = 0]
@declarations
{
   juli::Operator type = juli::UNKNOWN;
}
:
  op1=add { result = op1; }
  (
    ( OP_EQ      { type = juli::EQ; }
    | OP_NEQ     { type = juli::NEQ; }
    | OP_LT     { type = juli::LT; }
    | OP_GT     { type = juli::GT; }
    | OP_LEQ     { type = juli::LEQ; }
    | OP_GEQ     { type = juli::GEQ; }
    )
    op2=add  
    { 
      result = new juli::NBinaryOperator(result, type, op2);
      setSourceLoc(result, op1, op2);
    }
  )*
;

add returns [juli::NExpression* result = 0]
@declarations
{
   juli::Operator type = juli::UNKNOWN;
}
:
  op1=mul { result=op1; }
  (
    ( OP_PLUS      { type = juli::PLUS; }
    | OP_MINUS   { type = juli::SUB; }
    )
    op2=mul
    { 
      result = new juli::NBinaryOperator(result, type, op2);
      setSourceLoc(result, op1, op2);
    }
  )*
;

mul returns [juli::NExpression* result = 0]
@declarations
{
   juli::Operator type = juli::UNKNOWN;
}
:
  op1=unary { result=op1; }
  (
    ( OP_MUL    { type = juli::MUL; }
    | OP_DIV    { type = juli::DIV; }
    | OP_MOD    { type = juli::MOD; }
    )
    op2=unary  
    { 
      result = new juli::NBinaryOperator(result, type, op2);
      setSourceLoc(result, op1, op2);
    }
  )*
;

unary returns [juli::NExpression* result = 0]
@declarations
{
  juli::Operator type = juli::UNKNOWN;
  juli::NUnaryOperator* current = 0;
  pANTLR3_COMMON_TOKEN operatorToken = 0;
}:
  (
    ( OP_NOT    { type = juli::NOT; operatorToken = $OP_NOT; }
    | OP_TILDE  { type = juli::TILDE; operatorToken = $OP_TILDE; }
    | OP_HASH   { type = juli::HASH; operatorToken = $OP_HASH; }
    | OP_MINUS  { type = juli::MINUS; operatorToken = $OP_MINUS; }
    )
    {
      if (current) {
        juli::NUnaryOperator* uop = new juli::NUnaryOperator(0, type);
        current->expression = uop;
        setSourceLoc(uop, filename, operatorToken);
        setSourceLoc(current, current, uop);
        current = uop;
        
      } else {
        current = new juli::NUnaryOperator(0, type);
        setSourceLoc(current, filename, operatorToken);
      }
    }
  )*
  
  op=qualified_access  
  { 
    if (current) {
      current->expression = op;
      setSourceLoc(current, current, op);
      result = current;
    } else {
      result = op;
    }
  }
;

qualified_access returns [juli::NExpression* result = 0]:
  op1=array_access  { result=op1; }
  (
    '.'
    op2=qarray_access
    { 
      juli::NArrayAccess* aa = dynamic_cast<juli::NArrayAccess*>(op2);
      if (aa) {
        aa->ref = new juli::NQualifiedAccess(result, dynamic_cast<juli::NVariableRef*>(aa->ref));
        result = aa;
      } else {
        result = new juli::NQualifiedAccess(result, dynamic_cast<juli::NVariableRef*>(op2));
      }
      setSourceLoc(result, op1, op2);
    }
  )*
;

array_access returns [juli::NExpression* result = 0]
@declarations
{
  std::vector<juli::NExpression*> indices;
}:
vref=term { result = vref; }
(OSBR vindex=expression { indices.clear(); indices.push_back(vindex); } 
(COMMA i=expression { indices.push_back(i); })* CSBR 
{ 
  result = new juli::NArrayAccess(result, indices);
  setSourceLoc(result, vref, $CSBR);
}
)*
;

qarray_access returns [juli::NExpression* result = 0]
@declarations
{
  std::vector<juli::NExpression*> indices;
}:
vref=identifier { result = new juli::NVariableRef(vref); }
(OSBR vindex=expression { indices.clear(); indices.push_back(vindex); } 
(COMMA i=expression { indices.push_back(i); })* CSBR 
{ 
  result = new juli::NArrayAccess(result, indices);
  setSourceLoc(result, vref, $CSBR);
}
)*
;

alloc_array returns [juli::NExpression* result = 0]
@declarations
{
  std::vector<juli::NExpression*> indices;
}:
NEW
t=type
(OSBR vindex=expression { indices.clear(); indices.push_back(vindex); } 
(COMMA i=expression { indices.push_back(i); })* CSBR 
{ 
  result = new juli::NAllocateArray(t, indices);
  setSourceLoc(result, filename, $NEW, $CSBR);
}
)
;

alloc_object returns [juli::NExpression* result = 0]:
NEW
t=basic_type
{ 
  result = new juli::NAllocateObject(static_cast<juli::NBasicType*>(t));
  setSourceLoc(result, $NEW, t);
}
;

term returns [juli::NExpression* result = 0]:
val=literal { result = val; } |
s=identifier { result = new juli::NVariableRef(s); } |
val=function_call { result = val; } |
OPAR val=expression CPAR 
{ 
  result = val;
  setSourceLoc(result, filename, $OPAR, $CPAR); 
}
;

allocation returns [juli::NExpression* result = 0]:
val=alloc_array { result = val; } |
val=alloc_object { result = val; }
;

literal returns [juli::NExpression* result = 0]: 
val=double_literal { result = val; } | 
val=string_literal { result = val; } |
val=char_literal { result = val; } |
val=boolean_literal { result = val; } |
val=integer_literal { result = val; } |
val=null_literal { result = val; }
;

function_call returns [juli::NFunctionCall* result = 0]
@declarations 
{
  juli::ExpressionList arguments;
}:
id=identifier 
  OPAR
    (arg=expression { arguments.push_back(arg); })? 
    (',' arg=expression { arguments.push_back(arg); })* 
  CPAR
{
  result = new juli::NFunctionCall(id, arguments);
  setSourceLoc(result, id, $CPAR);
}
;

type returns [juli::NType* result = 0]:
t=array_type  { result = t; }
;

array_type returns [juli::NType* result = 0]:
t=nested_array_type { result = t; }|
t=multi_array_type { result = t; }
;

nested_array_type returns [juli::NType* result = 0]:
t=basic_type { result = t; }
(ARRAY_SUFFIX
{ 
  result = new juli::NArrayType(result);
  setSourceLoc(result, t, $ARRAY_SUFFIX);
})*
;

multi_array_type returns [juli::NType* result = 0]
@declarations 
{
  int dimension = 1;
}:
t=basic_type { result = t; }
OSBR
(
COMMA { dimension++; }
)*
CSBR
{
  result = new juli::NArrayType(result, dimension);
  setSourceLoc(result, t, $CSBR);
}
;

basic_type returns [juli::NType* result = 0]:
s=identifier          { result = new juli::NBasicType(s); } 
;

identifier returns [juli::NIdentifier* result = 0]:
Identifier 
{
  result = new juli::NIdentifier(getTokenString($Identifier)); 
  setSourceLoc(result, filename, $Identifier);
} 
;

double_literal returns [juli::NExpression* result = 0]:
FloatingPointLiteral
{ 
  std::stringstream valueStr(getTokenString($FloatingPointLiteral));
  double value = 0.0;
  valueStr >> value;
  result = new juli::NLiteral<double>(juli::DOUBLE_LITERAL, value, &juli::PrimitiveType::FLOAT64_TYPE); 
  setSourceLoc(result, filename, $FloatingPointLiteral);
} 
;

string_literal returns [juli::NExpression* result = 0]:
StringLiteral
{
  std::string tokenText = getTokenString($StringLiteral);
  tokenText = tokenText.substr(1, tokenText.size() - 2);
  result = new juli::NStringLiteral(tokenText);
  setSourceLoc(result, filename, $StringLiteral);
}
;

char_literal returns [juli::NExpression* result = 0]:
CharacterLiteral
{
  std::string tokenText = getTokenString($CharacterLiteral);
  tokenText = tokenText.substr(1, tokenText.size() - 2);
  result = new juli::NCharLiteral(tokenText);
  setSourceLoc(result, filename, $CharacterLiteral);
}
;

boolean_literal returns [juli::NExpression* result = 0]:
  TRUE    
  { 
    result = new juli::NLiteral<bool>(juli::BOOLEAN_LITERAL, true, &juli::PrimitiveType::BOOLEAN_TYPE); 
    setSourceLoc(result, filename, $TRUE);
  } 
| FALSE   
{ 
  result = new juli::NLiteral<bool>(juli::BOOLEAN_LITERAL, false, &juli::PrimitiveType::BOOLEAN_TYPE); 
  setSourceLoc(result, filename, $FALSE);
} 
;

null_literal returns [juli::NExpression* result = 0]:
  NIL    
  { 
    result = new juli::NLiteral<int>(juli::NULL_LITERAL, 0, &juli::PrimitiveType::NULL_TYPE); 
    setSourceLoc(result, filename, $NIL);
  }
;


integer_literal returns [juli::NExpression* result = 0]:
DecimalLiteral
{ 
  std::stringstream valueStr(getTokenString($DecimalLiteral));
  uint64_t value = 0;
  valueStr >> value;
  result = new juli::NLiteral<uint64_t>(juli::INTEGER_LITERAL, value, &juli::PrimitiveType::INT32_TYPE);
  setSourceLoc(result, filename, $DecimalLiteral);
}
;

// LEXER

VarArgs : '...' ;

HexLiteral : '0' ('x'|'X') HexDigit+ IntegerTypeSuffix? ;

DecimalLiteral : ('0' | '1'..'9' '0'..'9'*) IntegerTypeSuffix? ;

OctalLiteral : '0' ('0'..'7')+ IntegerTypeSuffix? ;

fragment
HexDigit : ('0'..'9'|'a'..'f'|'A'..'F') ;

fragment
IntegerTypeSuffix : ('l'|'L') ;

FloatingPointLiteral
    :   ('0'..'9')+ '.' ('0'..'9')* Exponent? FloatTypeSuffix?
    |   '.' ('0'..'9')+ Exponent? FloatTypeSuffix?
    |   ('0'..'9')+ Exponent FloatTypeSuffix?
    |   ('0'..'9')+ FloatTypeSuffix
    ;

fragment
Exponent : ('e'|'E') ('+'|'-')? ('0'..'9')+ ;

fragment
FloatTypeSuffix : ('f'|'F'|'d'|'D') ;

CharacterLiteral
    :   '\'' ( EscapeSequence | ~('\''|'\\') ) '\''
    ;

StringLiteral
    :  '"' ( EscapeSequence | ~('\\'|'"') )* '"'
    ;

fragment
EscapeSequence
    :   '\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
    |   UnicodeEscape
    |   OctalEscape
    ;

fragment
OctalEscape
    :   '\\' ('0'..'3') ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7')
    ;

fragment
UnicodeEscape
    :   '\\' 'u' HexDigit HexDigit HexDigit HexDigit
    ;
    
OP_PLUS : '+' ;
OP_MINUS : '-' ;
OP_MUL : '*' ;
OP_DIV : '/' ;
OP_MOD : '%' ;
OP_EQ : '==' ;
OP_NEQ : '!=' ;
OP_LT : '<' ;
OP_GT : '>' ;
OP_LEQ : '<=' ;
OP_GEQ : '>=' ;
OP_LOR : 'or' ;
OP_LAND : 'and' ;
OP_NOT : 'not' ;
OP_TILDE : '~' ;
OP_HASH : '#' ;
RETURN : 'return' ;
IMPORT : 'import' ;
IF : 'if' ;
ELSE : 'else' ;
WHILE : 'while' ;
NEW : 'new' ;
TRUE : 'true' ;
FALSE : 'false' ;
NIL : 'null' ;
STRUCT : 'struct' ;
ARRAY_SUFFIX : '[]' ;
OPAR : '(' ;
CPAR : ')' ;
OSBR : '[' ;
CSBR : ']' ;
OCBR : '{' ;
CCBR : '}' ;
SCOL : ';' ;
COMMA : ',' ;

C_MOD : 'C' ;
    
Identifier 
    :   Letter (Letter|JavaIDDigit)*
    ;
    


/**I found this char range in JavaCC's grammar, but Letter and Digit overlap.
   Still works, but...
 */
fragment
Letter
    :  '\u0024' |
       '\u0041'..'\u005a' |
       '\u005f' |
       '\u0061'..'\u007a' |
       '\u00c0'..'\u00d6' |
       '\u00d8'..'\u00f6' |
       '\u00f8'..'\u00ff' |
       '\u0100'..'\u1fff' |
       '\u3040'..'\u318f' |
       '\u3300'..'\u337f' |
       '\u3400'..'\u3d2d' |
       '\u4e00'..'\u9fff' |
       '\uf900'..'\ufaff'
    ;

fragment
JavaIDDigit
    :  '\u0030'..'\u0039' |
       '\u0660'..'\u0669' |
       '\u06f0'..'\u06f9' |
       '\u0966'..'\u096f' |
       '\u09e6'..'\u09ef' |
       '\u0a66'..'\u0a6f' |
       '\u0ae6'..'\u0aef' |
       '\u0b66'..'\u0b6f' |
       '\u0be7'..'\u0bef' |
       '\u0c66'..'\u0c6f' |
       '\u0ce6'..'\u0cef' |
       '\u0d66'..'\u0d6f' |
       '\u0e50'..'\u0e59' |
       '\u0ed0'..'\u0ed9' |
       '\u1040'..'\u1049'
   ;

WS  :  (' '|'\r'|'\t'|'\u000C'|'\n') {$channel=HIDDEN;}
    ;

COMMENT
    :   '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;

LINE_COMMENT
    : '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    ;

//IDENTIFIER : CHAR ALPHANUM* ;
//DOUBLE_LITERAL : DIGIT+ ('.' DIGIT+)? ;
////NEWLINE : (('\u000C')?('\r')? '\n' )+;
//OP_PLUS : '+' ;
//WS : (' ' | '\t' | '\n' | '\r' | '\f')+ { $channel = HIDDEN; };
//fragment ALPHANUM : ('a'..'z' | 'A'..'Z' | '0'..'9') ;
//fragment CHAR : 'a'..'z' | 'A'..'Z' ;
//fragment DIGIT : '0'..'9' ; 