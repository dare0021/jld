#include "JLDIO.h"

// might need to replace with cwctype -> iswspace() for unicode
#include <cctype>
#include <unordered_map>

JLDDoc* JLDIO::parseText(std::string rawText)
{
	rawText = findAndUseMacros(stripComments(rawText));
	// std::cout << "findAndUseMacros result: \n" << rawText << "\n";
	std::istringstream iss(rawText);
	return parseDoc(iss);
}

JLDDoc* JLDIO::parseDoc(std::istringstream& iss)
{
	ignoreWhiteSpace(iss);
	char charBuffer;
	iss.get(charBuffer);
	assert(charBuffer == '{' && "Input is not a doc");
	JLDDoc* out = new JLDDoc();
	// empty list
	ignoreWhiteSpace(iss);
	if(iss.peek() == '}')
	{
		iss.get(charBuffer);
		return out;
	}
	do
	{
		auto keyObj = parseString(iss);
		std::string key = keyObj->getValue();
		delete keyObj;
		ignoreWhiteSpace(iss);
		assert(iss.peek() == ':' && ": expected after a keyword");
		iss.get(charBuffer);
		ignoreWhiteSpace(iss);
		JLDData* valueObj;
		switch (iss.peek())
		{
			case '{':
				valueObj = parseDoc(iss);
				break;
			case '[':
				valueObj = parseList(iss);
				break;
			case '\'':
			case '\"':
				valueObj = parseString(iss);
				break;
			default:
				assert(0 && "Unexpected value");
				break;
		}
		out->push_back(key, valueObj);
	}while(moreItems(iss, '}'));
	iss.get(charBuffer);
	return out;
}

JLDList* JLDIO::parseList(std::istringstream& iss)
{
	ignoreWhiteSpace(iss);
	char charBuffer;
	iss.get(charBuffer);
	assert(charBuffer == '[' && "Input is not a list");
	JLDList* out = new JLDList();
	// empty list
	ignoreWhiteSpace(iss);
	if(iss.peek() == ']')
	{
		iss.get(charBuffer);
		return out;
	}
	do
	{
		ignoreWhiteSpace(iss);
		JLDData* valueObj;
		switch (iss.peek())
		{
			case '{':
				valueObj = parseDoc(iss);
				break;
			case '[':
				valueObj = parseList(iss);
				break;
			case '\'':
			case '\"':
				valueObj = parseString(iss);
				break;
			default:
				assert(0 && "Unexpected value");
				break;
		}
		out->push_back(valueObj);
	}while(moreItems(iss, ']'));
	iss.get(charBuffer);
	return out;
}

JLDString* JLDIO::parseString(std::istringstream& iss)
{
	ignoreWhiteSpace(iss);
	char charBuffer, matchedChar;
	iss.get(charBuffer);
	assert((charBuffer == '\'' || charBuffer == '\"') && "Input is not a string");
	matchedChar = charBuffer;
	std::string text = "";
	iss.get(charBuffer);
	while(charBuffer != matchedChar)
	{
		text += charBuffer;
		iss.get(charBuffer);
		if(!iss.good())
		{
			assert(0 && "No matching quote for a string");
			break;
		}
	}
	ignoreWhiteSpace(iss);
	if(iss.peek() == '+')
	{
		iss.get(charBuffer);
		auto concatObj = parseString(iss);
		text += concatObj->getValue();
		delete concatObj;
	}
	// std::cout<<"parseString found: '"<<text<<"'\n";
	// std::cout<<"Cursor now on: '"<<(char)iss.peek()<<"'\n";
	return new JLDString(text);
}


std::string JLDIO::stripComments(std::string raw)
{
	std::ostringstream oss;
	bool wasSlash = false;
	bool lineComment = false;
	bool blockComment = false;
	bool foundSecondStar = false;
	for (char c : raw)
	{
		if(lineComment)
		{
			if(c == '\n')
			{
				lineComment = false;
				oss << '\n';
			}
			continue;
		}
		if(blockComment)
		{
			if(foundSecondStar && c == '/')
				blockComment = false;
			else if(c == '*')
				foundSecondStar = true;
			else
				foundSecondStar = false;
			continue;
		}
		if(!wasSlash && c == '/')
		{
			wasSlash = true;
			continue;
		}
		if(wasSlash)
		{
			wasSlash = false;
			if(c == '/')
			{
				lineComment = true;
				continue;
			}
			else if(c == '*')
			{
				blockComment = true;
				continue;
			}
			else
			{
				oss << '/';
			}
		}
		oss << c;
	}
	oss << std::flush;
	// performs black magic. Code breaks without this line. Do not remove.
	oss.str();
	// std::cout << "Stripper result: \n" << oss.str() << "\n";
	return oss.str();
}

