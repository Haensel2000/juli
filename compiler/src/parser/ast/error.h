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

namespace juli {

class CompilerError {
private:
	std::stringstream msgstream;
	std::string file;
	unsigned int line;
	unsigned int column;
public:
	CompilerError(const CompilerError& ce) :
			msgstream(ce.getMessage()), file(ce.file), line(ce.line), column(
					ce.column) {
	}

	void operator=(const CompilerError& ce) {
		msgstream.str(ce.getMessage());
		file = ce.file;
		line = ce.line;
		column = ce.column;
	}

	CompilerError(std::string msg = std::string(""), std::string file =
			std::string("<unknown file>"), unsigned int line = 0,
			unsigned int column = 0) :
			msgstream(msg), file(file), line(line), column(column) {
	}

	const std::string getMessage() const {
		return msgstream.str();
	}

	std::stringstream& getStream() {
		return msgstream;
	}

	const std::string& getFile() const {
		return file;
	}

	unsigned int getLine() const {
		return line;
	}

	unsigned int getColumn() const {
		return column;
	}

};

}

std::ostream& operator<<(std::ostream& os, const juli::CompilerError& ce);

#endif /* ERROR_H_ */
