#ifndef __JLD_IO_H__
#define __JLD_IO_H__

#include "DataTypes.h"

#include <iostream>
#include <sstream>

class JLDIO
{
public:
	static JLDDoc* parseText(std::string rawText);

private:
	static std::string stripComments(std::string);
	static std::string findAndUseMacros(std::string);

	/// parses the given doc, and ignores further input.
	/// e.g. if the iss contains 2 docs, only the first is processed, 
	/// and the iss is returned with its cursor after the first doc
	/// Ignores initial white space
	static JLDDoc* parseDoc(std::istringstream&);
	static JLDList* parseList(std::istringstream&);
	static JLDString* parseString(std::istringstream&);

	static std::string listToString(std::list<char>);
	static bool skipOverQuotes(std::ostringstream&, std::istringstream&, std::list<char> *bufferToEmpty = NULL);
	/// Moves the iss cursor to the char before the first non white space char
	/// e.g.    	\n s
	///               ^here
	static bool ignoreWhiteSpace(std::istringstream&);
	static bool getNextChar(char& output, std::ostringstream&, std::istringstream&, std::list<char> *bufferToEmpty = NULL);
	/// checks if there are more items in a list that ends with delim
	/// and is separated with commas
	/// e.g. (a, b, c) is a 3 item list with delim ')'
	static bool moreItems(std::istringstream&, char delim);
};

#endif // __JLD_IO_H__