std::string JLDIO::findAndUseMacros(std::string raw)
{
	//find declaractions
	std::unordered_map<std::string, std::string> macros;
	/// list of keys in descending order of string size
	std::list<std::string> orderedKeyList;
	std::string line = "";
	std::string out = "";
	int cutAfter = 0;
	int thisLine = 0;
	int maxMacroSize = 0;
	for (char c : raw)
	{
		if(c != '\n')
			line += c;
		else
		{
			thisLine = line.size() + 1;
			if(line.substr(0, 8) == "#define ")
			{
				// position after "#define "
				int i = 7;
				while (line[i] == ' ' || line[i] == '\t')
					i++;
				line = line.substr(i);
				i = 0;
				while (line[i] != ' ' && line[i] != '\t')
					i++;
				auto key = line.substr(0, i);
				auto iter = orderedKeyList.begin();
				// sorting performance should not matter since
				// there shouldnt be many macros anyway
				while (iter != orderedKeyList.end())
				{
					if(iter->size() < key.size())
						break;
					iter++;
				}
				orderedKeyList.insert(iter, key);
				macros.insert({key, line.substr(i+1)});
			}
			else
				break;
			cutAfter += thisLine;
			line = "";
		}
	}
	out = raw.substr(cutAfter);
	maxMacroSize = orderedKeyList.front().size();
	// No macros found
	if(!maxMacroSize)
		return out;

	// std::cout<<"maxMacroSize: "<<maxMacroSize<<"\n";
	// std::cout<<"orderedKeyList: ";
	// for (auto s : orderedKeyList)
	// 	std::cout<<s<<" ";
	// std::cout<<"\n"<<std::flush;

	// find and replace
	std::ostringstream oss;
	std::istringstream iss(out);
	std::list<char> workingBuffer;
	char charBuffer;
	// Fill buffer
	while(workingBuffer.size() < maxMacroSize)
	{
		if(!getNextChar(charBuffer, oss, iss, &workingBuffer))
		{
			oss << std::flush;
			return oss.str();
		}
		workingBuffer.push_back(charBuffer);
	}

	do
	{
		bool matchFound = false;
		for(auto key : orderedKeyList)
		{
			auto kvp = macros.find(key);
			assert(kvp != macros.end() && "orderedKeyList should only contain valid keys");
			auto value = kvp->second;
			if(key == listToString(workingBuffer).substr(0, key.size()))
			{
				// remove key from workingBuffer
				for (int i=0; i<key.size(); i++)
				{
					// std::cout<<"REMOVE: '"<<workingBuffer.front()<<"'\n";
					workingBuffer.pop_front();	
				}
				// insert value in to the output
				// std::cout<<"INSERT: '"<<value<<"'\n";
				oss << '\"' << value << '\"';
				// refill workingBuffer
				// same as the one above
				while(workingBuffer.size() < maxMacroSize)
				{
					if(!getNextChar(charBuffer, oss, iss, &workingBuffer))
					{
						break;
					}
					else
					{
						workingBuffer.push_back(charBuffer);
					}
				}
				matchFound = true;
				break;
			}
		}

		if(!matchFound)
		{
			oss << workingBuffer.front();
			// move to next char
			workingBuffer.pop_front();
			// refill workingBuffer
			// because getNextChar() can empty the buffer
			// same as the one above
			while(workingBuffer.size() < maxMacroSize)
			{
				if(!getNextChar(charBuffer, oss, iss, &workingBuffer))
				{
					break;
				}
				else
				{
					workingBuffer.push_back(charBuffer);
				}
			}
			if(workingBuffer.empty())
			{
				oss<<std::flush;
				return oss.str();
			}
		}
	}while(true);
	assert(0 && "Unexpected exit");
	oss<<std::flush;
	return oss.str();
}

std::string JLDIO::listToString(std::list<char> lst)
{
	std::string out = "";
	for(char c : lst)
		out += c;
	return out;
}

/// directly sends the quoted text to oss
/// and returns if a quote was found
/// empties the given buffer in to oss if one is given
/// works only if a quote char is found where the iss cursor is.
bool JLDIO::skipOverQuotes(std::ostringstream& oss, std::istringstream& iss, std::list<char> *bufferToEmpty)
{
	char charBuffer = iss.peek();
	if(charBuffer == '\'' || charBuffer == '\"')
	{
		if(bufferToEmpty && !bufferToEmpty->empty())
		{
			oss << listToString(*bufferToEmpty);
			bufferToEmpty->clear();
		}
		// std::cout<<"Skipping: ";
		char matchedChar = charBuffer;
		iss.get(charBuffer);
		assert(matchedChar == charBuffer);
		do
		{
			oss << charBuffer;
			// std::cout<<charBuffer;
		}while(iss.get(charBuffer) && charBuffer != matchedChar);
		if(iss.good())
			oss << charBuffer;
		// std::cout<<charBuffer<<"\n";
		return true;
	}
	return false;
}

/// moves the iss's cursor to the next non-WS char
bool JLDIO::ignoreWhiteSpace(std::istringstream& iss)
{
	char charBuffer = iss.peek();
	bool output = false;
	while(std::isspace(charBuffer))
	{
		iss.get(charBuffer);
		charBuffer = iss.peek();
		output = true;
	}
	return output;
}

/// gets the next char from iss and places it in output
/// bypasses characters in quotes
/// empties the given buffer in to oss before hand if possible
/// returns false upon failure (i.e. returns iss.good())
bool JLDIO::getNextChar(char& output, std::ostringstream& oss, std::istringstream& iss, std::list<char> *bufferToEmpty)
{
	skipOverQuotes(oss, iss, bufferToEmpty);
	iss.get(output);
	return iss.good();
}

bool JLDIO::moreItems(std::istringstream& iss, char delim)
{
	char charBuffer;
	ignoreWhiteSpace(iss);
	if(iss.peek() == ',')
	{
		iss.get(charBuffer);
		return true;
	}
	else if(iss.peek() != delim)
	{
		std::cout<<"Expected '"<<delim<<"' but found '"<<(char)iss.peek()<<"'\n";
		assert(0 && "Unexpected char");
	}
	return false;
}