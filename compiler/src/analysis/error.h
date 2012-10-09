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

class Error {
private:
	std::stringstream msgstream;
public:
	Error(const Error& ce);

	Error() {}

	void operator=(const Error& ce);

	const std::string getMessage() const;

	std::stringstream& getStream();
};

class CompilerError: public Error {
private:
	const Indentable* node;
public:

	CompilerError(const Indentable* node);

	const std::string& getFile() const;

	const Marker getStart() const;

	const Marker getEnd() const;
};

class ImportError: public Error {
private:
public:
};

}

std::ostream& operator<<(std::ostream& os, const juli::Error& ce);

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce);

#endif /* ERROR_H_ */
