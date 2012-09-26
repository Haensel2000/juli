/*
 * error.h
 *
 *  Created on: Sep 16, 2012
 *      Author: hannes
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <string>
#include <sstream>

#include <parser/ast/node.h>

namespace juli {

class CompilerError {
private:
	std::stringstream msgstream;
	const Indentable* node;
public:
	CompilerError(const CompilerError& ce);

	void operator=(const CompilerError& ce);

	CompilerError(const Indentable* node);

	const std::string getMessage() const;

	std::stringstream& getStream();

	const std::string& getFile() const;

	unsigned int getLine() const;

	unsigned int getStart() const;

	unsigned int getEnd() const;

};

}

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce);

#endif /* ERROR_H_ */
