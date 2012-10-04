#include "ast.h"
#include <codegen/llvm/translationUnit.h>
#include <analysis/type/typeinfo.h>
#include <iostream>

#include <llvm/Analysis/Verifier.h>

using namespace juli;

juli::NBasicType::NBasicType(NIdentifier* id) :
		name(id->name) {
	setSourceLocation(id->filename, id->start, id->end);
}

void juli::NBasicType::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);
	if (flags & FLAG_TREE) {
		os << "BasicType: " << name;
		printLocation(os);
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
		os << "ArrayType: ";
		printLocation(os);
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
		os << " [" << expressionType << "]";
		printLocation(os);
	} else {
		os << " [underived]";
		printLocation(os);
	}
}

NAddressable::NAddressable(NodeType nodeType) :
		NExpression(nodeType), address(false) {
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

//juli::NQualifiedName::NQualifiedName(std::vector<std::string>& path) :
//		path(path) {
//}
//
//void juli::NQualifiedName::print(std::ostream& os, int indent,
//		unsigned int flags) const {
//	beginLine(os, indent);
//
//	if (flags & FLAG_TREE) {
//		os << "Identifier: ";
//		cpputils::debug::print(os, path, ".");
//		printLocation(os);
//	} else {
//		cpputils::debug::print(os, path, ".");
//	}
//}

juli::NQualifiedAccess::NQualifiedAccess(NExpression* ref, NIdentifier* name) :
		NAddressable(QUALIFIED_ACCESS), ref(ref), name(name), index(-1) {
}

juli::NQualifiedAccess::NQualifiedAccess(NExpression* ref, NVariableRef* name) :
		NAddressable(QUALIFIED_ACCESS), ref(ref), name(
				new NIdentifier(name->name)), index(-1) {
}

void juli::NQualifiedAccess::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "QualifiedAccess: ";
		printType(os);
		name->print(os, indent + 2, flags);
		ref->print(os, indent + 2, flags);
	} else {
		os << ref << "." << name;
	}
}

juli::NAllocateArray::NAllocateArray(NArrayAccess* aacc) :
		NExpression(NEW_ARRAY), type(0), sizes() {

	NExpression* ref;
	NArrayAccess* cacc = aacc;
	while (cacc) {
		sizes.push_back(cacc->index);
		ref = cacc->ref;
		cacc = dynamic_cast<NArrayAccess*>(ref);
	}

	type = new NBasicType(new NIdentifier(((NVariableRef*) ref)->name));

}

juli::NAllocateArray::NAllocateArray(NBasicType* type,
		std::vector<NExpression*>& sizes) :
		NExpression(NEW_ARRAY), type(type), sizes(sizes) {
}

const Type* juli::NAllocateArray::getType(const TypeInfo& typeInfo) const {
	return ArrayType::getMultiDimensionalArray(type->resolve(typeInfo), sizes.size());
}

void juli::NAllocateArray::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "AllocateArray: ";
		printType(os);
		type->print(os, indent + 2, flags);
		for (std::vector<NExpression*>::const_iterator i = sizes.begin();
				i != sizes.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
	} else {
		os << "new " << type;
		for (std::vector<NExpression*>::const_iterator i = sizes.begin();
				i != sizes.end(); ++i) {
			os << "[" << *i << "]";
		}
	}
}

juli::NAllocateObject::NAllocateObject(NBasicType* type) :
		NExpression(NEW_OBJECT), type(type) {
}

void juli::NAllocateObject::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "AllocateObject: ";
		printType(os);
		type->print(os, indent + 2, flags);
	} else {
		os << "new " << type;
	}
}

juli::NIdentifier::NIdentifier(const std::string& name) :
		name(name) {
}

void juli::NIdentifier::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "Identifier: " << name;
		printLocation(os);
	} else {
		os << name;
	}
}

juli::NIdentifier::operator std::string() {
	return name;
}

juli::NIdentifier::operator const std::string() const {
	return name;
}

juli::NVariableRef::NVariableRef(NIdentifier* id) :
		NAddressable(VARIABLE_REF), name(id->name) {
	setSourceLocation(id->filename, id->start, id->end);
}

