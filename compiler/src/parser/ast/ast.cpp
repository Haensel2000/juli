#include "ast.h"
#include <codegen/llvm/translationUnit.h>
#include <iostream>

#include <llvm/Analysis/Verifier.h>

using namespace juli;

juli::NBasicType::NBasicType(const std::string& name) :
		name(name) {
}

void juli::NBasicType::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);
	if (flags & FLAG_TREE) {
		os << "BasicType: " << name << std::endl;
	} else {
		os << name;
	}
}

const Type* NBasicType::resolve(const TypeInfo& types) const
		throw (CompilerError) {
	return types.getType(name, this);
}

juli::NArrayType::NArrayType(NType* elementType) :
		elementType(elementType) {
}

void juli::NArrayType::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "ArrayType: " << std::endl;
		elementType->print(os, indent + 2, flags);
	} else {
		os << elementType << "[]";
	}
}

const Type* NArrayType::resolve(const TypeInfo& types) const
		throw (CompilerError) {
	return new ArrayType(elementType->resolve(types));
}

juli::NExpression::NExpression(NodeType nodeType, const Type* expressionType) :
		Node(nodeType), expressionType(expressionType) {
}

const Type* juli::NExpression::getExpressionType() const {
	return expressionType;
}

void juli::NExpression::printType(std::ostream& os) const {
	if (expressionType) {
		os << " [" << expressionType << "]" << std::endl;
	} else {
		os << " [underived]" << std::endl;
	}
}

juli::NStringLiteral::NStringLiteral(std::string value) :
		NLiteral<std::string>(STRING_LITERAL, value,
				new ArrayType(&PrimitiveType::INT8_TYPE)) {

	std::stringstream sstream;
	unsigned char escCount = 0;
	for (std::string::iterator i = value.begin(); i != value.end(); ++i) {
		if (*i == '\\') {
			escCount++;
		} else if (escCount == 1) {
			switch (*i) {
			case 'n':
				sstream << "\n";
				break;
			default:
				sstream << "\\" << *i;
				break;
			}
			escCount = 0;
		} else if (escCount > 1) {
			for (int j = 0; j < escCount; ++j)
				sstream << "\\";
			sstream << *i;
		} else {
			sstream << *i;
		}
	}

	origValue = value;
	this->value = sstream.str();

}

void juli::NStringLiteral::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "Literal: " << origValue;
		printType(os);

	} else {
		os << "\"" << origValue << "\"";
	}
}

juli::NIdentifier::NIdentifier(const std::string& name) :
		NExpression(VARIABLE_REF), name(name) {
}

void juli::NIdentifier::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "VariableRef: " << name;
		printType(os);
	} else {
		os << name;
	}
}

juli::NCast::NCast(NExpression* expression, NType* target) :
		NExpression(CAST), expression(expression), target(target) {

}

void juli::NCast::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		if (!target)
			os << "Implicit ";
		os << "Cast: ";
		printType(os);
		expression->print(os, indent + 2, flags);
		if (target)
			target->print(os, indent + 2, flags);
	} else {
		if (target)
			os << "(" << target << ")";
		else
			os << "(implicit)";
		os << expression;
	}
}

juli::NFunctionCall::NFunctionCall(const std::string& id,
		ExpressionList& arguments) :
		NExpression(FUNCTION_CALL), id(id), arguments(arguments) {
}

juli::NFunctionCall::NFunctionCall(const std::string& id) :
		NExpression(FUNCTION_CALL), id(id) {
}

void juli::NFunctionCall::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionCall: " << id;
		printType(os);
		for (ExpressionList::const_iterator i = arguments.begin();
				i != arguments.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
	} else {
		os << id << "(" << arguments << ")";
	}
}

juli::NArrayAccess::NArrayAccess(NExpression* ref, NExpression* index) :
		NExpression(ARRAY_ACCESS), ref(ref), index(index) {
}

void juli::NArrayAccess::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "ArrayAccess: ";
		printType(os);
		ref->print(os, indent + 2, flags);
		index->print(os, indent + 2, flags);
	} else {
		os << ref << "[" << index << "]";
	}
}

juli::NUnaryOperator::NUnaryOperator(NExpression* expression, Operator op) :
		NExpression(UNARY_OPERATOR), expression(expression), op(op) {

}

void juli::NUnaryOperator::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "UnaryOperator: " << op;
		printType(os);
		expression->print(os, indent + 2, flags);
	} else {
		os << op << expression;
	}
}

juli::NBinaryOperator::NBinaryOperator(NExpression* lhs, Operator op,
		NExpression* rhs) :
		NExpression(BINARY_OPERATOR), lhs(lhs), op(op), rhs(rhs) {
}

void juli::NBinaryOperator::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "BinaryOperator: " << op;
		printType(os);
		lhs->print(os, indent + 2, flags);
		rhs->print(os, indent + 2, flags);
	} else {
		os << "(" << lhs << " " << op << " " << rhs << ")";
	}
}

