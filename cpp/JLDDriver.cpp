#include "JLDDriver.h"

#include <iostream>
#include <fstream>
#include "JLDIO.h"

//TODO: Check getAfterParse() functionality

std::string JLDDriver::readFile(std::string path)
{
	std::ifstream f;

	f.open(path);
	if(f.is_open())
	{
		std::string line, out;
		out = "";

		while(getline(f, line))
		{
			out += line + "\n";
		}

		f.close();
		return out;
	}
	else
		assert(0 && "Unable to open file"); 
}

int main(int argc, char* argv[])
{
	std::string path = "";
#ifdef DEBUG
	if(argc < 2)
	{
		std::cout << "No path given, using example.jld\n";
		path = "example.jld";
	}
#endif // DEBUG
	if(!path.length())
		path = std::string(argv[1]);

	std::string raw = JLDDriver::readFile(path);
#ifdef DEBUG
	std::cout <<"\nRead file: \n"<< raw << "\n";
#endif // DEBUG

	std::cout<<"\nStandard output:\n";
	JLDDoc* jld = JLDIO::parseText(raw);
	std::string referenceText = jld->toString();
	std::cout<<referenceText<<"\n";
	std::cout<<"\nCompacted output:\n";
	std::string compact = jld->toString(0, 1);
	std::cout<<compact<<"\n";
	delete jld;
	std::cout<<"\nCheck compacted output:\n";
	jld = JLDIO::parseText(compact);
	std::string notCompact = jld->toString();
	std::cout<<notCompact<<"\n Test: "<<(referenceText == notCompact)<<"\n";
	delete jld;
	std::cout<<"\nCheck standard output:\n";
	jld = JLDIO::parseText(notCompact);
	notCompact = jld->toString();
	std::cout<<notCompact<<"\n Test: "<<(referenceText == notCompact)<<"\n";
	delete jld;
	return 0;
}