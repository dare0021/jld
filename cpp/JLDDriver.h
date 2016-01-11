// requires C++11
// -std=c++1y

#ifndef __JLD_DRIVER_H__
#define __JLD_DRIVER_H__

#define DEBUG

#include "JLDIO.h"

/// Test driver class for the parser. Can be omitted.
class JLDDriver
{
public:
	int main(int argc, char* argv[]);

	static std::string readFile(std::string);
};

#endif // __JLD_DRIVER_H__