juli::NStatement::NStatement(NodeType nodeType) :
		Node(nodeType) {
}

juli::NExpressionStatement::NExpressionStatement(NExpression* expression) :
		NStatement(EXPRESSION), expression(expression) {
}

void juli::NExpressionStatement::print(std::ostream& os, int indent,
		unsigned int flags) const {
	//beginLine(os, indent);
	//os << expression;
	expression->print(os, indent, flags);
}

juli::NAssignment::NAssignment(const std::string& lhs, NExpression* rhs) :
		NStatement(ASSIGNMENT), lhs(lhs), rhs(rhs) {
}

void juli::NAssignment::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "Assignment: " << lhs << std::endl;
		rhs->print(os, indent + 2, flags);
	} else {
		os << lhs << " = " << rhs;
	}
}

juli::NBlock::NBlock() :
		NStatement(BLOCK) {
}

void juli::NBlock::addStatement(NStatement* statement) {
	statements.push_back(statement);
}

void juli::NBlock::print(std::ostream& os, int indent,
		unsigned int flags) const {
	for (std::vector<NStatement*>::const_iterator i = statements.begin();
			i != statements.end(); ++i) {
		(*i)->print(os, indent, flags);
	}
	if (!(flags & FLAG_TREE)) {
		beginLine(os, indent);
		os << ";" << std::endl;
	}
}

juli::NVariableDeclaration::NVariableDeclaration(NType* type,
		const std::string& name, NExpression *assignmentExpr) :
		NStatement(VARIABLE_DECL), name(name), type(type), assignmentExpr(
				assignmentExpr) {
}

void juli::NVariableDeclaration::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "VariableDeclaration: " << name << std::endl;
		type->print(os, indent + 2, flags);
		if (assignmentExpr)
			assignmentExpr->print(os, indent + 2, flags);
	} else {
		os << type << " " << name;
		if (assignmentExpr) {
			os << " = " << assignmentExpr;
		}
	}
}

juli::NReturnStatement::NReturnStatement(NExpression* expression) :
		NStatement(RETURN), expression(expression) {
}

void juli::NReturnStatement::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "Return: " << std::endl;
		if (expression)
			expression->print(os, indent + 2, flags);
	} else {
		os << "return ";
		if (expression)
			os << expression;
	}
}

juli::NIfClause::NIfClause(NExpression* condition, NBlock* body) :
		condition(condition), body(body) {
}

juli::NIfStatement::NIfStatement(std::vector<NIfClause*> clauses) :
		NStatement(IF), clauses(clauses) {
}

void juli::NIfStatement::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "If: " << std::endl;
		for (std::vector<NIfClause*>::const_iterator i = clauses.begin();
				i != clauses.end(); ++i) {
			beginLine(os, indent + 2);
			os << "IfClause: " << std::endl;
			(*i)->condition->print(os, indent + 4, flags);
			(*i)->body->print(os, indent + 4, flags);
		}
	} else {
		std::vector<NIfClause*>::const_iterator first = clauses.begin();
		os << "if " << (*first)->condition << std::endl;
		os << (*first)->body;
		for (std::vector<NIfClause*>::const_iterator i = ++first;
				i != clauses.end(); ++i) {
			if ((*i)->condition)
				os << "else if " << (*i)->condition << std::endl;
			else
				os << "else" << std::endl;
			os << (*i)->body;
		}
	}
}

juli::NWhileStatement::NWhileStatement(NExpression* condition, NBlock* body) :
		NStatement(WHILE), condition(condition), body(body) {
}

void juli::NWhileStatement::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "While: " << std::endl;
		condition->print(os, indent + 2, flags);
		body->print(os, indent + 2, flags);
	} else {
		os << "while (" << condition << ")" << std::endl;
		body->print(os, indent + 2, flags);
	}
}

juli::NFunctionSignature::NFunctionSignature(const NType* type,
		const std::string& name, const VariableList arguments, bool varArgs) :
		name(name), type(type), arguments(arguments), varArgs(varArgs) {
}

void juli::NFunctionSignature::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionSignature: " << name << std::endl;
		type->print(os, indent + 2, flags);
		for (VariableList::const_iterator i = arguments.begin();
				i != arguments.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
		beginLine(os, indent + 2);
		os << "VarArgs: " << varArgs << std::endl;
	} else {
		os << type << " " << name << "(" << arguments;
		if (varArgs)
			os << ", ...";
		os << ")";
	}
}

juli::NFunctionDefinition::NFunctionDefinition(NFunctionSignature * signature,
		NBlock* body) :
		NStatement(FUNCTION_DEF), signature(signature), body(body) {
}

void juli::NFunctionDefinition::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionDefinition: " << std::endl;
		signature->print(os, indent + 2, flags);
		if (body)
			body->print(os, indent + 2, flags);
	} else {
		os << signature << std::endl;
		if (body) {
			os << body;
		}
	}
}