void juli::NVariableRef::print(std::ostream& os, int indent,
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

juli::NFunctionCall::NFunctionCall(NIdentifier* name, ExpressionList& arguments) :
		NExpression(FUNCTION_CALL), name(name), arguments(arguments), function(
				0) {
}

juli::NFunctionCall::NFunctionCall(NIdentifier* name) :
		NExpression(FUNCTION_CALL), name(name), function(0) {
}

void juli::NFunctionCall::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionCall: ";
		printType(os);
		name->print(os, indent + 2, flags);
		for (ExpressionList::const_iterator i = arguments.begin();
				i != arguments.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
	} else {
		os << name << "(" << arguments << ")";
	}
}

juli::NArrayAccess::NArrayAccess(NExpression* ref, NExpression* index) :
		NAddressable(ARRAY_ACCESS), ref(ref), index(index) {
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

	//os << expression;
	if (flags & FLAG_TREE) {
		beginLine(os, indent);
		os << "ExpressionStatement: ";
		printLocation(os);
		expression->print(os, indent + 2, flags);
	} else {
		expression->print(os, indent, flags);
	}
}

juli::NAssignment::NAssignment(NExpression* lhs, NExpression* rhs) :
		NStatement(ASSIGNMENT), lhs(lhs), rhs(rhs) {
}

void juli::NAssignment::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "Assignment: ";
		printLocation(os);
		lhs->print(os, indent + 2, flags);
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

juli::NVariableDeclaration::NVariableDeclaration(NType* type, NIdentifier* name,
		NExpression *assignmentExpr) :
		NStatement(VARIABLE_DECL), name(name), type(type), assignmentExpr(
				assignmentExpr) {
}

void juli::NVariableDeclaration::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "VariableDeclaration: ";
		printLocation(os);
		name->print(os, indent + 2, flags);
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
		os << "Return: ";
		printLocation(os);
		if (expression)
			expression->print(os, indent + 2, flags);
	} else {
		os << "return ";
		if (expression)
			os << expression;
	}
}

juli::NIfClause::NIfClause(NExpression* condition, NBlock* body, bool first) :
		condition(condition), body(body), first(first) {
}

void juli::NIfClause::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);
	if (flags & FLAG_TREE) {
		os << "IfClause: ";
		printLocation(os);
		if (condition) {
			condition->print(os, indent + 2, flags);
		}
		body->print(os, indent + 2, flags);
	} else {
		if (condition) {
			if (first)
				os << "if";
			else
				os << "else if";
			os << " (" << condition << ")";
		} else {
			os << "else";
		}
		os << body;
	}
}

juli::NIfStatement::NIfStatement(std::vector<NIfClause*> clauses) :
		NStatement(IF), clauses(clauses) {
	assert(clauses.size() > 0);
	assert((*clauses.begin())->condition);
}

void juli::NIfStatement::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "If: ";
		printLocation(os);
		for (std::vector<NIfClause*>::const_iterator i = clauses.begin();
				i != clauses.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
	} else {
		for (std::vector<NIfClause*>::const_iterator i = clauses.begin();
				i != clauses.end(); ++i) {
			(*i)->print(os, indent, flags);
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
		os << "While: ";
		printLocation(os);
		condition->print(os, indent + 2, flags);
		body->print(os, indent + 2, flags);
	} else {
		os << "while (" << condition << ")" << std::endl;
		body->print(os, indent + 2, flags);
	}
}

juli::NFunctionSignature::NFunctionSignature(const NType* type,
		const std::string& name, const VariableList arguments, bool varArgs,
		unsigned int modifiers) :
		name(name), type(type), arguments(arguments), varArgs(varArgs), modifiers(
				modifiers) {
}

void juli::NFunctionSignature::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionSignature: " << name;
		printLocation(os);
		type->print(os, indent + 2, flags);
		for (VariableList::const_iterator i = arguments.begin();
				i != arguments.end(); ++i) {
			(*i)->print(os, indent + 2, flags);
		}
		beginLine(os, indent + 2);
		os << "VarArgs: " << varArgs << std::endl;
		beginLine(os, indent + 2);
		os << "C: " << bool(modifiers & MODIFIER_C) << std::endl;
	} else {
		os << type << " " << name << "(" << arguments;
		if (varArgs)
			os << ", ...";
		os << ")";
	}
}

const unsigned int juli::MODIFIER_C = 1;

juli::NFunctionDefinition::NFunctionDefinition(NFunctionSignature * signature,
		NBlock* body) :
		NStatement(FUNCTION_DEF), signature(signature), body(body) {
}

void juli::NFunctionDefinition::print(std::ostream& os, int indent,
		unsigned int flags) const {
	beginLine(os, indent);

	if (flags & FLAG_TREE) {
		os << "FunctionDefinition: ";
		printLocation(os);
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
